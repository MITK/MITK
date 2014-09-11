import QtQuick 2.0
import QmlMitk 1.0

Item {

  width: 800
  height: 600

     Rectangle {
        id: topbar

        color: "papayawhip"

        height: 30

        anchors.top : parent.top
        anchors.left : parent.left
        anchors.right : parent.right
    }

    Rectangle {
        id: leftbar

        color: "tomato"

        width: 40

        anchors.left : parent.left
        anchors.top : topbar.bottom
        anchors.bottom : parent.bottom
    }

     Rectangle {
        id: rightbar

        color: "peachpuff"

        width: 40

        anchors.right : parent.right
        anchors.top : topbar.bottom
        anchors.bottom : parent.bottom
    }

     Rectangle {
        id: bottombar

        color: "sandybrown"

        height: 15

        anchors.bottom : parent.bottom
        anchors.left : parent.left
        anchors.right : parent.right
    }



    QmlMitkFourRenderWindowWidget {
        id : mitkMultiWidget
        objectName : "mitkMultiWidget"

        anchors.left : leftbar.right
        anchors.right : rightbar.left
        anchors.top : topbar.bottom
        anchors.bottom : bottombar.top
        anchors.margins : 0
    }

    Rectangle
    {
      id: reactiveRect
      color: "mintcream"
      opacity: 0.8
      radius: 10
      border.width: 5
      border.color: "mintcream"

      width: 610
      height: 200

      anchors.horizontalCenter: mitkMultiWidget.horizontalCenter
      anchors.verticalCenter: mitkMultiWidget.verticalCenter

      rotation: 3

      Text
      {
          id: label
          color: "dodgerblue"
          wrapMode: Text.WordWrap
          font.pointSize: 16
          font.bold: true
          text: "The background is a QmlMitkFourRenderWindowWidget\nwrapping four QmlMitkRenderWindowItems\nbased on QVTKQuickItem from OpenView.\nAll the colorful environment is a QML scene..."

          width: 590
          anchors.horizontalCenter: reactiveRect.horizontalCenter
          anchors.verticalCenter: reactiveRect.verticalCenter

          rotation: 2

      }

      MouseArea { id: mouseArea; anchors.fill: reactiveRect; hoverEnabled: true }

      // When the text is pressed, move, rotate and change the color to yellow.
      states: State
      {
        name: "down"; when: mouseArea.containsMouse == true

        PropertyChanges
        {
          target: reactiveRect; rotation: -2;
        }

        PropertyChanges
        {
          target: label; color: "deeppink"; rotation: -2
        }
      }

      // How do we get from the implicit state to the down sate?
      transitions: Transition
      {
        from: ""; to: "down"; reversible: true

        ParallelAnimation
        {
          NumberAnimation { properties: "y,rotation"; duration: 500; easing.type: Easing.InOutQuad }
          ColorAnimation { duration: 500 }
        }
      }
    }

}
