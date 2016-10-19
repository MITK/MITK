import QtQuick 2.5
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3

TextField
{
    id: textField;

    style: TextFieldStyle
    {
        textColor: "white"
        font.weight: Font.Normal;
        font.pixelSize: 12;
        background: Rectangle
        {
            radius: 2
            implicitWidth: 100
            implicitHeight: 20
            color: "#565656"
            border.color: "#282828"
            border.width: 1
        }
    }
}