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

#include "QmitkExtPreferencePage.h"

#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QCheckBox>
#include <QHashIterator>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <mitkIInputDeviceRegistry.h>
#include <mitkIInputDeviceDescriptor.h>
#include <mitkCoreExtConstants.h>

QmitkExtPreferencePage::QmitkExtPreferencePage()
: m_MainControl(0)
{
  
}

QmitkExtPreferencePage::QmitkExtPreferencePage(const QmitkExtPreferencePage& other)
{
  Q_UNUSED(other)
  throw std::runtime_error("Copy constructor not implemented");
}

void QmitkExtPreferencePage::Init(berry::IWorkbench::Pointer )
{
}

void QmitkExtPreferencePage::CreateQtControl(QWidget* parent)
{
  //empty page
  m_MainControl = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout;
  m_MainControl->setLayout(layout);
  this->Update();
}

QWidget* QmitkExtPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool QmitkExtPreferencePage::PerformOk()
{
  return true;
}

void QmitkExtPreferencePage::PerformCancel()
{

}

void QmitkExtPreferencePage::Update()
{
}
