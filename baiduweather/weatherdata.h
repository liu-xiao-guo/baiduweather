#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QObject>

class WeatherData : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString date
               READ date WRITE setDate
               NOTIFY dataChanged)
    Q_PROPERTY(QString dayPictureUrl
               READ dayPictureUrl WRITE setDayPictureUrl
               NOTIFY dataChanged)
    Q_PROPERTY(QString nightPictureUrl
               READ nightPictureUrl WRITE setNightPictureUrl
               NOTIFY dataChanged)
    Q_PROPERTY(QString weather
               READ weather WRITE setWeather
               NOTIFY dataChanged)
    Q_PROPERTY(QString wind
               READ wind WRITE setWind
               NOTIFY dataChanged)
    Q_PROPERTY(QString temp
               READ temp WRITE setTemp
               NOTIFY dataChanged)
public:
    explicit WeatherData(QObject *parent = 0);
    WeatherData(const WeatherData &other);

    QString date() const;
    QString dayPictureUrl() const;
    QString nightPictureUrl() const;
    QString weather() const;
    QString wind() const;
    QString temp() const;

    void setDate(const QString &value);
    void setDayPictureUrl(const QString &value);
    void setNightPictureUrl(const QString &value);
    void setWeather(const QString &value);
    void setWind(const QString &value);
    void setTemp(const QString &value);

signals:
    void dataChanged();

private:
    QString m_date;
    QString m_dayPictureUrl;
    QString m_nightPictureUrl;
    QString m_weather;
    QString m_wind;
    QString m_temp;
};

Q_DECLARE_METATYPE(WeatherData)

#endif // WEATHERDATA_H
