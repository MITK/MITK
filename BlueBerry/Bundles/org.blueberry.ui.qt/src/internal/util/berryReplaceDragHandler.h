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


#ifndef BERRYREPLACEDRAGHANDLER_H_
#define BERRYREPLACEDRAGHANDLER_H_

#include "berryTabDragHandler.h"

#include "berryAbstractTabFolder.h"

namespace berry {

class ReplaceDragHandler : public TabDragHandler {

private:

  struct DragCookie : public Object {
        int insertPosition;

        DragCookie(int pos);
    };

    AbstractTabFolder* tabFolder;

  public:

    ReplaceDragHandler(AbstractTabFolder* folder);

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.presentations.util.TabDragHandler#dragOver(org.eclipse.swt.widgets.Control, org.eclipse.swt.graphics.Point)
     */
    StackDropResult::Pointer DragOver(QWidget*, const Point& location,
            int dragStart);

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.presentations.util.TabDragHandler#getInsertionPosition(java.lang.Object)
     */
    int GetInsertionPosition(Object::Pointer cookie);

};

}

#endif /* BERRYREPLACEDRAGHANDLER_H_ */
