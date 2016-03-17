import QtQuick 2.4

Item
{
  id: root
  property alias text: label.text

  Rectangle
  {
    anchors.fill: parent
    color: "#2E2E2E"

    Text
    {
      id: label

      anchors.left: parent.left
      anchors.verticalCenter: parent.verticalCenter
      anchors.leftMargin: 10
      color: "#ffffff"
      font.pointSize: 10
      font.weight: Font.DemiBold
    }
  }
}