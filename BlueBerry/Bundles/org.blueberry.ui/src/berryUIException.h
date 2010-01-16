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

#ifndef BERRYUIEXCEPTIONS_H_
#define BERRYUIEXCEPTIONS_H_

#include "berryUiDll.h"

#include <berryPlatformException.h>

namespace berry {

/**
 * \ingroup org_blueberry_ui
 * @{
 */

POCO_DECLARE_EXCEPTION(BERRY_UI, UIException, PlatformException);

POCO_DECLARE_EXCEPTION(BERRY_UI, WorkbenchException, UIException);

POCO_DECLARE_EXCEPTION(BERRY_UI, PartInitException, WorkbenchException);

/*@}*/
}

#endif /*BERRYUIEXCEPTIONS_H_*/
