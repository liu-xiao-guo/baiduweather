#include "weatherdata.h"

WeatherData::WeatherData(QObject *parent) :
        QObject(parent)
{
}

WeatherData::WeatherData(const WeatherData &other) :
        QObject(0),
        m_date(other.m_date),
        m_dayPictureUrl(other.m_dayPictureUrl),
        m_nightPictureUrl(other.m_nightPictureUrl),
        m_weather(other.m_weather),
        m_wind(other.m_wind),
        m_temp(other.m_temp)
{
}

QString WeatherData::date() const
{
    return m_date;
}

void WeatherData::setDate(const QString &value)
{
    if ( m_date == value )
        return;

    m_date = value;
    emit dataChanged();
}

QString WeatherData::nightPictureUrl() const
{
    return m_nightPictureUrl;
}

void WeatherData::setNightPictureUrl(const QString &value)
{
    if (m_nightPictureUrl == value)
        return;

    m_nightPictureUrl = value;
    emit dataChanged();
}


QString WeatherData::dayPictureUrl() const
{
    return m_dayPictureUrl;
}

void WeatherData::setDayPictureUrl(const QString &value)
{
    if (m_dayPictureUrl == value)
        return;

    m_dayPictureUrl = value;
    emit dataChanged();
}

QString WeatherData::weather() const
{
    return m_weather;
}

void WeatherData::setWeather(const QString &value)
{
    if ( m_weather == value)
        return;

    m_weather = value;
    emit dataChanged();
}

QString WeatherData::wind() const
{
    return m_wind;
}

void WeatherData::setWind(const QString &value)
{
    if ( m_wind == value)
        return;

    m_wind = value;
    emit dataChanged();
}

QString WeatherData::temp() const
{
    return m_temp;
}

void WeatherData::setTemp(const QString &value)
{
    if ( m_temp == value)
        return;

    m_temp = value;
    emit dataChanged();
}

