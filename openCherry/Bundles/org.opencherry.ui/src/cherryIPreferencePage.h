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

#ifndef CHERRYIPREFERENCEPAGE_H_
#define CHERRYIPREFERENCEPAGE_H_

#include "cherryObject.h"
#include "cherryIPreferences.h"

namespace cherry 
{

/**
 * \ingroup org_opencherry_ui
 *
 */
struct IPreferencePage : virtual public Object 
{

  cherryInterfaceMacro(IPreferencePage, cherry);

  ///
  /// Invoked when the OK button was clicked in the preferences dialog
  ///
  virtual bool PerformOk() = 0;

  ///
  /// Invoked when the Cancel button was clicked in the preferences dialog
  ///
  virtual void PerformCancel() = 0;

  ///
  /// Invoked when the user performed an import. As the values of the preferences may have changed
  /// you should read all values again from the preferences service.
  ///
  virtual void Update() = 0;
};

}

#endif /*CHERRYIPREFERENCEPAGE_H_*/
