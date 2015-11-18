import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0
import Mitk.Views 1.0
import "./components" as Mitk

Rectangle
{
    id: root;
    property var tooltipArea;
    property color splitColor: "#4C4C4C";
    
    MultiItem
    {
        id: stdmultiItem
    }

    Mitk.MultiToolbar
    {
        id: toolbar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top

        height: 15

        tooltipArea: root.tooltipArea
        multiItem: stdmultiItem
    }
    
    SplitView 
    {
        //anchors.margins: 2
        anchors.left: parent.left
        anchors.top: toolbar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        orientation: Qt.Vertical

        handleDelegate: Rectangle
        {
            width: 3
            height: 3
            color: root.splitColor
        }
        
        SplitView
        {

            height: parent.height/2
            orientation: Qt.Horizontal
            handleDelegate: Rectangle
            {
                width: 3
                height: 3
                color: root.splitColor
            }
            
            ViewItem
            {
                id: viewer_axial;
                
                multiItem: stdmultiItem
                width: parent.width/2
                viewType: 0;
                
                Component.onCompleted:
                {
                    viewer_axial.setupView();
                }

                Text
                {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 5
                    anchors.bottomMargin: 5
                    color: "red"
                    text: "Axial"
                }

                Rectangle
                {
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 1
                    border.color: "red"
                }

            }
            
            ViewItem
            {
                id: viewer_saggital;

                multiItem: stdmultiItem
                width: parent.width/2
                viewType: 1;

                Component.onCompleted:
                {
                    viewer_saggital.setupView();
                }

                Text
                {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 5
                    anchors.bottomMargin: 5
                    color: "green"
                    text: "Saggital"
                }

                Rectangle
                {
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 1
                    border.color: "green"
                }
            }
            
        }

        SplitView
        {

            height: parent.height/2
            orientation: Qt.Horizontal
            handleDelegate: Rectangle
            {
                width: 3
                height: 3
                color: root.splitColor
            }
            
            ViewItem
            {
                id: viewer_frontal;
                
                multiItem: stdmultiItem
                width: parent.width/2
                viewType: 2;
                
                Component.onCompleted:
                {
                    viewer_frontal.setupView();
                }

                Text
                {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 5
                    anchors.bottomMargin: 5
                    color: "blue"
                    text: "Coronal"
                }

                Rectangle
                {
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 1
                    border.color: "blue"
                }
            }
            
            ViewItem
            {
                id: viewer_default;
                
                multiItem: stdmultiItem
                width: parent.width/2
                viewType: 3;
                
                Component.onCompleted:
                {
                    viewer_default.setupView();
                }
                Text
                {
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 5
                    anchors.bottomMargin: 5
                    color: "yellow"
                    text: "3D"
                }

                Rectangle
                {
                    anchors.fill: parent
                    color: "transparent"
                    border.width: 1
                    border.color: "yellow"
                }
            }        
        }
    }

    Component.onCompleted:
    {
        stdmultiItem.init()
    }    
}
