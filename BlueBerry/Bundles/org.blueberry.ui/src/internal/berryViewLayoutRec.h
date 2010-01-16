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

#ifndef BERRYVIEWLAYOUTREC_H_
#define BERRYVIEWLAYOUTREC_H_

#include <berryMacros.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Encapsulates the perspective layout information for a view.
 *
 * @since 3.0
 */
struct ViewLayoutRec : public Object {

  berryObjectMacro(ViewLayoutRec)

    bool isCloseable;

    bool isMoveable;

    bool isStandalone;

    bool showTitle;

    ViewLayoutRec() : isCloseable(true), isMoveable(true),
      isStandalone(false), showTitle(true) {}

    ViewLayoutRec(const ViewLayoutRec& rec)
     : Object(), isCloseable(rec.isCloseable), isMoveable(rec.isMoveable),
     isStandalone(rec.isStandalone), showTitle(rec.showTitle)
    { }
};

}

#endif /*BERRYVIEWLAYOUTREC_H_*/
