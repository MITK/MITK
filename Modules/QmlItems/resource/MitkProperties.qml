import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Mitk.Views 1.0
import "./components" as Mitk

Item
{
	id: root
	property var tooltipArea;

	property bool wRendering: properties.rendering;

	Properties
	{
		id: properties
	}

	Behavior on width
	{
		PropertyAnimation
		{
			duration: 120
		}
	}

	Rectangle
	{
		id: attributes

		anchors.fill: parent
		color: "#2E2E2E"

		Rectangle
		{
			id: visualization

			anchors.left: parent.left
			anchors.top: parent.top
			anchors.right: parent.right

			height: 22

			gradient: Gradient {
				GradientStop { position: 0.0; color: "#4C4C4C" }
				GradientStop { position: 1.0; color: "#474747" }
			}

			Text
			{
				anchors.left: parent.left
				anchors.verticalCenter: parent.verticalCenter
				anchors.leftMargin: 5
				text: "Volume Visualization"
				color: "#ffffff"
				font.pointSize: 12
			}
		}

		Mitk.Checkbox
		{
			id: cb_volumerendering

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: visualization.bottom
			anchors.leftMargin: 10
			anchors.rightMargin: 10
			anchors.topMargin: 7
			text: "Volumerendering"
			checked: false
			enabled: false

			onCheckedChanged:
			{
				properties.rendering = checked
			}
		}

		Mitk.Checkbox
		{
			id: cb_lod

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: cb_volumerendering.bottom
			anchors.leftMargin: 10
			anchors.rightMargin: 10
			anchors.topMargin: 7
			checked: false
			enabled: false

			text: "Level of Detail"

			onCheckedChanged:
			{
				properties.lod = checked
			}
		}

		Mitk.Combobox
		{
			id: opt_mode;

			anchors.left: parent.left
			anchors.top: cb_lod.bottom
			anchors.right: parent.right
			anchors.leftMargin: 10
			anchors.rightMargin: 10
			anchors.topMargin: 7
			currentIndex: 0
			enabled: false

			model: [
			"CPU raycast",
			"CPU MIP raycast",
			"GPU slicing",
			"GPU raycast",
			"GPU MIP raycast",
			]

			onCurrentIndexChanged:
			{
				properties.mode = currentIndex
			}
		}

		Mitk.TransferFunction
		{
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: opt_mode.bottom
			anchors.topMargin: 10
			height: 185
			enabled: cb_volumerendering.checked
		}
	}

	Rectangle
	{
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: parent.bottom

		width: 1
		color: "#242424"
	}

	Connections
	{
		target: properties;

		onSync:
		{
			opt_mode.currentIndex = properties.mode;
			opt_mode.enabled = properties.enabled;

			cb_lod.checked = properties.lod;
			cb_lod.enabled = properties.enabled;

			cb_volumerendering.checked = properties.rendering;
			cb_volumerendering.enabled = properties.enabled;
		}
	}
}
