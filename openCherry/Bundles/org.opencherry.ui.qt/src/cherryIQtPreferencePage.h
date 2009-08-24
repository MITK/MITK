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

#include <QWidget>

#include "cherryIPreferencePage.h"

namespace cherry 
{

/**
 * \ingroup org_opencherry_ui
 *
 */
struct IQtPreferencePage : public QWidget, public IPreferencePage 
{

  cherryInterfaceMacro(IQtPreferencePage, cherry);

public:
  IQtPreferencePage(QWidget * parent = 0, Qt::WindowFlags f = 0)
    : QWidget(parent, f)
  {
  }

  ///
  /// Invoked when the OK button was clicked in the preferences dialog
  ///
  virtual bool PerformOk() = 0;

  ///
  /// Invoked when the Cancel button was clicked in the preferences dialog
  ///
  virtual void PerformCancel() = 0;
};

}

#endif /*CHERRYIQTPREFERENCEPAGE_H_*/
