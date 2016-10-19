import QtQuick 2.4

Text
{
    id: root;

    property var toolArea;
    property string icon: "-";
    property string tooltip: "-";
    property int fadeDuration: 100;
    property int pixelSize: 15;
    property int clickMargin: 0;
    property color defaultColor: "#EFEFEF";
    property color disableColor: "#333333"
    property bool enabled: false;

    signal clicked;

    FontLoader
    {
        id: fontLoader;
        source: "../../fonts/font-awesome/fontawesome-webfont.ttf";
    }

    textFormat:             Text.PlainText;
    style:                  Text.Normal;
    color:                  !root.enabled ? root.disableColor : mouseArea.pressed ? Qt.darker(root.defaultColor, 1.5) : root.defaultColor
    font.family:            fontLoader.name;
    font.pixelSize:         root.pixelSize;
    verticalAlignment:      Text.AlignVCenter;
    text:                   root.icon;

    Behavior on color
    {
        ColorAnimation
        {
            duration: root.fadeDuration;
        }
    }

    MouseArea
    {
        id: mouseArea;

        anchors.fill: parent;

        hoverEnabled: true;

        onClicked:
        {
            root.clicked();
        }

        onEntered:
        {
            if(root.toolArea)
                root.toolArea.text = root.tooltip;
        }

        onExited:
        {
            if(root.toolArea)
                root.toolArea.text = "";
        }
    }
}
