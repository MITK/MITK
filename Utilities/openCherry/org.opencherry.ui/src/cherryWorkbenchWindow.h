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

#ifndef CHERRYWORKBENCHWINDOW_H_
#define CHERRYWORKBENCHWINDOW_H_

#include "cherryIWorkbenchWindow.h"

#include "internal/cherryWorkbenchWindowConfigurer.h"

#include "application/cherryWorkbenchAdvisor.h"
#include "application/cherryWorkbenchWindowAdvisor.h"
#include "application/cherryActionBarAdvisor.h"

namespace cherry {

struct IWorkbench;
struct IWorkbenchPage;
struct IPartService;
struct ISelectionService;

class Workbench;

/**
 * \ingroup org_opencherry_ui
 * 
 */
class CHERRY_UI WorkbenchWindow : public IWorkbenchWindow
{
public:
  cherryClassMacro(WorkbenchWindow);
  
  WorkbenchWindow();
  
  void Init();
  
  SmartPointer<IWorkbenchPage> GetActivePage();
  
  SmartPointer<IWorkbench> GetWorkbench();
    
  IPartService* GetPartService();
    
  ISelectionService* GetSelectionService();
  
  
  virtual void* GetMenuManager() = 0;
  
  virtual bool SaveState(IMemento::Pointer memento);
  
protected:
  
  friend class WorkbenchConfigurer;
  friend class WorkbenchWindowConfigurer;
  
  /**
   * Creates the default contents and layout of the shell.
   * 
   * @param shell
   *            the shell
   */
  virtual void CreateDefaultContents(void* shell) = 0;
  
  /**
   * Returns the unique object that applications use to configure this window.
   * <p>
   * IMPORTANT This method is declared package-private to prevent regular
   * plug-ins from downcasting IWorkbenchWindow to WorkbenchWindow and getting
   * hold of the workbench window configurer that would allow them to tamper
   * with the workbench window. The workbench window configurer is available
   * only to the application.
   * </p>
   */
  WorkbenchWindowConfigurer::Pointer GetWindowConfigurer();
  
  /**
   * Fills the window's real action bars.
   * 
   * @param flags
   *            indicate which bars to fill
   */
  void FillActionBars(int flags);
  
  /**
   * The <code>WorkbenchWindow</code> implementation of this method
   * delegates to the window configurer.
   * 
   * @since 3.0
   */
  Point GetInitialSize();
  
private:
  
  /**
   * Constant indicating that all the actions bars should be filled.
   * 
   * @since 3.0
   */
  static const int FILL_ALL_ACTION_BARS;
  
  /**
   * Object for configuring this workbench window. Lazily initialized to an
   * instance unique to this window.
   * 
   * @since 3.0
   */
  WorkbenchWindowConfigurer::Pointer windowConfigurer;
  
  WorkbenchWindowAdvisor* windowAdvisor;

  ActionBarAdvisor::Pointer actionBarAdvisor;
  
  /**
   * Notifies interested parties (namely the advisor) that the window is about
   * to be opened.
   * 
   * @since 3.1
   */
  void FireWindowOpening();
  
  /**
   * Returns the workbench advisor. Assumes the workbench has been created
   * already.
   * <p>
   * IMPORTANT This method is declared private to prevent regular plug-ins
   * from downcasting IWorkbenchWindow to WorkbenchWindow and getting hold of
   * the workbench advisor that would allow them to tamper with the workbench.
   * The workbench advisor is internal to the application.
   * </p>
   */
  /* private - DO NOT CHANGE */
  WorkbenchAdvisor* GetAdvisor();

  /**
   * Returns the window advisor, creating a new one for this window if needed.
   * <p>
   * IMPORTANT This method is declared private to prevent regular plug-ins
   * from downcasting IWorkbenchWindow to WorkbenchWindow and getting hold of
   * the window advisor that would allow them to tamper with the window. The
   * window advisor is internal to the application.
   * </p>
   */
  /* private - DO NOT CHANGE */
  WorkbenchWindowAdvisor* GetWindowAdvisor();

  /**
   * Returns the action bar advisor, creating a new one for this window if
   * needed.
   * <p>
   * IMPORTANT This method is declared private to prevent regular plug-ins
   * from downcasting IWorkbenchWindow to WorkbenchWindow and getting hold of
   * the action bar advisor that would allow them to tamper with the window's
   * action bars. The action bar advisor is internal to the application.
   * </p>
   */
  /* private - DO NOT CHANGE */
  ActionBarAdvisor::Pointer GetActionBarAdvisor();

  /*
   * Returns the IWorkbench implementation.
   */
  SmartPointer<Workbench> GetWorkbenchImpl();
  
};

}

#endif /*CHERRYWORKBENCHWINDOW_H_*/
