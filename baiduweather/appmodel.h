#ifndef APPMODEL_H
#define APPMODEL_H

#include <QObject>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QQmlListProperty>

class AppModelPrivate;
class WeatherData;

class AppModel : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool ready
               READ ready
               NOTIFY readyChanged)
    Q_PROPERTY(QQmlListProperty<WeatherData> forecast
               READ forecast
               NOTIFY weatherChanged)   
    Q_PROPERTY(QString city
               READ city WRITE setCity
               NOTIFY cityChanged)
    Q_PROPERTY(QString pm25
               READ pm25 WRITE setPm25
               NOTIFY pm25Changed)

public:
    explicit AppModel(QObject *parent = 0);

    bool ready() const;
    QQmlListProperty<WeatherData> forecast() const;
    QString city() const;
    void setCity(const QString &value);
    QString pm25() const;
    void setPm25(const QString &value);

private slots:
    void handleWeatherNetworkData(QObject *replyObj);
    void handleGeoNetworkData(QObject *replyObj);
    void networkSessionOpened();
    void positionUpdated(QGeoPositionInfo gpsPos);
    void positionError(QGeoPositionInfoSource::Error e);
    void queryCity();

public slots:
    Q_INVOKABLE void refreshWeather();

signals:
    void readyChanged();
    void cityChanged();
    void pm25Changed();
    void weatherChanged();

private:
    void hadError(bool tryAgain);

private:
    AppModelPrivate *d;
};

#endif // APPMODEL_H
