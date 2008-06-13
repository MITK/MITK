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

#include "cherryQtViewPane.h"

#include <QVBoxLayout>

namespace cherry {

QtViewPane::QtViewPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
 : PartPane(partReference, workbenchPage)
{
  
}

QtViewPane::~QtViewPane()
{
  
}

void QtViewPane::SetControlEnabled(bool enabled)
{
  m_Control->setEnabled(enabled); 
}

void QtViewPane::CreateControl(void* parent)
{
  if (this->GetControl() != 0) {
    return;
  }

  //partReference.addPropertyListener(this);
  //partReference.addPartPropertyListener(this);
  
  // Create view form.  
  m_DockWidget = new QDockWidget(static_cast<QWidget*>(parent));
  m_Control = new QWidget(m_DockWidget);
  m_DockWidget->setWidget(m_Control);
  
  // the part should never be visible by default.  It will be made visible 
  // by activation.  This allows us to have views appear in tabs without 
  // becoming active by default.
  //m_DockWidget->setVisible(false);
  //control.moveAbove(null);
  
  // Create a title bar.
  this->CreateTitleBar();

  
  // When the pane or any child gains focus, notify the workbench.
  //control.addListener(SWT.Activate, this);

  //control.addTraverseListener(traverseListener);
  
  control = m_Control;
}

void QtViewPane::CreateTitleBar()
{
  m_DockWidget->setWindowTitle(this->GetPartReference()->GetPartName().c_str());
  m_DockWidget->setToolTip(this->GetPartReference()->GetTitleToolTip().c_str());
}

void QtViewPane::DoHide()
{
  
}

Rectangle QtViewPane::GetBounds()
{
  return Rectangle();
}

void QtViewPane::MoveAbove(void* refControl)
{
  
}

void QtViewPane::ShowFocus(bool inFocus)
{
  
}

bool QtViewPane::IsCloseable()
{
  return true;
}

bool QtViewPane::GetControlVisible()
{
  return m_Control->isVisible();
}

void QtViewPane::SetControlVisible(bool visible)
{
  m_Control->setVisible(visible);
}

}
