import QtQuick 2.4
import "../components" as Mitk

Rectangle
{
  id: root
  property var tooltipArea;
  property var multiItem;

  gradient: Gradient {
    GradientStop { position: 0.0; color: "#334C4C4C" }
    GradientStop { position: 1.0; color: "#33474747" }
  }

  Row
  {
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 10
    spacing: 15

    Mitk.VectorIcon
    {
      icon: "\uf1cb"
      tooltip: "Toggle planes"
      toolArea: root.tooltipArea
      pixelSize: 13
      enabled: true
      defaultColor: "#666666";
      disableColor: "#EFEFEF"

      onClicked: multiItem.togglePlanes()
    }
  }
}