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
    StackDropResult::Pointer DragOver(QWidget* currentControl, const Point& location,
            int dragStart);

    /* (non-Javadoc)
     * @see org.eclipse.ui.internal.presentations.util.TabDragHandler#getInsertionPosition(java.lang.Object)
     */
    int GetInsertionPosition(Object::Pointer cookie);

};

}

#endif /* BERRYREPLACEDRAGHANDLER_H_ */
