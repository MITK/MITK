/*=========================================================================

 Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkExtAppWorkbenchAdvisor.h"
#include "internal/QmitkExtApplicationPlugin.h"

#include <QmitkExtWorkbenchWindowAdvisor.h>

const std::string QmitkExtAppWorkbenchAdvisor::DEFAULT_PERSPECTIVE_ID =
    "org.mitk.extapp.defaultperspective";

void
QmitkExtAppWorkbenchAdvisor::Initialize(berry::IWorkbenchConfigurer::Pointer configurer)
{
  berry::QtWorkbenchAdvisor::Initialize(configurer);

  configurer->SetSaveAndRestore(true);
}

berry::WorkbenchWindowAdvisor*
QmitkExtAppWorkbenchAdvisor::CreateWorkbenchWindowAdvisor(
        berry::IWorkbenchWindowConfigurer::Pointer configurer)
{
  QmitkExtWorkbenchWindowAdvisor* advisor = new
    QmitkExtWorkbenchWindowAdvisor(this, configurer);

  // Exclude the help perspective from org.blueberry.ui.qt.help from
  // the normal perspective list.
  // The perspective gets a dedicated menu entry in the help menu
  std::vector<std::string> excludePerspectives;
  excludePerspectives.push_back("org.blueberry.perspectives.help");
  advisor->SetPerspectiveExcludeList(excludePerspectives);

  advisor->SetWindowIcon(":/QmitkExtApplication/icon_research.xpm");
  return advisor;
  //return new QmitkExtWorkbenchWindowAdvisor(this, configurer);
}

std::string QmitkExtAppWorkbenchAdvisor::GetInitialWindowPerspectiveId()
{
  return DEFAULT_PERSPECTIVE_ID;
}
