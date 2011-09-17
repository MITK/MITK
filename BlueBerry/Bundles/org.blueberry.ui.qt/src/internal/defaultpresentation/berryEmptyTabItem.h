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


#ifndef BERRYEMPTYTABITEM_H_
#define BERRYEMPTYTABITEM_H_

#include "../util/berryAbstractTabItem.h"

namespace berry {

struct EmptyTabItem : public AbstractTabItem {

    /* (non-Javadoc)
     * @see AbstractTabItem#getBounds()
     */
    QRect GetBounds();

    /* (non-Javadoc)
     * @see AbstractTabItem#setInfo(org.blueberry.ui.internal.presentations.util.PartInfo)
     */
    void SetInfo(const PartInfo& info);

    /* (non-Javadoc)
     * @see AbstractTabItem#Dispose()
     */
    void Dispose();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.AbstractTabItem#getData()
     */
    Object::Pointer GetData();

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.AbstractTabItem#setData(java.lang.Object)
     */
    void SetData(Object::Pointer data);
};

}

#endif /* BERRYEMPTYTABITEM_H_ */
