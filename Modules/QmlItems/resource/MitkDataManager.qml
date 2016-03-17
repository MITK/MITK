import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import Mitk.Views 1.0
import "./components" as Mitk

Item
{
	id: root
	property var tooltipArea;

	DataManager
	{
		id: dataManager
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
		anchors.fill: parent
		color: "#2E2E2E"

		ListView
		{
			id: list

			anchors.fill: parent
			model: dataStorage
			clip: true

			delegate: Component
			{
				Rectangle
				{
					id: listItem
					width: parent.width
					height: 30
					color: "#aa333333"

					MouseArea
					{
						anchors.fill: parent
						onClicked: list.currentIndex = index
					}

					Row
					{
						anchors.fill: parent
						anchors.leftMargin: 10

						spacing: 10
						height: parent.height

						Mitk.Checkbox
						{
							id: cb
							checked: true
							anchors.verticalCenter: parent.verticalCenter

							onCheckedChanged:
							{
								list.currentIndex = index
								dataManager.toggleVisibility(checked)
							}
						}

						Text
						{
							anchors.verticalCenter: parent.verticalCenter
							text: model.display
							color: "#ffffff"
							font.pointSize: 10
							font.weight: Font.DemiBold
						}
					}

					Rectangle
					{
						anchors.left: parent.left
						anchors.right: parent.right
						anchors.bottom: parent.bottom
						height: 1
						color: "#262626"
					}

					ListView.onAdd: list.currentIndex = index
				}
			}

			Mitk.DataToolbar
			{
				id: toolbar

				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: parent.bottom

				height: 22

				tooltipArea: root.tooltipArea
				datamanager: dataManager
			}

			highlight: Rectangle
			{
				color: "#006EFF"
			}

			highlightMoveDuration: 0
			highlightResizeDuration: 0

			onCurrentIndexChanged:
			{
				dataManager.index = list.currentIndex
			}

			onCountChanged:
			{
				if(list.count > 0)
					toolbar.enabled = true;
				else
					toolbar.enabled = false;
			}
		}
	}

	Rectangle
	{
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.bottom: parent.bottom

		width: 1
		color: "#242424"
	}
}
