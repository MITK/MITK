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


#ifndef BERRYEMPTYTABITEM_H_
#define BERRYEMPTYTABITEM_H_

#include "internal/util/berryAbstractTabItem.h"

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
