import QtQuick 2.5

Item
{
    id: root

    property real minimumValue: 0;
    property real maximumValue: 100;
    property real valueLower: minimumValue;
    property real valueUpper: minimumValue;

    width: 200
    height: 6

    onMinimumValueChanged: updatePos();
    onMaximumValueChanged: updatePos();
    onValueLowerChanged: updatePos();
    onValueUpperChanged: updatePos();

    signal valueUpperChangedByHandle;
    signal valueLowerChangedByHandle;

    function updatePos()
    {
        if(slider.width < 1)
        return

        if(maximumValue > minimumValue)
        {
            var posLower = (valueLower-minimumValue)*slider.width/(maximumValue-minimumValue)- btn1.width/2
            var posUpper = (valueUpper-minimumValue)*slider.width/(maximumValue-minimumValue)-btn2.width/2

            btn1.x = posLower
            btn2.x = posUpper
        }
    }

    onWidthChanged:
    {
        updatePos();
    }

    Rectangle
    {
        id: slider
        width: parent.width
        height: parent.height
        anchors.centerIn: parent
        color: "gray"
        /*
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#C0C0C0" }
            GradientStop { position: 1.0; color: "#CCCCCC" }
        }
        */

        border.width: 1
        border.color: "#888888"

        radius: 3

        Item
        {
            id: sliderItem

            Rectangle
            {
                height: slider.height
                anchors.left: btn1.left
                anchors.right: btn2.right

                border.width: 1
                border.color: "#387FB8"

                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#69BAFB" }
                    GradientStop { position: 1.0; color: "#55A8EB"}
                }
            }

            Rectangle
            {
                id: btn1

                width: 16
                height: 16
                radius: 2
                color: "#565656"

                y: (slider.height-width)/2

                border.width: 1
                border.color: "#282828"

                MouseArea
                {
                    id: btn1MouseArea
                    anchors.fill: parent
                    drag.axis: Drag.XAxis
                    drag.target: parent
                    drag.minimumX: -btn1.width/2
                    drag.maximumX: btn2.x
                    drag.threshold: 0

                    onPositionChanged:
                    {
                        valueLower = Math.max(minimumValue, Math.min(maximumValue, (maximumValue - minimumValue) * (btn1.x + btn1.width/2)/slider.width + minimumValue))
                        root.valueLowerChangedByHandle();
                    }
                }
            }

            Rectangle
            {
                id: btn2

                width: 16
                height: 16
                radius: 2
                color: "#565656"

                y: (slider.height-width)/2

                border.width: 1
                border.color: "#282828"

                MouseArea
                {
                    id: btn2MouseArea
                    anchors.fill: parent
                    drag.axis: Drag.XAxis
                    drag.target: parent
                    drag.minimumX: btn1.x
                    drag.maximumX: slider.width - btn2.width/2

                    drag.threshold: 0
                    onPositionChanged:
                    {
                        valueUpper = Math.max(minimumValue, Math.min(maximumValue, (maximumValue - minimumValue) * (btn2.x + btn2.width/2)/slider.width + minimumValue))
                        root.valueUpperChangedByHandle();
                    }
                }
            }


        }
    }
}