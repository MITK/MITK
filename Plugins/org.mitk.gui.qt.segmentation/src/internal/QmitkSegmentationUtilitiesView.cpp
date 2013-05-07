/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Mitk
#include <mitkSurfaceToImageFilter.h>

// Qmitk
#include "QmitkSegmentationUtilitiesView.h"

// Qt
#include <QMessageBox>


const std::string QmitkSegmentationUtilitiesView::VIEW_ID = "org.mitk.views.segmentationutilities";

QmitkSegmentationUtilitiesView::QmitkSegmentationUtilitiesView()
{
  m_IsOfTypeImagePredicate = mitk::NodePredicateAnd::New(mitk::TNodePredicateDataType<mitk::Image>::New(),
                                                         mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_IsOfTypeSurface = mitk::NodePredicateAnd::New(mitk::TNodePredicateDataType<mitk::Surface>::New(),
                                                  mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  m_IsBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  m_IsNotBinaryPredicate = mitk::NodePredicateNot::New( m_IsBinaryPredicate );

  m_IsNotABinaryImagePredicate = mitk::NodePredicateAnd::New( m_IsOfTypeImagePredicate, m_IsNotBinaryPredicate );
  m_IsABinaryImagePredicate = mitk::NodePredicateAnd::New( m_IsOfTypeImagePredicate, m_IsBinaryPredicate);
}

void QmitkSegmentationUtilitiesView::SetFocus()
{
//  m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkSegmentationUtilitiesView::CreateQtPartControl( QWidget *parent )
{
  m_Controls.setupUi( parent );

  mitk::DataStorage::Pointer dataStorage = this->GetDataStorage();

  m_Controls.cmbBooleanOperationsSegImage1->SetPredicate(m_IsABinaryImagePredicate);
  m_Controls.cmbBooleanOperationsSegImage1->SetDataStorage(dataStorage);
  m_Controls.cmbBooleanOperationsSegImage2->SetPredicate(m_IsABinaryImagePredicate);
  m_Controls.cmbBooleanOperationsSegImage2->SetDataStorage(dataStorage);

  m_Controls.cmbMorphologicalOperationsSegImage->SetPredicate(m_IsABinaryImagePredicate);
  m_Controls.cmbMorphologicalOperationsSegImage->SetDataStorage(dataStorage);

  m_Controls.cmbMaskingReferenceData->SetPredicate(m_IsOfTypeImagePredicate);
  m_Controls.cmbMaskingReferenceData->SetDataStorage(dataStorage);
  m_Controls.cmbMaskingData->SetPredicate(m_IsABinaryImagePredicate);
  m_Controls.cmbMaskingData->SetDataStorage(dataStorage);

  //Create connections for boolean operations
  connect (m_Controls.cmbBooleanOperationsSegImage1, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
           this, SLOT(OnBooleanOperationsSegImage1SelectionChanged(const mitk::DataNode*)));
  connect (m_Controls.cmbBooleanOperationsSegImage2, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
           this, SLOT(OnBooleanOperationsSegImage2SelectionChanged(const mitk::DataNode*)));

  //Create connections for morphological operations
  connect (m_Controls.cmbMorphologicalOperationsSegImage, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
           this, SLOT(OnMorphologicalOperationsDataSelectionChanged(const mitk::DataNode*)));

  //Create connections for image masking
  connect (m_Controls.rbMaskImage, SIGNAL(toggled(bool)), this, SLOT(OnImageMaskingToggled(bool)));
  connect (m_Controls.rbMaskSurface, SIGNAL(toggled(bool)), this, SLOT(OnSurfaceMaskingToggled(bool)));
  connect (m_Controls.btnMaskImage, SIGNAL(pressed()), this, SLOT(OnMaskedImagePressed()));
  connect (m_Controls.cmbMaskingData, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
           this, SLOT(OnMaskingDataSelectionChanged(const mitk::DataNode*)));
  connect (m_Controls.cmbMaskingReferenceData, SIGNAL(OnSelectionChanged(const mitk::DataNode*)),
           this, SLOT(OnMaskingReferenceDataSelectionChanged(const mitk::DataNode*)));
}

void QmitkSegmentationUtilitiesView::OnSelectionChanged( berry::IWorkbenchPart::Pointer /*source*/,
                                             const QList<mitk::DataNode::Pointer>& nodes )
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes )
  {
    if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
    {
      return;
    }
  }
}

/*
 * Functionality for boolean operations
 */
void QmitkSegmentationUtilitiesView::OnBooleanOperationsSegImage1SelectionChanged(const mitk::DataNode *node)
{
  if (node != m_Controls.cmbBooleanOperationsSegImage2->GetSelectedNode())
    EnableBooleanButtons(true);
  else
    EnableBooleanButtons(false);
}

void QmitkSegmentationUtilitiesView::OnBooleanOperationsSegImage2SelectionChanged(const mitk::DataNode *node)
{
  if (node != m_Controls.cmbBooleanOperationsSegImage1->GetSelectedNode())
    EnableBooleanButtons(true);
  else
    EnableBooleanButtons(false);
}

void QmitkSegmentationUtilitiesView::EnableBooleanButtons(bool status)
{
  m_Controls.btnDifference->setEnabled(status);
  m_Controls.btnUnion->setEnabled(status);
  m_Controls.btnIntersection->setEnabled(status);
  m_Controls.lblBooleanOperationWarning->setVisible(!status);
}

/*
 * Functionality for morphological operations
 */
void QmitkSegmentationUtilitiesView::OnMorphologicalOperationsDataSelectionChanged(const mitk::DataNode *node)
{
  bool enableButtons(node);
  m_Controls.btnClosing->setEnabled(enableButtons);
  m_Controls.btnDilatation->setEnabled(enableButtons);
  m_Controls.btnErosion->setEnabled(enableButtons);
  m_Controls.btnFillHoles->setEnabled(enableButtons);
  m_Controls.btnOpening->setEnabled(enableButtons);
  m_Controls.lblMorphologicOperationsWarning->setVisible(!enableButtons);
}

/*
 * Functionality for image masking
 */
void QmitkSegmentationUtilitiesView::OnImageMaskingToggled(bool status)
{
  if (status)
  {
    m_Controls.cmbMaskingData->SetPredicate(m_IsABinaryImagePredicate);
    m_Controls.lblMaskingWarnings->setText("Select a regular image and a binary image above");
  }
}

void QmitkSegmentationUtilitiesView::OnSurfaceMaskingToggled(bool status)
{
  if (status)
  {
    m_Controls.cmbMaskingData->SetPredicate((m_IsOfTypeSurface));
    m_Controls.lblMaskingWarnings->setText("Select a regular image and a surface above");
  }
}

void QmitkSegmentationUtilitiesView::OnMaskingDataSelectionChanged(const mitk::DataNode *node)
{
  m_Controls.btnMaskImage->setEnabled(m_Controls.cmbMaskingReferenceData->GetSelectedNode().IsNotNull() && node);
  m_Controls.lblMaskingWarnings->setVisible(!(m_Controls.cmbMaskingReferenceData->GetSelectedNode().IsNotNull() && node));
}

void QmitkSegmentationUtilitiesView::OnMaskingReferenceDataSelectionChanged(const mitk::DataNode *node)
{
  m_Controls.btnMaskImage->setEnabled(m_Controls.cmbMaskingData->GetSelectedNode().IsNotNull() && node);
}

void QmitkSegmentationUtilitiesView::OnMaskImagePressed()
{
  if (m_Controls.rbMaskImage->isChecked())
  {
    //Do actual image masking
  }
  else
  {
    //Do actual surface masking
  }
}
