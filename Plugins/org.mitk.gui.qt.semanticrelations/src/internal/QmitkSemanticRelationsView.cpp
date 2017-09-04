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

// semantic relations plugin
#include "QmitkSemanticRelationsView.h"

// semantic relations module
#include <mitkSemanticRelationsManager.h>
#include <mitkNodePredicates.h>
#include <mitkUIDGeneratorBoost.h>

// blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

const std::string QmitkSemanticRelationsView::VIEW_ID = "org.mitk.views.semanticrelations";

void QmitkSemanticRelationsView::SetFocus()
{
  // nothing here
}

void QmitkSemanticRelationsView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  // initialize the semantic relations
  m_SemanticRelations = std::make_unique<mitk::SemanticRelations>(GetDataStorage());

  //m_PatientTableWidget = new QmitkPatientTableWidget(m_SemanticRelations, parent);
  //m_PatientTableWidget->setObjectName(QStringLiteral("m_SemanticRelationsTableView"));
  //m_Controls.verticalLayout->addWidget(m_PatientTableWidget);

  m_LesionInfoWidget = new QmitkLesionInfoWidget(m_SemanticRelations, parent);
  m_Controls.verticalLayout->addWidget(m_LesionInfoWidget);

  m_PatientInfoWidget = new QmitkPatientInfoWidget(parent);
  //m_Controls.verticalLayout->addWidget(m_PatientInfoWidget);
  //m_PatientInfoWidget->hide();

  connect(m_Controls.caseIDComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnCaseIDSelectionChanged(const QString&)));
  //connect(m_PatientTableWidget, SIGNAL(SelectionChanged(const mitk::DataNode*)), SLOT(OnPatientTableSelectionChanged(const mitk::DataNode*)));
}

void QmitkSemanticRelationsView::NodeAdded(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  { 
    return;
  }

  // not a helper object
  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(node))
  {
    // add the image to the semantic relations storage
    mitk::SemanticRelationsManager::AddImageInstance(node, *m_SemanticRelations);
    mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(mitk::DICOMHelper::GetCaseIDFromDataNode(node));
    m_PatientTableWidget->SetPixmapOfNode(node);
    if (m_Controls.caseIDComboBox->currentText().toStdString() == caseIDTag->GetValueAsString())
    {
      // updating the patient table of the case that is currently active / visible
      m_PatientTableWidget->UpdatePatientTable();
    }
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(node))
  {
    // get parent node of the current segmentation node with the node predicate
    mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(node, mitk::NodePredicates::GetImagePredicate(), false);

    mitk::BaseData* baseData = node->GetData();
    if (nullptr == baseData)
    {
      MITK_INFO << "No valid base data: Cannot transfer DICOM tags to the segmentation node";
      return;
    }
    // transfer DICOM tags to the segmentation node
    mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(mitk::DICOMHelper::GetCaseIDFromDataNode(parentNodes->front()));
    baseData->SetProperty("DICOM.0010.0010", caseIDTag); // DICOM tag is "PatientName"
    // add UID to distinguish between different segmentations of the same parent node
    mitk::StringProperty::Pointer nodeIDTag = mitk::StringProperty::New(mitk::DICOMHelper::GetIDFromDataNode(parentNodes->front()) + mitk::UIDGeneratorBoost::GenerateUID());
    baseData->SetProperty("DICOM.0020.000E", nodeIDTag); // DICOM tag is "SeriesInstanceUID"

    // add the segmentation to the semantic relations storage
    mitk::SemanticRelationsManager::AddSegmentationInstance(node, parentNodes->front(), *m_SemanticRelations);
    if (m_Controls.caseIDComboBox->currentText().toStdString() == caseIDTag->GetValueAsString())
    {
      // updating the lesion list of the case that is currently active / visible
      m_LesionInfoWidget->UpdateLesionInfoWidget();
    }
  }

  const mitk::SemanticTypes::CaseID caseID = mitk::DICOMHelper::GetCaseIDFromDataNode(node);
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (-1 == foundIndex)
  {
    // add the caseID to the combo box, if it is not already contained
    m_Controls.caseIDComboBox->addItem(QString::fromStdString(caseID));
  }
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  {
    return;
  }

  // not a helper object
  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(node))
  {
    // remove the image from the semantic relations storage
    mitk::SemanticRelationsManager::RemoveImageInstance(node, *m_SemanticRelations);
    mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(mitk::DICOMHelper::GetCaseIDFromDataNode(node));
    m_PatientTableWidget->DeletePixmapOfNode(node);
    if (m_Controls.caseIDComboBox->currentText().toStdString() == caseIDTag->GetValueAsString())
    {
      // updating the patient table of the case that is currently active / visible
      m_PatientTableWidget->UpdatePatientTable();
    }
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(node))
  {
    // get parent node of the current segmentation node with the node predicate
    mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(node, mitk::NodePredicates::GetImagePredicate(), false);

    mitk::BaseData* baseData = node->GetData();
    if (nullptr == baseData)
    {
      MITK_INFO << "No valid base data: Cannot transfer DICOM tags to the segmentation node";
      return;
    }
    // remove the segmentation from the semantic relations storage
    mitk::SemanticRelationsManager::RemoveSegmentationInstance(node, *m_SemanticRelations);
    mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(mitk::DICOMHelper::GetCaseIDFromDataNode(node));
    if (m_Controls.caseIDComboBox->currentText().toStdString() == caseIDTag->GetValueAsString())
    {
      // updating the lesion list of the case that is currently active / visible
      m_LesionInfoWidget->UpdateLesionInfoWidget();
    }
  }

  const mitk::SemanticTypes::CaseID caseID = mitk::DICOMHelper::GetCaseIDFromDataNode(node);
  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(caseID);
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (allControlPoints.empty() && -1 != foundIndex)
  {
    // caseID does not contain any remove the caseID, if it is still contained
    m_Controls.caseIDComboBox->removeItem(foundIndex);
  }
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_PatientTableWidget->SetCurrentCaseID(caseID.toStdString());
  m_LesionInfoWidget->SetCurrentCaseID(caseID.toStdString());
}

void QmitkSemanticRelationsView::OnPatientTableSelectionChanged(const mitk::DataNode* node)
{
  m_PatientInfoWidget->SetPatientInfo(node);
  m_PatientInfoWidget->show();
}
