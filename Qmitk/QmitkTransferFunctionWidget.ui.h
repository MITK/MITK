/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include "mitkTransferFunctionProperty.h"
#include <mitkDataTreeNode.h>
#include <mitkRenderingManager.h>

/**
 * \class QmitkTransferFunctionWidget
 * \brief A custom widget for editing transfer functions for volume rendering. 
 * \ingroup Widgets
 */ 
void QmitkTransferFunctionWidget::SetDataTreeNode( mitk::DataTreeNode* node)
{
  if (node) {
    if (mitk::TransferFunctionProperty::Pointer tfp = dynamic_cast<mitk::TransferFunctionProperty*>(node->GetProperty("TransferFunction").GetPointer()))
    {
      if (tfp)
      {
        mitk::TransferFunction* tf = dynamic_cast<mitk::TransferFunction*>(tfp->GetValue().GetPointer());
        if (tf)
        {
          std::cout << "TF access" << std::endl;
          m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction(tf->GetScalarOpacityFunction());
          m_ScalarOpacityFunctionCanvas->SetHistogram(tf->GetHistogram());
          m_ColorTransferFunctionCanvas->SetColorTransferFunction(tf->GetColorTransferFunction());
          UpdateMinMaxLabels();
        }
      }
    }
    else 
    {
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      if (mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData())) {
        tf->InitializeByMitkImage(image);
        m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction(tf->GetScalarOpacityFunction());
        m_ScalarOpacityFunctionCanvas->SetHistogram(tf->GetHistogram());
        m_ColorTransferFunctionCanvas->SetColorTransferFunction(tf->GetColorTransferFunction());
        UpdateMinMaxLabels();
        node->SetProperty("TransferFunction", new mitk::TransferFunctionProperty(tf.GetPointer()));
      } else {
        std::cerr << "QmitkTransferFunctionWidget::SetDataTreeNode called with non-image node" << std::endl;
      }
    }
    m_ScalarOpacityFunctionCanvas->setEnabled(true);
    m_ColorTransferFunctionCanvas->setEnabled(true);
    m_ScalarOpacityFunctionCanvas->update();
    m_ColorTransferFunctionCanvas->update();
  }
  else {
    // called with null-node
    //
    m_ScalarOpacityFunctionCanvas->setEnabled(false);
    m_ColorTransferFunctionCanvas->setEnabled(false);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkTransferFunctionWidget::UpdateMinMaxLabels()
{
  if (m_ScalarOpacityFunctionCanvas)
  {
    m_MinLabel->setText(QString::number(m_ScalarOpacityFunctionCanvas->GetMin()));
    m_MaxLabel->setText(QString::number(m_ScalarOpacityFunctionCanvas->GetMax()));
  }
}
void QmitkTransferFunctionWidget::PropertyChange( QListViewItem * PropertyItem )
{
  return;
}
