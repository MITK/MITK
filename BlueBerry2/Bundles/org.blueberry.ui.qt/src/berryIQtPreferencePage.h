/*=========================================================================

Program:   BlueBerry Platform
Language:  C++
Date:      $Date: 2009-01-23 09:44:29 +0100 (Fr, 23 Jan 2009) $
Version:   $Revision: 16084 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef BERRYIQTPREFERENCEPAGE_H_
#define BERRYIQTPREFERENCEPAGE_H_

#ifdef __MINGW32__
// We need to include winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QWidget>

#include "berryIPreferencePage.h"

#include "berryUiQtDll.h"

namespace berry 
{

/**
 * \ingroup org_blueberry_ui
 *
 */
struct BERRY_UI_QT IQtPreferencePage : public IPreferencePage
{

  berryInterfaceMacro(IQtPreferencePage, berry);

  virtual void CreateQtControl(QWidget* parent) = 0;
  virtual QWidget* GetQtControl() const = 0;

protected:

  void CreateControl(void* parent);
  void* GetControl() const;

};

}

#endif /*BERRYIQTPREFERENCEPAGE_H_*/
