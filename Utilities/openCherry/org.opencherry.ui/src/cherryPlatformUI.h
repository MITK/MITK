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

#ifndef CHERRYPLATFORMUI_H_
#define CHERRYPLATFORMUI_H_

#include "cherryUiDll.h"
#include "cherryWorkbench.h"
#include "application/cherryWorkbenchAdvisor.h"

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 */
class CHERRY_UI PlatformUI
{
public:
  
  static const std::string PLUGIN_ID;
  static const std::string XP_WORKBENCH;
  static const std::string XP_VIEWS;
  
  /**
   * Return code (value 0) indicating that the workbench terminated normally.
   * 
   * @see #CreateAndRunWorkbench
   * @since 3.0
   */
  static const int RETURN_OK;

  /**
   * Return code (value 1) indicating that the workbench was terminated with
   * a call to <code>IWorkbench.restart</code>.
   * 
   * @see #CreateAndRunWorkbench
   * @see IWorkbench#Restart
   * @since 3.0
   */
  static const int RETURN_RESTART;

  /**
   * Return code (value 2) indicating that the workbench failed to start.
   * 
   * @see #CreateAndRunWorkbench
   * @see IWorkbench#Restart
   * @since 3.0
   */
  static const int RETURN_UNSTARTABLE;

  /**
   * Return code (value 3) indicating that the workbench was terminated with
   * a call to IWorkbenchConfigurer#emergencyClose.
   * 
   * @see #CreateAndRunWorkbench
   * @since 3.0
   */
  static const int RETURN_EMERGENCY_CLOSE;
  
  
  static int CreateAndRunWorkbench(WorkbenchAdvisor* advisor);
  
  static IWorkbench::Pointer GetWorkbench();
  
  static bool IsWorkbenchRunning();
  
private:
  static Workbench::Pointer m_Workbench;
};

}

#endif /*CHERRYPLATFORMUI_H_*/
