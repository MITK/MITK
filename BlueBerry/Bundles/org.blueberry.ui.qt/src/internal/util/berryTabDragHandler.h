/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYTABDRAGHANDLER_H_
#define BERRYTABDRAGHANDLER_H_

#include <berryStackDropResult.h>

#include <QWidget>
#include <QPoint>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct BERRY_UI_QT TabDragHandler {

    virtual ~TabDragHandler();

    /**
     * Returns the StackDropResult for the location being dragged over.
     *
     * @param currentControl control being dragged over
     * @param location mouse position (display coordinates)
     * @param initialTab the index of the tab in this stack being dragged,
     *      or -1 if dragging a tab from another stack.
     * @return the StackDropResult for this drag location
     */
    virtual StackDropResult::Pointer DragOver(QWidget* currentControl,
            const Point& location, int initialTab) = 0;

    virtual int GetInsertionPosition(Object::Pointer cookie) = 0;
};

}

#endif /* BERRYTABDRAGHANDLER_H_ */
