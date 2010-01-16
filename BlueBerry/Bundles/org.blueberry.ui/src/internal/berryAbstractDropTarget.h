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
