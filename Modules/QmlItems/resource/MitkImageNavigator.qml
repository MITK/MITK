import QtQuick 2.4
import QtQuick.Controls 1.4
import Mitk.Views 1.0
import "./components" as Mitk

ImageNavigator
{
	id: imageNavigator

	Row
	{
		id: worldCoordItem

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.margins: 5
		spacing: 5
		//height: 50

		Mitk.Spinbox
		{
			id: worldCoordX
			decimals: 2
			borderColor: "#40B358"

			onValueChanged:
			{
				imageNavigator.worldCoordinateX = value
			}
		}

		Mitk.Spinbox
		{
			id: worldCoordY
			decimals: 2
			borderColor: "#0250AB"

			onValueChanged:
			{
				imageNavigator.worldCoordinateY = value
			}
		}

		Mitk.Spinbox
		{
			id: worldCoordZ
			decimals: 2
			borderColor: "#E15944"

			onValueChanged:
			{
				imageNavigator.worldCoordinateZ = value
			}
		}
	}

	Column
	{
		anchors.left: parent.left
		anchors.top: worldCoordItem.bottom
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.topMargin: 5

		spacing: -3

        Mitk.SliderNavigator
		{
			id: navigator_axial
			height: parent.height/4

			text: "Axial"
		}

		Mitk.SliderNavigator
		{
			id: navigator_sagittal
			height: parent.height/4

			text: "Sagittal"
		}

		Mitk.SliderNavigator
		{
			id: navigator_cornal
			height: parent.height/4

			text: "Coronal"
		}

		Mitk.SliderNavigator
		{
			id: navigator_time
			height: parent.height/4

			text: "Time"
		}
	}

	Component.onCompleted:
	{
		imageNavigator.navigatorAxial = navigator_axial;
		imageNavigator.navigatorSagittal = navigator_sagittal;
		imageNavigator.navigatorCoronal = navigator_cornal;
		imageNavigator.navigatorTime = navigator_time;

		imageNavigator.initialize()
	}

	onSync:
	{

		worldCoordX.value = imageNavigator.worldCoordinateX;
		worldCoordY.value = imageNavigator.worldCoordinateY;
		worldCoordZ.value = imageNavigator.worldCoordinateZ;

		worldCoordX.minimumValue = imageNavigator.worldCoordinateXMin;
		worldCoordY.minimumValue = imageNavigator.worldCoordinateYMin;
		worldCoordZ.minimumValue = imageNavigator.worldCoordinateZMin;

		worldCoordX.maximumValue = imageNavigator.worldCoordinateXMax;
		worldCoordY.maximumValue = imageNavigator.worldCoordinateYMax;
		worldCoordZ.maximumValue = imageNavigator.worldCoordinateZMax;

	}
}