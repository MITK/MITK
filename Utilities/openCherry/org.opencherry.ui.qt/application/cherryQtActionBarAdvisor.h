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

#ifndef CHERRYQTACTIONBARADVISOR_H_
#define CHERRYQTACTIONBARADVISOR_H_

#include <application/cherryActionBarAdvisor.h>

#include "../cherryUiQtDll.h"

#include <QMenuBar>

namespace cherry
{

/**
 * Qt specific subclass of ActionBarAdvisor.
 * <p>
 * Qt applications should subclass this class and override
 * the Qt specific methods to configure the Qt workbench.
 * </p>
 */
class CHERRY_UI_QT QtActionBarAdvisor : public ActionBarAdvisor
{
  
public:
  
  cherryClassMacro(QtActionBarAdvisor);
  
  QtActionBarAdvisor(IActionBarConfigurer::Pointer configurer);
  
protected:
  
  /**
   * Overrides ActionBarAdvisor#FillMenuBar(void*)
   * <p>
   * This method casts the menuBar to a QMenuBar
   * and calls #FillQtMenuBar(QMenuBar*).
   * </p>
   * 
   * @param menuBar the menu manager for the menu bar
   */
  void FillMenuBar(void* menuBar);
  
  /**
   * @see ActionBarAdvisor#FillMenuBar(void*)
   * 
   * The default implementation does nothing.
   */
  virtual void FillQtMenuBar(QMenuBar* menuBar);
};

}
#endif /*CHERRYQTACTIONBARADVISOR_H_*/
