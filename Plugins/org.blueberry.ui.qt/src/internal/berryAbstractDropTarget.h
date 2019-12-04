/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYABSTRACTDROPTARGET_H_
#define BERRYABSTRACTDROPTARGET_H_

#include "berryIDropTarget.h"

namespace berry {

struct AbstractDropTarget : public IDropTarget
{

    /*
     * @see IDropTarget#GetSnapRectangle()
     */
    QRect GetSnapRectangle() override
    {
        return QRect();
    }

    void DragFinished(bool  /*dropPerformed*/) override
    {

    }
};

}

#endif /* BERRYABSTRACTDROPTARGET_H_ */
