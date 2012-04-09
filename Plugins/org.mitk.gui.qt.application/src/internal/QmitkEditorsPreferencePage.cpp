/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-07 16:57:15 +0200 (Di, 07 Jul 2009) $
Version:   $Revision: 18019 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkEditorsPreferencePage.h"

#include <QVBoxLayout>

QmitkEditorsPreferencePage::QmitkEditorsPreferencePage()
: m_MainControl(0)
{
  
}

void QmitkEditorsPreferencePage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkEditorsPreferencePage::CreateQtControl(QWidget* parent)
{
  //empty page
  m_MainControl = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout;
  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkEditorsPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkEditorsPreferencePage::PerformOk()
{
  return true;
}

void QmitkEditorsPreferencePage::PerformCancel()
{

}

void QmitkEditorsPreferencePage::Update()
{
}
