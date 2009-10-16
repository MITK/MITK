/*=========================================================================

Program:   openCherry Platform
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

#ifndef CHERRYIQTPREFERENCEPAGE_H_
#define CHERRYIQTPREFERENCEPAGE_H_

#ifdef __MINGW32__
// We need to inlclude winbase.h here in order to declare
// atomic intrinsics like InterlockedIncrement correctly.
// Otherwhise, they would be declared wrong within qatomic_windows.h .
#include <windows.h>
#endif

#include <QWidget>

#include "cherryIPreferencePage.h"

namespace cherry 
{

/**
 * \ingroup org_opencherry_ui
 *
 */
struct CHERRY_UI IQtPreferencePage : public IPreferencePage
{

  cherryInterfaceMacro(IQtPreferencePage, cherry);

  virtual void CreateQtControl(QWidget* parent) = 0;
  virtual QWidget* GetQtControl() const = 0;

protected:

  void CreateControl(void* parent);
  void* GetControl() const;

};

}

#endif /*CHERRYIQTPREFERENCEPAGE_H_*/
