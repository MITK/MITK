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

    LevelWindow
    {
        id: levelWindow
        anchors.top: toolbar.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        width: 50
        color: "#4C4C4C"
        borderColor: "transparent"
        fontSize: 9
        fontColor: "white"

        onEnabledChanged:
        {
            if(enabled)
            levelWindow.width = 50
            else
            levelWindow.width = 0

        }

        Behavior on width
        {
            PropertyAnimation
            {
                duration: 120
            }
        }

        Mitk.TextField
        {
            id: window_field
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 3

            text: ""

            Keys.onPressed:
            {
                if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
                {
                    levelWindow.window = text
                }
            }

        }

        Mitk.TextField
        {
            id: level_field
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: window_field.top
            anchors.margins: 3

            text: ""

            Keys.onPressed:
            {
                if (event.key == Qt.Key_Enter || event.key == Qt.Key_Return)
                {
                    levelWindow.level = text
                }
            }
        }

    }

    Connections
    {
        target: levelWindow;

        onSync:
        {
            window_field.text = levelWindow.window;
            level_field.text = levelWindow.level;
        }
    }

    SplitView
    {
        id: splitView
        anchors.left: parent.left
        anchors.top: toolbar.bottom
        anchors.right: levelWindow.left
        anchors.bottom: parent.bottom
        orientation: Qt.Vertical

        /*
        handleDelegate: Rectangle
        {
        width: 3
        height: 3
        color: root.splitColor
    }
    */

    SplitView
    {

        height: parent.height/2
        orientation: Qt.Horizontal
        /*
        handleDelegate: Rectangle
        {
        width: 3
        height: 3
        color: root.splitColor
    }
    */

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
            color: "#E15944"
            text: "Axial"
        }

        Rectangle
        {
            anchors.fill: parent
            color: "transparent"
            border.width: 1
            border.color: "#E15944"
        }

    }

    ViewItem
    {
        id: viewer_sagittal;

        multiItem: stdmultiItem
        width: parent.width/2
        viewType: 2;

        Component.onCompleted:
        {
            viewer_sagittal.setupView();
        }

        Text
        {
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: 5
            anchors.bottomMargin: 5
            color: "#40B358"
            text: "Sagittal"
        }

        Rectangle
        {
            anchors.fill: parent
            color: "transparent"
            border.width: 1
            border.color: "#40B358"
        }
    }

}

SplitView
{

    height: parent.height/2
    orientation: Qt.Horizontal
    /*
    handleDelegate: Rectangle
    {
    width: 3
    height: 3
    color: root.splitColor
}
*/

ViewItem
{
    id: viewer_frontal;

    multiItem: stdmultiItem
    width: parent.width/2
    viewType: 1;

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
        color: "#0250AB"
        text: "Coronal"
    }

    Rectangle
    {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: "#0250AB"
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
        color: "#F0CA3E"
        text: "3D"
    }
    Rectangle
    {
        anchors.fill: parent
        color: "transparent"
        border.width: 1
        border.color: "#F0CA3E"

    }

}
}
}

Component.onCompleted:
{
    stdmultiItem.init()
}
}
