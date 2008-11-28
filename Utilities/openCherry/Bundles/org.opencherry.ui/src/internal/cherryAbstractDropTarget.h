/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYABSTRACTDROPTARGET_H_
#define CHERRYABSTRACTDROPTARGET_H_

#include "cherryIDropTarget.h"

namespace cherry {

struct AbstractDropTarget : public IDropTarget
{

    /*
     * @see IDropTarget#GetSnapRectangle()
     */
    Rectangle GetSnapRectangle()
    {
        return Rectangle();
    }

    void DragFinished(bool dropPerformed)
    {

    }
};

}

#endif /* CHERRYABSTRACTDROPTARGET_H_ */
