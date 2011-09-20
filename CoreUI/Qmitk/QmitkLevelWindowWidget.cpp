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
#include "QmitkLevelWindowWidget.h"
#include "QmitkSliderLevelWindowWidget.h"


QmitkLevelWindowWidget::QmitkLevelWindowWidget(QWidget* parent, Qt::WindowFlags f)
 : QWidget(parent, f)
{
  this->setupUi(this);
  
  m_Manager = mitk::LevelWindowManager::New();

  SliderLevelWindowWidget->setLevelWindowManager(m_Manager.GetPointer());
  LineEditLevelWindowWidget->setLevelWindowManager(m_Manager.GetPointer());
} 


void QmitkLevelWindowWidget::SetDataStorage( mitk::DataStorage* ds ) 
{ 
  m_Manager->SetDataStorage(ds); 
} 


mitk::LevelWindowManager* QmitkLevelWindowWidget::GetManager()
{
  return m_Manager.GetPointer();
}

void QmitkLevelWindowWidget::SetIntensityRangeType(QmitkLineEditLevelWindowWidget::IntensityRangeType intensityRangeType)
{
  LineEditLevelWindowWidget->SetIntensityRangeType(intensityRangeType);
}

void QmitkLevelWindowWidget::SetExponentialFormat(bool value)
{
  LineEditLevelWindowWidget->SetExponentialFormat(value);
}

void QmitkLevelWindowWidget::SetPrecision(int precision)
{
  LineEditLevelWindowWidget->SetPrecision(precision);
}
