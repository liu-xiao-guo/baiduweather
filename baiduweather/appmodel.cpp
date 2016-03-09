#include <QGeoPositionInfoSource>
#include <QNetworkAccessManager>
#include <QGeoCoordinate>
#include <QNetworkSession>
#include <QQmlListProperty>
#include <QSignalMapper>
#include <QElapsedTimer>
#include <QTimer>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QNetworkConfigurationManager>

#include "appmodel.h"
#include "weatherdata.h"

const QString CITY_REQ = "http://api.map.baidu.com/geocoder?location=%1,%2&output=json&key=DdzwVcsGMoYpeg5xQlAFrXQt";
const QString WEATHER_REQ = "http://api.map.baidu.com/telematics/v3/weather?location=%1&output=json&ak=DdzwVcsGMoYpeg5xQlAFrXQt";

class AppModelPrivate
{
public:
    static const int baseMsBeforeNewRequest = 5 * 1000; // 5 s, increased after each missing answer up to 10x
    QGeoPositionInfoSource *src;
    QGeoCoordinate coord;
    QString longitude, latitude;
    QString city;
    QNetworkAccessManager *nam;
    QNetworkSession *ns;
    WeatherData now;
    QList<WeatherData*> forecast;
    QQmlListProperty<WeatherData> *fcProp;
    QSignalMapper *geoReplyMapper;
    QSignalMapper *weatherReplyMapper;
    bool ready;
    bool useGps;
    bool hasValidCity;
    QElapsedTimer throttle;
    int nErrors;
    int minMsBeforeNewRequest;
    QTimer delayedCityRequestTimer;
    QString pm25;

    AppModelPrivate() :
            src(NULL),
            nam(NULL),
            ns(NULL),
            fcProp(NULL),
            ready(false),
            useGps(true),
            hasValidCity(false),
            nErrors(0),
            minMsBeforeNewRequest(baseMsBeforeNewRequest),
            pm25("")
    {
        delayedCityRequestTimer.setSingleShot(true);
        delayedCityRequestTimer.setInterval(1000); // 1 s
        throttle.invalidate();
    }
};

static void forecastAppend(QQmlListProperty<WeatherData> *prop, WeatherData *val)
{
    Q_UNUSED(val);
    Q_UNUSED(prop);
}

static WeatherData *forecastAt(QQmlListProperty<WeatherData> *prop, int index)
{
    AppModelPrivate *d = static_cast<AppModelPrivate*>(prop->data);
    return d->forecast.at(index);
}

static int forecastCount(QQmlListProperty<WeatherData> *prop)
{
    AppModelPrivate *d = static_cast<AppModelPrivate*>(prop->data);
    return d->forecast.size();
}

static void forecastClear(QQmlListProperty<WeatherData> *prop)
{
    static_cast<AppModelPrivate*>(prop->data)->forecast.clear();
}

AppModel::AppModel(QObject *parent) :
    QObject(parent),
    d(new AppModelPrivate)
{
    d->fcProp = new QQmlListProperty<WeatherData>(this, d,
                                                        forecastAppend,
                                                        forecastCount,
                                                        forecastAt,
                                                        forecastClear);

    d->geoReplyMapper = new QSignalMapper(this);
    d->weatherReplyMapper = new QSignalMapper(this);

    connect(d->geoReplyMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(handleGeoNetworkData(QObject*)));

    connect(d->weatherReplyMapper, SIGNAL(mapped(QObject*)),
            this, SLOT(handleWeatherNetworkData(QObject*)));

    // make sure we have an active network session
    d->nam = new QNetworkAccessManager(this);

    networkSessionOpened();
}

void AppModel::networkSessionOpened()
{
    qDebug() << "networkSessionOpened!";

    d->src = QGeoPositionInfoSource::createDefaultSource(this);

    qDebug() << "d->src" << d->src;

    if (d->src) {
        d->useGps = true;
        connect(d->src, SIGNAL(positionUpdated(QGeoPositionInfo)),
                this, SLOT(positionUpdated(QGeoPositionInfo)));
        connect(d->src, SIGNAL(error(QGeoPositionInfoSource::Error)),
                this, SLOT(positionError(QGeoPositionInfoSource::Error)));
        d->src->startUpdates();
    } else {
        d->useGps = false;
        d->city = "Brisbane";
        emit cityChanged();
        this->refreshWeather();
    }
}

void AppModel::positionUpdated(QGeoPositionInfo gpsPos)
{
    qDebug() << "positionUpdated";

    d->coord = gpsPos.coordinate();

    qDebug() << "coord: " << d->coord.longitude() << " " << d->coord.latitude();

    if (!(d->useGps))
        return;

    if ( !d->hasValidCity ) {
        queryCity();
    }
}

void AppModel::positionError(QGeoPositionInfoSource::Error e)
{
    qDebug() << "positionError";

    Q_UNUSED(e);
    qWarning() << "Position source error. Falling back to simulation mode.";
    // cleanup insufficient QGeoPositionInfoSource instance
    d->src->stopUpdates();
    d->src->deleteLater();
    d->src = 0;

    // activate simulation mode
    d->useGps = false;
    d->city = "Beijing";
    emit cityChanged();
    this->refreshWeather();
}


void AppModel::handleGeoNetworkData(QObject *replyObj)
{
    qDebug() << "Entering handleGeoNetworkData";

    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(replyObj);
    if ( !networkReply ) {
        hadError(false); // should retry?
        return;
    }

    if ( !networkReply->error() ) {
        d->nErrors = 0;

        if (!d->throttle.isValid())
            d->throttle.start();

        d->minMsBeforeNewRequest = d->baseMsBeforeNewRequest;
        //convert coordinates to city name
        QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());

        qDebug() << "document: " << document;

        QVariantMap root = document.toVariant().toMap();
        QVariantMap result = root["result"].toMap();
        QVariantMap addressComponent = result["addressComponent"].toMap();

        qDebug() << "addressComopnent: " << addressComponent;

        QString city = addressComponent["city"].toString();
        qDebug() << "got city: " << city;

        if (city != d->city) {
            d->city = city;
            d->hasValidCity = true;
            emit cityChanged();
            refreshWeather();
        }
    } else {
        hadError(true);
    }

    networkReply->deleteLater();

    qDebug()<< "Exiting handleGeoNetworkData...";
}


void AppModel::handleWeatherNetworkData(QObject *replyObj)
{
    qDebug() << "handleWeatherNetworkData";
    qDebug() << "got weather network data";

    QNetworkReply *networkReply = qobject_cast<QNetworkReply*>(replyObj);
    if (!networkReply)
        return;

    if (!networkReply->error()) {
        // We need to clear the previously stored data

        QJsonDocument document = QJsonDocument::fromJson(networkReply->readAll());

        qDebug() << "document: " << document;

        QVariantMap root = document.toVariant().toMap();
        QString date = root["date"].toString();
        qDebug() << "date: " << date;

        QList<QVariant> list = root["results"].toList();
        int count = list.count();

        for (int i = 0; i < count; i++ ) {
            QVariantMap item = list.at(i).toMap() ;

            QString city = item["currentCity"].toString();
            qDebug() << "city: " << city;

            QString pm25 = item["pm25"].toString();
            d->pm25 = pm25;
            emit pm25Changed();
            qDebug() << "PM25: " << pm25;

            QList<QVariant> index = item["index"].toList();
            int size = index.count();
            qDebug() << "size: " << size;

            for ( int j = 0; j < size; j ++ ) {
                QVariantMap each = index.at(j).toMap() ;

                QString title = each["title"].toString();
                qDebug() << "title: " << title;

                QString des = each["des"].toString();
                qDebug() << "des: " << des;
            }

            d->forecast.clear();

            foreach (const QVariant &k, item["weather_data"].toList()) {                
                QVariantMap each = k.toMap();

                QString date = each["date"].toString();
                qDebug() << "date: " << date << " length: " << date.length();
                date = date.left(2);
                qDebug() << "new date: " << date;

                QString dayPictureUrl = each["dayPictureUrl"].toString();
                qDebug() << "dayPictureUrl: " << dayPictureUrl;

                QString nightPictureUrl = each["nightPictureUrl"].toString();
                qDebug() << "nightPictureUrl: " << nightPictureUrl;

                QString weather = each["weather"].toString();
                qDebug() << "weather: " << weather;

                QString wind = each["wind"].toString();
                qDebug() << "wind: " << wind;

                QString temperature = each["temperature"].toString();
                qDebug() << "temperature: " << temperature;

                // Now let's fill in the weather data
                WeatherData *forecastEntry = new WeatherData();

                forecastEntry->setDate(date);
                forecastEntry->setDayPictureUrl(dayPictureUrl);
                forecastEntry->setNightPictureUrl(nightPictureUrl);
                forecastEntry->setWeather(weather);
                forecastEntry->setWind(wind);
                forecastEntry->setTemp(temperature);

                d->forecast.append(forecastEntry);
            }

        }

        if (!(d->ready)) {
            d->ready = true;
            emit readyChanged();
        }

        emit weatherChanged();
    }

    networkReply->deleteLater();
}

void AppModel::hadError(bool tryAgain)
{
    qDebug() << "positionError";

    d->throttle.start();
    if (d->nErrors < 10)
        ++d->nErrors;
    d->minMsBeforeNewRequest = (d->nErrors + 1) * d->baseMsBeforeNewRequest;
    if (tryAgain)
        d->delayedCityRequestTimer.start();
}

void AppModel::refreshWeather()
{
    qDebug() << "refreshWeather";

    if (d->city.isEmpty()) {
        qDebug() << "refreshing weather skipped (no city)";
        return;
    }

    qDebug() << "refreshing weather";

    QString temp = WEATHER_REQ.arg(d->city);
    QUrl url(temp);
    qDebug() << "weather req: " << temp;

    QNetworkReply *rep = d->nam->get(QNetworkRequest(url));

    // connect up the signal right away
    d->weatherReplyMapper->setMapping(rep, rep);
    connect(rep, SIGNAL(finished()),
            d->weatherReplyMapper, SLOT(map()));
}

bool AppModel::ready() const
{
    return d->ready;
}

void AppModel::queryCity()
{
    qDebug() << "queryCity....";

    //don't update more often then once a minute
    //to keep load on server low
    if (d->throttle.isValid() && d->throttle.elapsed() < d->minMsBeforeNewRequest ) {
        qDebug() << "delaying query of city";
        if (!d->delayedCityRequestTimer.isActive())
            d->delayedCityRequestTimer.start();
        return;
    }

    qDebug() << "requested query of city";

    d->throttle.start();
    d->minMsBeforeNewRequest = (d->nErrors + 1) * d->baseMsBeforeNewRequest;

    QString latitude, longitude;
    longitude.setNum(d->coord.longitude());
    latitude.setNum(d->coord.latitude());

    QString temp = CITY_REQ.arg(d->coord.latitude()).arg(d->coord.longitude());
    qDebug() << "req string: " << temp;

    QUrl url(temp);

    QNetworkReply *rep = d->nam->get(QNetworkRequest(url));

    // connect up the signal right away
    d->geoReplyMapper->setMapping(rep, rep);
    connect(rep, SIGNAL(finished()),
            d->geoReplyMapper, SLOT(map()));

    qDebug()<< "exiting queryCity...";
}

QQmlListProperty<WeatherData> AppModel::forecast() const
{
    return *(d->fcProp);
}

QString AppModel::city() const
{
    return d->city;
}

void AppModel::setCity(const QString &value)
{
    if ( d->city == value)
        return;

    d->city = value;
    emit cityChanged();
    refreshWeather();
}

QString AppModel::pm25() const
{
    return d->pm25;
}

void AppModel::setPm25(const QString &value)
{
    if ( d->pm25 == value)
        return;

    d->pm25 = value;
    emit pm25Changed();
}

