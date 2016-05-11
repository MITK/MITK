import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Mitk.Views 1.0
import "../components" as Mitk

TransferItem
{
  id: root

  PiecewiseItem
  {
    id: opacityItem
    anchors.left: parent.left
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.leftMargin: 10
    anchors.rightMargin: 10
    height: 150
    focus: true
    enabled: root.enabled

    Mitk.TransferOverlay
    {
      id: opacityField
      anchors.top: parent.top
      anchors.right: parent.right
      anchors.topMargin: 2
      label: "Opacity"
      enabled: root.enabled

      onPressed:
      {
        opacityItem.opacity = text
      }
    }

    Mitk.TransferOverlay
    {
      id: grayField
      anchors.top: opacityField.bottom
      anchors.right: parent.right
      anchors.topMargin: 2
      label: "Value"
      enabled: root.enabled

      onPressed:
      {
        opacityItem.greyValue = text
      }
    }

    Mitk.TransferOverlay
    {
      id: colorField
      anchors.top: grayField.bottom
      anchors.right: parent.right
      anchors.topMargin: 2
      label: "Color"
      enabled: root.enabled

      onPressed:
      {
        colorItem.greyValue = text
      }
    }
  }

  ColorItem
  {
    id: colorItem
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: opacityItem.bottom
    anchors.leftMargin: 10
    anchors.rightMargin: 10
    height: 20
    enabled: root.enabled
  }

  Mitk.Slider
  {
    id: slider
    anchors.top: colorItem.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    anchors.topMargin: 9
    anchors.leftMargin: 10
    anchors.rightMargin: 10

    minimumValue: root.min;
    maximumValue: root.max;

    onValueLowerChangedByHandle:
    {
      root.currentMin = valueLower;
    }

    onValueUpperChangedByHandle:
    {
      root.currentMax = valueUpper;
    }
  }

  Connections
  {
    target: colorItem;

    onSync:
    {
      colorField.text = colorItem.greyValue.toFixed(2);
    }
  }


  Connections
  {
    target: opacityItem;

    onSync:
    {
      grayField.text = opacityItem.greyValue.toFixed(2);
      opacityField.text = opacityItem.opacity.toFixed(2);
    }
  }

  onSync:
  {
    slider.valueUpper = root.currentMax
    slider.valueLower = root.currentMin
  }

  Component.onCompleted:
  {
    root.opacityItem = opacityItem;
    root.colorItem = colorItem;
  }
}