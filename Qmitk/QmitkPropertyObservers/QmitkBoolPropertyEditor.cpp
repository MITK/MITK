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
#include <QmitkBoolPropertyEditor.h>
#include <mitkRenderingManager.h>

QmitkBoolPropertyEditor::QmitkBoolPropertyEditor( const mitk::BoolProperty* property, QWidget* parent, const char* name )
: QmitkBoolPropertyView( property, parent, name )
{
  setEnabled( true );
  connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggle(bool)));
}

QmitkBoolPropertyEditor::~QmitkBoolPropertyEditor()
{
}

void QmitkBoolPropertyEditor::PropertyRemoved()
{
  QmitkBoolPropertyView::PropertyRemoved(); // inherited function

  setEnabled(false);
}

void QmitkBoolPropertyEditor::onToggle(bool on)
{
  if (m_BoolProperty)
  {
    BeginModifyProperty();  // deregister from events
  
    const_cast<mitk::BoolProperty*>(m_BoolProperty)->SetValue(on);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll(true);
  
    EndModifyProperty();  // again register for events
  }
}

