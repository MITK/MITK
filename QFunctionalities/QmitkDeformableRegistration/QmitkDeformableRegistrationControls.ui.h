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

#include <stdio.h>
#include <stdlib.h>
#include "mitkRenderingManager.h"

void QmitkDeformableRegistrationControls::init()
{
  connect(this,SIGNAL(calculateDemonsRegistration()),qmitkDemonsRegistrationControls1,SLOT(CalculateTransformation()));
  this->CheckCalculateEnabled();
}

void QmitkDeformableRegistrationControls::ReinitFixedClicked()
{
  if (m_FixedSelector->GetSelectedNode() != NULL)
  {
    mitk::DataTreeNode* node = m_FixedSelector->GetSelectedNode();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      emit reinitFixed( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkDeformableRegistrationControls::ReinitMovingClicked()
{
    if (m_MovingSelector->GetSelectedNode() != NULL)
  {
    mitk::DataTreeNode* node = m_MovingSelector->GetSelectedNode();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      emit reinitMoving( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkDeformableRegistrationControls::GlobalReinitClicked()
{
  emit globalReinit( (dynamic_cast<mitk::DataTreeIteratorBase*> (m_FixedSelector->m_DataTreeIteratorClone.GetPointer())) );
}

void QmitkDeformableRegistrationControls::MovingImageOpacityChanged( int value )
{
  float fValue = ((float)value)/100.0f;
  emit opacityChanged(fValue);
}

void QmitkDeformableRegistrationControls::ShowImagesRedGreenSelected( bool show )
{
  emit showImagesRedGreen(show);
}

void QmitkDeformableRegistrationControls::CheckCalculateEnabled()
{
  if (m_MovingSelector->GetSelectedNode() != NULL && m_FixedSelector->GetSelectedNode() != NULL && m_DeformableTransform->label(m_DeformableTransform->currentPageIndex()) != "Preregister")
  {
    m_CalculateTransformation->setEnabled(true);
  }
  else
  {
    m_CalculateTransformation->setEnabled(false);
  }
}

void QmitkDeformableRegistrationControls::ShowBothImagesSelected()
{
  emit showBothImages();
}

void QmitkDeformableRegistrationControls::ShowFixedImageSelected()
{
  emit showFixedImage();
}

void QmitkDeformableRegistrationControls::ShowMovingImageSelected()
{
  emit showMovingImage();
}

void QmitkDeformableRegistrationControls::MovingImageChanged()
{
  this->CheckCalculateEnabled();
}

void QmitkDeformableRegistrationControls::FixedImageChanged()
{
  this->CheckCalculateEnabled();
}

void QmitkDeformableRegistrationControls::Calculate()
{
  qmitkDemonsRegistrationControls1->SetFixedNode(m_FixedSelector->GetSelectedNode());
  qmitkDemonsRegistrationControls1->SetMovingNode(m_MovingSelector->GetSelectedNode());
  if (m_DeformableTransform->label(m_DeformableTransform->currentPageIndex()) == "Demons")
  {
    emit calculateDemonsRegistration();
  }
}
