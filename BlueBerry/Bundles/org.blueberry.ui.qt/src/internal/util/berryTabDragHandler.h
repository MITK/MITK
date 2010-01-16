/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYTABDRAGHANDLER_H_
#define BERRYTABDRAGHANDLER_H_

#include <berryStackDropResult.h>

#include <QWidget>
#include <QPoint>

namespace berry {

struct TabDragHandler {

    virtual ~TabDragHandler() {}
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
