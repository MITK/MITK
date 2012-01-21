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

#include "QmitkBooleanOperationsView.h"
#include <mitkProperties.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

QmitkBooleanOperationsView::QmitkBooleanOperationsView()
  : m_Parent(NULL)
{
}

QmitkBooleanOperationsView::~QmitkBooleanOperationsView()
{
}

void QmitkBooleanOperationsView::CreateQtPartControl(QWidget *parent)
{
  mitk::DataStorage::Pointer dataStorage = GetDefaultDataStorage();
  
  mitk::NodePredicateAnd::Pointer segmentationPredicate = mitk::NodePredicateAnd::New(
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)),
    mitk::NodePredicateNot::New(
      mitk::NodePredicateProperty::New("helper object")));
  
  m_Parent = parent;

  m_Controls.setupUi(parent);

  m_Controls.cmbSegmentationImage1->SetDataStorage(dataStorage);
  m_Controls.cmbSegmentationImage1->SetPredicate(segmentationPredicate);
  
  m_Controls.cmbSegmentationImage2->SetPredicate(segmentationPredicate);
  m_Controls.cmbSegmentationImage2->SetDataStorage(dataStorage);

  connect(m_Controls.cmbSegmentationImage1, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSegmentationImage1Changed(const mitk::DataNode *)));
  connect(m_Controls.cmbSegmentationImage2, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSegmentationImage2Changed(const mitk::DataNode *)));
  connect(m_Controls.btnDifference, SIGNAL(clicked()), this, SLOT(OnDifferenceButtonClicked()));
  connect(m_Controls.btnUnion, SIGNAL(clicked()), this, SLOT(OnUnionButtonClicked()));
  connect(m_Controls.btnIntersection, SIGNAL(clicked()), this, SLOT(OnIntersectionButtonClicked()));
}

void QmitkBooleanOperationsView::EnableButtons(bool enable)
{
  m_Controls.lblInputImagesWarning->setVisible(!enable);
  m_Controls.btnDifference->setEnabled(enable);
  m_Controls.btnUnion->setEnabled(enable);
  m_Controls.btnIntersection->setEnabled(enable);
}

void QmitkBooleanOperationsView::DisableButtons(bool disable)
{
  EnableButtons(false);
}

void QmitkBooleanOperationsView::OnSegmentationImage1Changed(const mitk::DataNode *dataNode)
{
  if (dataNode != m_Controls.cmbSegmentationImage2->GetSelectedNode())
    EnableButtons();
  else
    DisableButtons();
}

void QmitkBooleanOperationsView::OnSegmentationImage2Changed(const mitk::DataNode *dataNode)
{
  if (dataNode != m_Controls.cmbSegmentationImage1->GetSelectedNode())
    EnableButtons();
  else
    DisableButtons();
}

void QmitkBooleanOperationsView::OnDifferenceButtonClicked()
{
}

void QmitkBooleanOperationsView::OnUnionButtonClicked()
{
}

void QmitkBooleanOperationsView::OnIntersectionButtonClicked()
{
}
