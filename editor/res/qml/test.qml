import QtQuick 2.0
import QtQuick.Window 2.0
import QtLocation 5.6
import QtPositioning 5.6

//CSA 46
//CSA 47
Rectangle {
    id: root
//        width: d
//        height: d
//    anchors.centerIn: parent
    color: "red"

    Plugin {
           id: mapPlugin
           name: "osm" // "mapboxgl", "esri", ...
           // specify plugin parameters if necessary
           // PluginParameter {
           //     name:
           //     value:
           // }
       }

       Map {
           id: map
           anchors.fill: parent
           plugin: mapPlugin
           center: QtPositioning.coordinate(43.938129, 5.768346) // Oslo
           zoomLevel: 15
           property MapCircle circle

           Component.onCompleted: {
                   circle = Qt.createQmlObject('import QtLocation 5.6; MapCircle {}', page)
                   circle.center = QtPositioning.coordinate(43.938129, 5.768346)
                   circle.radius = 500000.0
                   circle.color = 'green'
                   circle.border.width = 3
                   map.addMapItem(circle)
               }

           MapCircle {
               center {
                   latitude: 43.938129
                   longitude: 5.768346
               }
               border.width: 4
               radius: 5000
               color : 'green'
           }




           /*MapRectangle {
                   color: 'green'
                   border.width: 2
                   topLeft {
                       latitude: 43.938129
                       longitude: 5.768346
                   }
                   bottomRight {
                       latitude: 44.938129
                       longitude: 6.768346
                   }
               }*/

       }
}
