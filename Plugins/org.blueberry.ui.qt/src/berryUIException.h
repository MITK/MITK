/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYUIEXCEPTIONS_H_
#define BERRYUIEXCEPTIONS_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryPlatformException.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui_qt
 * @{
 */

CTK_DECLARE_EXCEPTION(BERRY_UI_QT, UIException, PlatformException)

CTK_DECLARE_EXCEPTION(BERRY_UI_QT, WorkbenchException, UIException)

CTK_DECLARE_EXCEPTION(BERRY_UI_QT, PartInitException, WorkbenchException)

/*@}*/
}

#endif /*BERRYUIEXCEPTIONS_H_*/
