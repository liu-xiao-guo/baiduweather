import QtQuick 2.0
import Ubuntu.Components 1.1
import WeatherInfo 1.0

/*!
    \brief MainView with a Label and Button elements.
*/

MainView {
    // objectName for functional testing purposes (autopilot-qt5)
    objectName: "mainView"

    // Note! applicationName needs to match the "name" field of the click manifest
    applicationName: "baiduweather.liu-xiao-guo"

    /*
     This property enables the application to change orientation
     when the device is rotated. The default is false.
    */
    //automaticOrientation: true

    // Removes the old toolbar and enables new features of the new header.
    useDeprecatedToolbar: false

    width: units.gu(60)
    height: units.gu(85)

    AppModel {
        id: mymodel
        onReadyChanged: {
            console.log("ready is changed!");
            console.log("city: " + city)
            //            console.log("forecast[0]: " + forecast[0].dayPictureUrl);

            if (mymodel.ready) {
                page.state = "ready"
                input.text = mymodel.city
                //                listview.model = mymodel.forecast;
            } else {
                page.state = "loading"
            }
        }
    }

    Page {
        id: page    // We cannot name it window.
        title: i18n.tr("Baidu Weather")

        Column {
            anchors.fill: parent
            spacing: units.gu(1)

            TextField {
                id: input
                anchors.horizontalCenter: parent.horizontalCenter

                onAccepted: {
                    mymodel.city = text;
                }

                onTextChanged: {
                    mymodel.city = text;
                }
            }

            Image {
                id: image
                height: parent.height/3
                width: height
                anchors.horizontalCenter: parent.horizontalCenter
                source: {
                    if ( !mymodel.ready )
                        return ""

                    var date = new Date();
                    var n = date.getHours();
                    if ( n >= 7 && n < 18 ) {
                        return mymodel.forecast[0].dayPictureUrl;
                    }
                    else {
                        return mymodel.forecast[0].nightPictureUrl;
                    }
                }

                Label {
                    id: city
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: mymodel.city
                    fontSize: "large"
                }

                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top:city.bottom
                    anchors.bottomMargin: units.gu(1)
                    text: "PM25: " + mymodel.pm25;
                    fontSize: "large"
                }
            }

            Row {
                id: firstrow
                spacing: units.gu(1)
                width: parent.width
                anchors.horizontalCenter: parent.horizontalCenter

                Repeater {
                    model: mymodel.forecast
                    width: page.width
                    anchors.horizontalCenter: parent.horizontalCenter

                    Column {
                        spacing: units.gu(2)

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: date
                            font.pixelSize: units.gu(3)
                        }

                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: (page.width - firstrow.spacing*3 ) /4
                            height: width
                            source: dayPictureUrl
                        }

                        Image {
                            anchors.horizontalCenter: parent.horizontalCenter
                            width: (page.width - firstrow.spacing*3) /4
                            height: width
                            source: nightPictureUrl
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: mymodel.forecast[index].temp
                        }
                    }
                }
            }
        }
    }
}

