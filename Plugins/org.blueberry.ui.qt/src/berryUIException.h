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
