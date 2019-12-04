/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYEMPTYTABITEM_H_
#define BERRYEMPTYTABITEM_H_

#include "internal/util/berryAbstractTabItem.h"

namespace berry {

struct EmptyTabItem : public AbstractTabItem {

    /* (non-Javadoc)
     * @see AbstractTabItem#getBounds()
     */
    QRect GetBounds() override;

    /* (non-Javadoc)
     * @see AbstractTabItem#setInfo(org.blueberry.ui.internal.presentations.util.PartInfo)
     */
    void SetInfo(const PartInfo& info) override;

    /* (non-Javadoc)
     * @see AbstractTabItem#Dispose()
     */
    void Dispose() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.AbstractTabItem#getData()
     */
    Object::Pointer GetData() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.internal.presentations.util.AbstractTabItem#setData(java.lang.Object)
     */
    void SetData(Object::Pointer data) override;
};

}

#endif /* BERRYEMPTYTABITEM_H_ */
