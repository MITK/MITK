/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYWORKBENCHPAGETWEAKLET_H_
#define CHERRYWORKBENCHPAGETWEAKLET_H_

#include "../internal/cherryTweaklets.h"
#include "../cherryIWorkbenchPage.h"

namespace cherry
{

struct CHERRY_UI WorkbenchPageTweaklet : public Object
{
  cherryInterfaceMacro(WorkbenchPageTweaklet, cherry);

  static Tweaklets::TweakKey<WorkbenchPageTweaklet> KEY;

  virtual void* CreateClientComposite(void* pageControl) = 0;
  virtual void* CreatePaneControl(void* parent) = 0;

  virtual IViewPart::Pointer CreateErrorViewPart(const std::string& partName = "", const std::string& msg = "") = 0;
  virtual IEditorPart::Pointer CreateErrorEditorPart(const std::string& partName = "", const std::string& msg = "") = 0;

};

}

#endif /* CHERRYWORKBENCHPAGETWEAKLET_H_ */
