/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  berryObjectMacro(ViewLayoutRec);

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
