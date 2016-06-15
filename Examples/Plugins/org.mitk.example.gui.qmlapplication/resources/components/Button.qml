import QtQuick 2.4

Item
{
	id: root

	property string source: "-";
	property string tooltip: "-";
	property color activeColor: "#282828";
	property color buttonColor: mouseArea.pressed||toggle ? activeColor : "#454545";
	property color topHighlightColor: mouseArea.pressed||toggle ? activeColor : "#515151";
	property color bottomHighlightColor: mouseArea.pressed||toggle ? activeColor : "#2C2C2C";
	property int radius: 3;
	property bool rightEdges: false;
	property bool leftEdges: false;
	property bool isToggle: false;
	property bool toggle: false;
	property var tooltipArea;

	clip: true

	signal clicked()

	Rectangle
	{
		id: topHighlight

		color: root.topHighlightColor
		x: button.x
		y: button.y-1

		width: parent.width
		height: parent.height

		radius: root.radius
	}

	Rectangle
	{
		id: bottomHighlight

		color: root.bottomHighlightColor
		x: button.x
		y: button.y+1

		width: parent.width
		height: parent.height

		radius: root.radius
	}

	Rectangle
	{
		id: button

		height: root.height-2
		width: root.width
		radius: root.radius
		y: 1

		Image
		{
			anchors.fill: parent
			anchors.margins: 3

			smooth: true
			fillMode: Image.PreserveAspectFit
			source: root.source
		}

		color: root.buttonColor

		MouseArea
		{
			id: mouseArea

			anchors.fill: parent
			hoverEnabled: true

			onClicked:
			{
				if (root.isToggle)
				{
					root.toggle = !root.toggle;
				}
				root.clicked()
			}

			onEntered:
			{
				root.tooltipArea.text = root.tooltip
			}

			onExited:
			{
				root.tooltipArea.text = ""
			}
		}
	}

	Rectangle
	{
		id: rightEdge

		height: root.height
		width: 4
		x: root.width - width
		color: root.rightEdges ? root.buttonColor : "transparent"

		Rectangle
		{
			id: innerRightEdge1

			anchors.top: parent.top
			anchors.right: parent.right
			anchors.left: parent.left
			height: 1

			color: root.rightEdges ? root.topHighlightColor : "transparent"
		}

		Rectangle
		{
			id: innerRightEdge2

			anchors.bottom: parent.bottom
			anchors.right: parent.right
			anchors.left: parent.left
			height: 1

			color: root.rightEdges ? root.bottomHighlightColor : "transparent"
		}
	}

	Rectangle
	{
		id: leftEdge

		height: root.height
		width: 4
		color: root.leftEdges ? root.buttonColor : "transparent"

		Rectangle
		{
			id: innerLeftEdge1

			anchors.top: parent.top
			anchors.right: parent.right
			anchors.left: parent.left
			height: 1

			color: root.leftEdges ? root.topHighlightColor : "transparent"
		}

		Rectangle
		{
			id: innerLeftEdge2

			anchors.bottom: parent.bottom
			anchors.right: parent.right
			anchors.left: parent.left
			height: 1

			color: root.leftEdges ? root.bottomHighlightColor : "transparent"
		}
	}
}