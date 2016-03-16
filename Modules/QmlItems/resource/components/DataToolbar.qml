import QtQuick 2.4
import "../components" as Mitk

Rectangle
{
  id: root
  property var tooltipArea;
  property bool enabled;
  property var datamanager;

  gradient: Gradient {
    GradientStop { position: 0.0; color: "#4C4C4C" }
    GradientStop { position: 1.0; color: "#474747" }
  }

  Row
  {
    anchors.verticalCenter: parent.verticalCenter
    anchors.left: parent.left
    anchors.leftMargin: 10
    spacing: 15

    Mitk.VectorIcon
    {
      icon: "\uf021"
      tooltip: "Reinit selected node"
      toolArea: root.tooltipArea
      enabled: root.enabled

      onClicked: datamanager.reinitNode()
    }

    Mitk.VectorIcon
    {
      icon: "\uf0ac"
      tooltip: "Global Reinit"
      toolArea: root.tooltipArea
      enabled: root.enabled

      onClicked: datamanager.globalReinit()
    }

    Mitk.VectorIcon
    {
      icon: "\uf1f8"
      tooltip: "Delete seleted node"
      toolArea: root.tooltipArea
      enabled: root.enabled

      onClicked: datamanager.deleteNode()
    }
  }
}