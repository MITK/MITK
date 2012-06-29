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


#ifndef BERRYABSTRACTDROPTARGET_H_
#define BERRYABSTRACTDROPTARGET_H_

#include "berryIDropTarget.h"

namespace berry {

struct AbstractDropTarget : public IDropTarget
{

    /*
     * @see IDropTarget#GetSnapRectangle()
     */
    Rectangle GetSnapRectangle()
    {
        return Rectangle();
    }

    void DragFinished(bool  /*dropPerformed*/)
    {

    }
};

}

#endif /* BERRYABSTRACTDROPTARGET_H_ */
