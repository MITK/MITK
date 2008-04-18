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

#ifndef CHERRYWORKBENCH_H_
#define CHERRYWORKBENCH_H_

#include <org.opencherry.osgi/cherryDll.h>

#include "cherryIWorkbench.h"

namespace cherry {

class ViewRegistry;

class CHERRY_API Workbench : public IWorkbench
{
  
public:
 
  cherryClassMacro(Workbench);
  
  Workbench();
  virtual ~Workbench();
  
  IViewRegistry* GetViewRegistry();
  //IEditorRegistry* GetEditorRegistry();
  
  
  void ShowPerspective(const std::string& perspectiveId);
  
private:
  
  ViewRegistry* m_ViewRegistry;
 
};

} // namespace cherry

#endif /*CHERRYWORKBENCH_H_*/
