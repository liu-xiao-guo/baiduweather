#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>

#include "appmodel.h"
#include "weatherdata.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<WeatherData>("WeatherInfo", 1, 0, "WeatherData");
    qmlRegisterType<AppModel>("WeatherInfo", 1, 0, "AppModel");

    QQuickView view;
    view.setSource(QUrl(QStringLiteral("qrc:///Main.qml")));
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.show();
    return app.exec();
}

