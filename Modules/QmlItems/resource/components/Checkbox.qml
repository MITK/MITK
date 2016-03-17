import QtQuick 2.5
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

CheckBox
{
    id: checkBox;

    style: CheckBoxStyle
    {
        indicator: Rectangle
        {
            implicitWidth: 16
            implicitHeight: 16
            radius: 3
            color: "#565656"
            border.color: "#282828"
            border.width: 1

            VectorIcon
            {
                visible: control.checked
                anchors.centerIn: parent;
                pixelSize: 10;
                icon: "\uf00c";
                color: "#fff";

                opacity: 1;
            }
        }

        label: Label
        {
            verticalAlignment: Qt.AlignVCenter
            anchors.left: parent.left
            anchors.leftMargin: 5

            text: checkBox.text;
            font.weight: Font.Normal;
            font.pixelSize: 12;
            color: checkBox.enabled ? "white" : "#666666";
        }
    }
}