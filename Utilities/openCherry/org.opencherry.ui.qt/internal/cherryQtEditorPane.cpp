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

#include "cherryQtEditorPane.h"

#include <QHBoxLayout>

namespace cherry
{

QtEditorPane::QtEditorPane(IWorkbenchPartReference::Pointer partReference,
    WorkbenchPage::Pointer workbenchPage)
 : PartPane(partReference, workbenchPage)
{
  
}

QtEditorPane::~QtEditorPane()
{
  
}

void QtEditorPane::SetControlEnabled(bool enabled)
{
  m_Widget->setEnabled(enabled); 
}

void QtEditorPane::CreateControl(void* parent)
{
  if (this->GetControl() != 0) {
    return;
  }

  //partReference.addPropertyListener(this);
  //partReference.addPartPropertyListener(this);
  
  // Create editor form.
  m_Widget = new QWidget(static_cast<QWidget*>(parent));
  QHBoxLayout* layout = new QHBoxLayout(m_Widget);
  layout->setContentsMargins(0,0,0,0);
  m_Widget->setLayout(layout);
  
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
  
  control = m_Widget;
}

void QtEditorPane::CreateTitleBar()
{
  
}

void QtEditorPane::DoHide()
{
  
}

Rectangle QtEditorPane::GetBounds()
{
  return Rectangle();
}

void QtEditorPane::MoveAbove(void* refControl)
{
  
}

void QtEditorPane::ShowFocus(bool inFocus)
{
  
}

bool QtEditorPane::IsCloseable()
{
  return true;
}

bool QtEditorPane::GetControlVisible()
{
  return m_Widget->isVisible();
}

void QtEditorPane::SetControlVisible(bool visible)
{
  m_Widget->setVisible(visible);
}

}
