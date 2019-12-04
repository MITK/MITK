/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
    StackDropResult::Pointer DragOver(QWidget*, const QPoint& location,
            int dragStart) override;

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.presentations.util.TabDragHandler#getInsertionPosition(java.lang.Object)
     */
    int GetInsertionPosition(Object::Pointer cookie) override;

};

}

#endif /* BERRYREPLACEDRAGHANDLER_H_ */
