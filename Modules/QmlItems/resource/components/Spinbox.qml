import QtQuick 2.5
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

SpinBox
{
    id: root
    property color borderColor;

    style: SpinBoxStyle
    {
        id: spinStyle
        textColor: "white"
        font.pixelSize: 12;
        background: Rectangle
        {
            implicitWidth: 70
            implicitHeight: 20
            border.width: 1
            border.color: root.borderColor
            color: "#565656"
            radius: 2
        }
    }
}