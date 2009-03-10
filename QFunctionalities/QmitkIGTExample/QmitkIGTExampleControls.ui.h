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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkVector.h"
#include "qstring.h"
#include "qvalidator.h"


void QmitkIGTExampleControls::init()
{
  QDoubleValidator*  v = new QDoubleValidator(-10000.0, 10000.0, 3, this);
  m_X->setValidator(v);
  m_Y->setValidator(v);
  m_Z->setValidator(v);
}


void QmitkIGTExampleControls::m_ChangeOffset_clicked()
{
  mitk::Vector3D v;
  v[0] = m_X->text().toFloat();
  v[1] = m_Y->text().toFloat();
  v[2] = m_Z->text().toFloat();
  if (m_Parameters.IsNull())
    m_Parameters = mitk::PropertyList::New();
  m_Parameters->SetProperty("NavigationDataDisplacementFilter_Offset", mitk::Vector3DProperty::New(v));
  emit ParametersChanged();
}


void QmitkIGTExampleControls::SetDisplacementFilterParameters( mitk::PropertyList * p )
{
  if (p == NULL)
    return;
  mitk::Vector3D v;
  if (p->GetPropertyValue<mitk::Vector3D>("NavigationDataDisplacementFilter_Offset", v) == true) 
  {
    m_X->setText(QString::number(v[0]));
    m_Y->setText(QString::number(v[1]));
    m_Z->setText(QString::number(v[2]));    
  }
}
