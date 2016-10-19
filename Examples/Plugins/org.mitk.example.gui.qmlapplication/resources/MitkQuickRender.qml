import QtQuick 2.4
import QtQuick.Window 2.2
import QtGraphicalEffects 1.0
import Mitk.Views 1.0
import "./views" as Mitk

Window
{
	id: window;

	visible: true
	width: 1000
	height: 600

	minimumWidth: 800
	minimumHeight: 600

	color: "#333333"

	Workbench
	{
		id: workbench

		anchors.fill: parent

		Mitk.Toolbar
		{
			id: toolbar

			anchors.left: parent.left
			anchors.top: parent.top
			anchors.right: parent.right

			color: "#333333"
			height: 50

			workbench: workbench
			tooltipArea: footer
		}

		MitkDataManager
		{
			id: dataManager

			anchors.left: parent.left
			anchors.top: toolbar.bottom
			anchors.bottom: sliderNavigator.top
			tooltipArea: footer

			width: 240
		}

		MitkImageNavigator
		{
			id: sliderNavigator

			anchors.left: parent.left
			anchors.bottom: footer.top
			width: 240
			height: 120
		}

		MitkProperties
		{
			id: mitkproperties

			anchors.right: parent.right
			anchors.top: toolbar.bottom
			anchors.bottom: footer.top
			tooltipArea: footer

			width: 0
		}

		MitkStdMultiItem
		{
			id: multiItem

			anchors.left: dataManager.right
			anchors.top: toolbar.bottom
			anchors.right: mitkproperties.left
			anchors.bottom: footer.top
			tooltipArea: footer

			color: "#222222"
		}

		Mitk.Footer
		{
			id: footer

			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom

			height: 20
		}
	}
}
