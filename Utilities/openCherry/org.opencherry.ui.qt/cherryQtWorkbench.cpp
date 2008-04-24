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

#include "cherryQtWorkbench.h"

#include "internal/cherryQtShowViewDialog.h"
#include "internal/cherryQtViewPane.h"

#include <org.opencherry.ui/cherryPlatformUI.h>

namespace cherry {

QtWorkbench::~QtWorkbench()
{
}

IWorkbenchWindow::Pointer QtWorkbench::GetActiveWorkbenchWindow()
{
  return m_QtWindow;
}

void
QtWorkbench::Run()
{
  m_QtWindow = new QtWorkbenchWindow();
    
  m_QtWindow->show();
}

IDialog::Pointer
QtWorkbench::CreateDialog(const std::string& dialogid)
{
  if (dialogid == DIALOG_ID_SHOW_VIEW)
    return new QtShowViewDialog(PlatformUI::GetWorkbench()->GetViewRegistry());
  else
    return IDialog::Pointer(0);
}

IViewPart::Pointer QtWorkbench::CreateErrorViewPart(const std::string& partName, Poco::Exception exc)
{
  return 0;
}

IViewPane::Pointer QtWorkbench::CreateViewPane()
{
  return new QtViewPane("", m_QtWindow);
}

}  // namespace cherry
