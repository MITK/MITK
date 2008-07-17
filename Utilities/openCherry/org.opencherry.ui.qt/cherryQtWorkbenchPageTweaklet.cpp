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

#include "cherryQtWorkbenchPageTweaklet.h"
#include <cherryQtSimpleWorkbenchWindow.h>

#include <QWidget>

namespace cherry
{

void* QtWorkbenchPageTweaklet::CreateClientComposite(IWorkbenchPage::Pointer page)
{
  QWidget* parent = static_cast<QWidget*>(page->GetWorkbenchWindow().Cast<QtWorkbenchWindow>()->GetPageComposite());
  QWidget* composite = new QWidget(parent);
  //composite.setVisible(false); // Make visible on activate.

  // force the client composite to be layed out
  //parent.layout();
  return composite;
}

}
