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


#ifndef BERRYQTWORKBENCHPAGETWEAKLET_H_
#define BERRYQTWORKBENCHPAGETWEAKLET_H_

#include <berryWorkbenchPageTweaklet.h>

#include "../berryUiQtDll.h"

namespace berry
{

class BERRY_UI_QT QtWorkbenchPageTweaklet : public WorkbenchPageTweaklet
{

public:

  berryObjectMacro(QtWorkbenchPageTweaklet);

  void* CreateClientComposite(void* pageControl);
  void* CreatePaneControl(void* parent);

  Object::Pointer CreateStatusPart(void* parent, const std::string& title, const std::string& msg);
  IEditorPart::Pointer CreateErrorEditorPart(const std::string& partName, const std::string& msg);

};

}

#endif /* BERRYQTWORKBENCHPAGETWEAKLET_H_ */
