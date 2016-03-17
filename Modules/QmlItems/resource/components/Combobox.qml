import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

ComboBox
{
    id: control;

    anchors.left: parent.left
    anchors.top: parent.top
    anchors.right: parent.right

    height: 20

    style: ComboBoxStyle
    {
        background: Rectangle
        {
            id: compBackground;

            width: parent.width;
            height: parent.height;
            color: "#565656";
            border.color: "#282828";
            border.width: 1;
            radius: 3;

            VectorIcon
            {
                id: icon;

                anchors.verticalCenter: parent.verticalCenter;
                anchors.right: parent.right;
                anchors.rightMargin: 6;
                font.pixelSize: 10;

                icon: "\uf0dc";
            }
        }

        label: Label
        {
            verticalAlignment: Qt.AlignVCenter
            anchors.left: parent.left
            anchors.leftMargin: 5

            text: control.currentText;
            font.weight: Font.Normal;
            font.pixelSize: 12;
            color: control.enabled ? "white" : "#666666";
        }

    }
}