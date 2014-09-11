import QtQuick 2.0
import QmlMitk 1.0

Item {
    anchors.fill: parent

    QmlMitkRenderWindowItem {
        id : renderwindownodeAxial
        objectName : "mitkRenderItemAxial"

        anchors.left : parent.left
        anchors.right : parent.horizontalCenter
        anchors.top : parent.top
        anchors.bottom : parent.verticalCenter
        anchors.margins : 0
    }

    Rectangle {
        color: Qt.rgba(1, 1, 1, 0.0)
        radius: 1; border.width: 3; border.color: "red"
        anchors.fill: renderwindownodeAxial
    }


    QmlMitkRenderWindowItem {
        id : renderwindownodeSagittal
        objectName : "mitkRenderItemSagittal"

        anchors.left : parent.horizontalCenter
        anchors.right : parent.right
        anchors.top : parent.top
        anchors.bottom : parent.verticalCenter
        anchors.margins : 0

    }

    Rectangle {
        color: Qt.rgba(1, 1, 1, 0.0)
        radius: 1; border.width: 3; border.color: "green"
        anchors.fill: renderwindownodeSagittal
    }

    QmlMitkRenderWindowItem {
        id : renderwindownodeFrontal
        objectName : "mitkRenderItemFrontal"

        anchors.left : parent.left
        anchors.right : parent.horizontalCenter
        anchors.top : parent.verticalCenter
        anchors.bottom : parent.bottom
        anchors.margins : 0

    }

    Rectangle {
        color: Qt.rgba(1, 1, 1, 0.0)
        radius: 1; border.width: 3; border.color: "blue"
        anchors.fill: renderwindownodeFrontal
    }

    QmlMitkRenderWindowItem {
        id : renderwindownode3D
        objectName : "mitkRenderItem3D"

        anchors.left : parent.horizontalCenter
        anchors.right : parent.right
        anchors.top : parent.verticalCenter
        anchors.bottom : parent.bottom
        anchors.margins : 0

    }

    Rectangle {
        color: Qt.rgba(1, 1, 1, 0.0)
        radius: 1; border.width: 3; border.color: "yellow"
        anchors.fill: renderwindownode3D
    }

}
