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
  m_SemanticRelations = std::make_shared<mitk::SemanticRelations>(GetDataStorage());
  m_SemanticRelationsManager = std::make_unique<mitk::SemanticRelationsManager>(GetDataStorage());

  connect(m_Controls.caseIDComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnCaseIDSelectionChanged(const QString&)));
  connect(m_Controls.selectPatientNodePushButton, SIGNAL(clicked()), SLOT(OnSelectPatientNodeButtonClicked()));

  m_Controls.lesionInfoWidget->SetDataStorage(GetDataStorage());
  m_SemanticRelationsManager->AddObserver(m_Controls.lesionInfoWidget);

  m_PatientTableWidget = new QmitkPatientTableWidget(GetDataStorage());
  m_SemanticRelationsManager->AddObserver(m_PatientTableWidget);

  m_SelectPatientNodeDialog = new QmitkSelectNodeDialog(parent);
  m_SelectPatientNodeDialog->setWindowTitle("Select patient image node");
  m_SelectPatientNodeDialog->SetSelectionWidget(m_PatientTableWidget);

  m_SimpleDatamanagerWidget = new QmitkSimpleDatamanagerWidget(GetDataStorage());
  m_SimpleDatamanagerNodeDialog = new QmitkSelectNodeDialog(parent);
  m_SimpleDatamanagerNodeDialog->setWindowTitle("Select node");
  m_SimpleDatamanagerNodeDialog->SetSelectionWidget(m_SimpleDatamanagerWidget);

  // connect buttons to modify semantic relations
  connect(m_Controls.addLesionPushButton, SIGNAL(clicked()), SLOT(OnAddLesionButtonClicked()));
  connect(m_Controls.removeLesionPushButton, SIGNAL(clicked()), SLOT(OnRemoveLesionButtonClicked()));
  connect(m_Controls.linkLesionPushButton, SIGNAL(clicked()), SLOT(OnLinkLesionButtonClicked()));
  connect(m_Controls.unlinkLesionPushButton, SIGNAL(clicked()), SLOT(OnUnlinkLesionButtonClicked()));
  connect(m_Controls.addImagePushButton, SIGNAL(clicked()), SLOT(OnAddImageButtonClicked()));
  connect(m_Controls.removeImagePushButton, SIGNAL(clicked()), SLOT(OnRemoveImageButtonClicked()));
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  // ToDo: automatically remove node from storage, so that there will be no access anymore
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_CaseID = caseID.toStdString();
  m_SemanticRelationsManager->SetCurrentCaseID(m_CaseID);
  m_Controls.lesionInfoWidget->SetCurrentCaseID(m_CaseID);
}

void QmitkSemanticRelationsView::OnSelectPatientNodeButtonClicked()
{
  int dialogReturnValue = m_SelectPatientNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SelectPatientNodeDialog->GetSelectedDataNode();
  if (nullptr == selectedDataNode)
  {
    m_Controls.selectedPatientNodeLineEdit->clear();
  }
  else
  {
    m_Controls.selectedPatientNodeLineEdit->setText(QString::fromStdString(selectedDataNode->GetName()));
  }
  //m_PatientInfoWidget->SetPatientInfo(node);
  //m_PatientInfoWidget->show();
}

void QmitkSemanticRelationsView::OnAddLesionButtonClicked()
{
  if (m_CaseID.empty())
  {
    MITK_INFO << "No case ID selected";
    return;
  }

  m_SemanticRelationsManager->GenerateNewLesion(m_CaseID);
}

void QmitkSemanticRelationsView::OnRemoveLesionButtonClicked()
{

}

void QmitkSemanticRelationsView::OnLinkLesionButtonClicked()
{
  // ToDo: link with selected lesion item from QmitkLesionInfoWidget
  int dialogReturnValue = m_SimpleDatamanagerNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SimpleDatamanagerNodeDialog->GetSelectedDataNode();
  if (nullptr != selectedDataNode)
  {
    if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(selectedDataNode))
    {
      // get parent node of the current segmentation node with the node predicate
      mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(selectedDataNode, mitk::NodePredicates::GetImagePredicate(), false);

      mitk::BaseData* baseData = selectedDataNode->GetData();
      if (nullptr == baseData)
      {
        MITK_INFO << "No valid base data: Cannot transfer DICOM tags to the segmentation node";
        return;
      }
      // transfer DICOM tags to the segmentation node
      mitk::SemanticTypes::CaseID caseID = mitk::DICOMHelper::GetCaseIDFromDataNode(parentNodes->front());
      mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(caseID);
      baseData->SetProperty("DICOM.0010.0010", caseIDTag); // DICOM tag is "PatientName"
                                                           // add UID to distinguish between different segmentations of the same parent node
      mitk::StringProperty::Pointer nodeIDTag = mitk::StringProperty::New(caseID + mitk::UIDGeneratorBoost::GenerateUID());
      baseData->SetProperty("DICOM.0020.000E", nodeIDTag); // DICOM tag is "SeriesInstanceUID"
                                                           // add the segmentation to the semantic relations storage
      m_SemanticRelationsManager->AddSegmentationInstance(selectedDataNode, parentNodes->front());

      AddToComboBox(caseID);
    }
    else
    {
      MITK_INFO << "No segmentation node selected";
      return;
    }
  }
}

void QmitkSemanticRelationsView::OnUnlinkLesionButtonClicked()
{
  /*
  int dialogReturnValue = m_SelectPatientNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedSegmentation = m_SelectPatientNodeDialog->GetSelectedDataNode();
  if (nullptr != selectedSegmentation)
  {
    if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(selectedSegmentation))
    {
      // remove the segmentation from the semantic relations storage
      m_SemanticRelationsManager->RemoveSegmentationInstance(node);

      mitk::SemanticTypes::CaseID caseID = mitk::DICOMHelper::GetCaseIDFromDataNode(node);
      RemoveFromComboBox(caseID);
    }
  }
  */
}

void QmitkSemanticRelationsView::OnAddImageButtonClicked()
{
  int dialogReturnValue = m_SimpleDatamanagerNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SimpleDatamanagerNodeDialog->GetSelectedDataNode();
  if (nullptr != selectedDataNode)
  {
    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(selectedDataNode))
    {
      // add the image to the semantic relations storage
      m_SemanticRelationsManager->AddImageInstance(selectedDataNode);
      m_PatientTableWidget->SetPixmapOfNode(selectedDataNode);
      mitk::SemanticTypes::CaseID caseID = mitk::DICOMHelper::GetCaseIDFromDataNode(selectedDataNode);
      AddToComboBox(caseID);
    }
    else
    {
      MITK_INFO << "No image node selected";
      return;
    }
  }
}

void QmitkSemanticRelationsView::OnRemoveImageButtonClicked()
{
  // TODO: remove from lesion list widget?
  int dialogReturnValue = m_SimpleDatamanagerNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SimpleDatamanagerNodeDialog->GetSelectedDataNode();
  if (nullptr != selectedDataNode)
  {
    if (mitk::NodePredicates::GetImagePredicate()->CheckNode(selectedDataNode))
    {
      // remove the image from the semantic relations storage
      m_PatientTableWidget->DeletePixmapOfNode(selectedDataNode);
      m_SemanticRelationsManager->RemoveImageInstance(selectedDataNode);
    }
    else
    {
      MITK_INFO << "No image node selected";
      return;
    }
  }
}

void QmitkSemanticRelationsView::AddToComboBox(const mitk::SemanticTypes::CaseID& caseID)
{
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (-1 == foundIndex)
  {
    // add the caseID to the combo box, as it is not already contained
    m_Controls.caseIDComboBox->addItem(QString::fromStdString(caseID));
  }
}

void QmitkSemanticRelationsView::RemoveFromComboBox(const mitk::SemanticTypes::CaseID& caseID)
{
  std::vector<mitk::SemanticTypes::ControlPoint> allControlPoints = m_SemanticRelations->GetAllControlPointsOfCase(caseID);
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (allControlPoints.empty() && -1 != foundIndex)
  {
    // TODO: find new way to check for empty case id
    // caseID does not contain any control points and therefore no data
    // remove the caseID, if it is still contained
    m_Controls.caseIDComboBox->removeItem(foundIndex);
  }
}