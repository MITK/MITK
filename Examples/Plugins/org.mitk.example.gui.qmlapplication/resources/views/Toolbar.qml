import QtQuick 2.4
import "../components" as Mitk

Rectangle
{
  id: windowbar

  property var workbench;
  property var tooltipArea;

  Row
  {
    anchors.left: parent.left
    anchors.leftMargin: 30
    anchors.verticalCenter: parent.verticalCenter
    spacing: 1

    Mitk.Button
    {
      width: 35
      height: 25
      rightEdges: true

      tooltip: "Open File"
      tooltipArea: windowbar.tooltipArea

      source: "../icons/document-open.svg"

      onClicked: workbench.loadFiles()
    }

    Mitk.Button
    {
      width: 35
      height: 25
      radius: 0

      tooltip: "Save Project - not implemented"
      tooltipArea: windowbar.tooltipArea

      source: "../icons/document-save.svg"
    }

    Mitk.Button
    {
      width: 35
      height: 25
      leftEdges: true

      tooltip: "Close Project - not implemented"
      tooltipArea: windowbar.tooltipArea

      source: "../icons/edit-delete.svg"
    }
  }

  Row
  {
    anchors.right: parent.right
    anchors.rightMargin: 30
    anchors.verticalCenter: parent.verticalCenter
    spacing: 1

    Mitk.Button
    {
      width: 35
      height: 25
      rightEdges: true

      tooltip: "Toogle Data Manager"
      tooltipArea: windowbar.tooltipArea

      source: "../icons/datamanager.png"

      isToggle: true
      toggle: true

      onToggleChanged:
      {
        if(toggle)
        {
          dataManager.width = 240
        }
        else
        {
          dataManager.width = 0
        }
      }
    }

    Mitk.Button
    {
      width: 35
      height: 25
      leftEdges: true

      tooltip: "Toogle Properties"
      tooltipArea: windowbar.tooltipArea

      source: "../icons/properties.png"

      isToggle: true
      toggle: false

      onToggleChanged:
      {
        if(toggle)
        {
          mitkproperties.width = 320
        }
        else
        {
          mitkproperties.width = 0
        }
      }
    }
  }

  Rectangle
  {
    color: "#242424"
    anchors.left: parent.left
    anchors.bottom: parent.bottom
    anchors.right: parent.right

    height: 1
  }
}