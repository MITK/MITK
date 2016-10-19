import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Mitk.Views 1.0
import "../components" as Mitk

SliderNavigator
{
  id: sliderNavigator

  anchors.left: parent.left
  anchors.right: parent.right
  anchors.leftMargin: 5
  anchors.rightMargin: 5

  property alias text: label.text

  Row
  {
    anchors.fill: parent
    spacing: 5

    Label
    {
      id: label
      anchors.verticalCenter: parent.verticalCenter
      width: parent.width/4
      color: "white"
      font.pixelSize: 12
    }

    Slider
    {
      id: slider
      anchors.verticalCenter: parent.verticalCenter
      width: parent.width/2 -10
      value: 0

      style: SliderStyle {
        groove: Rectangle {
          implicitWidth: 20
          implicitHeight: 4
          color: "gray"
          radius: 3
        }
        handle: Rectangle {
          anchors.centerIn: parent
          color: control.pressed ? "white" : "lightgray"
          border.color: "gray"
          border.width: 1
          implicitWidth: 10
          implicitHeight: 10
          radius: 2
        }
      }

      onValueChanged:
      {
        sliderNavigator.value = value
      }
    }

    Mitk.Spinbox
    {
      id: spinbox
      value: 0
      width: parent.width/4
      anchors.verticalCenter: parent.verticalCenter
      borderColor: "#282828"

      onValueChanged:
      {
        sliderNavigator.value = value
      }
    }

  }

  onSync:
  {
    spinbox.value = sliderNavigator.value
    slider.value = sliderNavigator.value

    spinbox.minimumValue = sliderNavigator.min
    slider.minimumValue = sliderNavigator.min

    spinbox.maximumValue = sliderNavigator.max
    slider.maximumValue = sliderNavigator.max
  }

}