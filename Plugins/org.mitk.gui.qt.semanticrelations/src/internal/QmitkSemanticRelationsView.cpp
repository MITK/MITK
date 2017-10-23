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
#include <mitkSemanticRelationException.h>
#include <mitkUIDGeneratorBoost.h>

// blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// qt
#include <QMessageBox>

const std::string QmitkSemanticRelationsView::VIEW_ID = "org.mitk.views.semanticrelations";

QmitkSemanticRelationsView::QmitkSemanticRelationsView()
  : m_PatientTableWidget(nullptr)
  , m_SimpleDatamanagerWidget(nullptr)
  , m_SelectPatientNodeDialog(nullptr)
  , m_SimpleDatamanagerNodeDialog(nullptr)
  , m_SemanticRelations(nullptr)
{
  // nothing here
}

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

  connect(m_Controls.caseIDComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(OnCaseIDSelectionChanged(const QString&)));
  connect(m_Controls.selectPatientNodePushButton, SIGNAL(clicked()), SLOT(OnSelectPatientNodeButtonClicked()));

  m_LesionInfoWidget = new QmitkLesionInfoWidget(GetDataStorage());
  m_Controls.gridLayout->addWidget(m_LesionInfoWidget, 3, 0, 1, 3);
  connect(m_LesionInfoWidget, SIGNAL(JumpToPosition(const mitk::Point3D&)), SLOT(OnJumpToPosition(const mitk::Point3D&)));
  connect(m_LesionInfoWidget, SIGNAL(ImageRemoved(const mitk::DataNode*)), SLOT(OnImageRemoved(const mitk::DataNode*)));

  m_PatientTableWidget = new QmitkPatientTableWidget(GetDataStorage());

  m_SelectPatientNodeDialog = new QmitkSelectNodeDialog(parent);
  m_SelectPatientNodeDialog->setWindowTitle("Select patient image node");
  m_SelectPatientNodeDialog->SetSelectionWidget(m_PatientTableWidget);

  m_SimpleDatamanagerWidget = new QmitkSimpleDatamanagerWidget(GetDataStorage());
  m_SimpleDatamanagerNodeDialog = new QmitkSelectNodeDialog(parent);
  m_SimpleDatamanagerNodeDialog->setWindowTitle("Select node");
  m_SimpleDatamanagerNodeDialog->SetSelectionWidget(m_SimpleDatamanagerWidget);

  // connect buttons to modify semantic relations
  connect(m_Controls.addLesionPushButton, SIGNAL(clicked()), SLOT(OnAddLesionButtonClicked()));
  connect(m_Controls.addSegmentationPushButton, SIGNAL(clicked()), SLOT(OnAddSegmentationButtonClicked()));
  connect(m_Controls.addImagePushButton, SIGNAL(clicked()), SLOT(OnAddImageButtonClicked()));
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  // ToDo: automatically remove node from storage, so that there will be no access anymore
}

void QmitkSemanticRelationsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_CaseID = caseID.toStdString();
  m_LesionInfoWidget->SetCurrentCaseID(m_CaseID);
  m_PatientTableWidget->SetCurrentCaseID(m_CaseID);
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

void QmitkSemanticRelationsView::OnJumpToPosition(const mitk::Point3D& position)
{
  mitk::IRenderWindowPart* renderWindowPart = GetRenderWindowPart();
  if (nullptr != renderWindowPart)
  {
    renderWindowPart->SetSelectedPosition(position);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkSemanticRelationsView::OnAddLesionButtonClicked()
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to add a lesion, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  mitk::SemanticTypes::Lesion newLesion;
  newLesion.UID = mitk::UIDGeneratorBoost::GenerateUID();
  newLesion.lesionClass = mitk::SemanticTypes::LesionClass();
  newLesion.lesionClass.UID = mitk::UIDGeneratorBoost::GenerateUID();

  try
  {
    m_SemanticRelations->AddLesion(m_CaseID, newLesion);
  }
  catch (mitk::SemanticRelationException& e)
  {
    MITK_INFO << "Could not add a new lesion. " << e;
  }
}

void QmitkSemanticRelationsView::OnAddSegmentationButtonClicked()
{
  if (m_CaseID.empty())
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No case ID set.");
    msgBox.setText("In order to add a segmentation, please specify the current case / patient.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  int dialogReturnValue = m_SimpleDatamanagerNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SimpleDatamanagerNodeDialog->GetSelectedDataNode();
  if (nullptr == selectedDataNode)
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No valid segmentation node selected.");
    msgBox.setText("In order to add a segmentation, please specify a valid segmentation node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  if (!mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(selectedDataNode))
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No segmentation selected");
    msgBox.setText("In order to add a segmentation, please specify a valid segmentation node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  mitk::BaseData* baseData = selectedDataNode->GetData();
  if (nullptr == baseData)
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No valid base data.");
    msgBox.setText("In order to link the selected lesion to a segmentation, please specify a valid segmentation node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  // continue with valid segmentation data
  // get parent node of the current segmentation node with the node predicate
  mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(selectedDataNode, mitk::NodePredicates::GetImagePredicate(), false);

  // check for already existing, identifying base properties
  mitk::BaseProperty* caseIDProperty = baseData->GetProperty("DICOM.0010.0010");
  mitk::BaseProperty* nodeIDProperty = baseData->GetProperty("DICOM.0020.000E");
  if (nullptr == caseIDProperty && nullptr == nodeIDProperty)
  {
    MITK_INFO << "No DICOM tags for case and node identification found. Transferring DICOM tags from the parent node to the selected segmentation node.";

    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(parentNodes->front());
    mitk::SemanticTypes::ID nodeID = mitk::GetIDFromDataNode(parentNodes->front());
    // transfer DICOM tags to the segmentation node
    mitk::StringProperty::Pointer caseIDTag = mitk::StringProperty::New(caseID);
    baseData->SetProperty("DICOM.0010.0010", caseIDTag); // DICOM tag is "PatientName"

    // add UID to distinguish between different segmentations of the same parent node
    mitk::StringProperty::Pointer nodeIDTag = mitk::StringProperty::New(nodeID + mitk::UIDGeneratorBoost::GenerateUID());
    baseData->SetProperty("DICOM.0020.000E", nodeIDTag); // DICOM tag is "SeriesInstanceUID"
  }

  try
  {
    m_SemanticRelations->AddSegmentation(selectedDataNode, parentNodes->front());
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not add the selected segmentation.");
    msgBox.setText("The program wasn't able to correctly add the selected segmentation.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkSemanticRelationsView::OnAddImageButtonClicked()
{
  int dialogReturnValue = m_SimpleDatamanagerNodeDialog->exec();
  if (QDialog::Rejected == dialogReturnValue)
  {
    return;
  }

  mitk::DataNode* selectedDataNode = m_SimpleDatamanagerNodeDialog->GetSelectedDataNode();
  if (nullptr == selectedDataNode)
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No valid image node selected.");
    msgBox.setText("In order to add an image, please specify a valid image node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  if (!mitk::NodePredicates::GetImagePredicate()->CheckNode(selectedDataNode))
  {
    QMessageBox msgBox;
    msgBox.setWindowTitle("No image selected");
    msgBox.setText("In order to add an image, please specify a valid image node.");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
    return;
  }

  try
  {
    // add the image to the semantic relations storage
    m_SemanticRelations->AddImage(selectedDataNode);
    m_PatientTableWidget->SetPixmapOfNode(selectedDataNode);
    mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(selectedDataNode);
    AddToComboBox(caseID);
  }
  catch (mitk::Exception& e)
  {
    std::stringstream exceptionMessage; exceptionMessage << e;
    QMessageBox msgBox;
    msgBox.setWindowTitle("Could not add the selected image.");
    msgBox.setText("The program wasn't able to correctly add the selected image.\n"
      "Reason:\n" + QString::fromStdString(exceptionMessage.str()));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();
  }
}

void QmitkSemanticRelationsView::OnImageRemoved(const mitk::DataNode* imageNode)
{
  m_PatientTableWidget->DeletePixmapOfNode(imageNode);
  mitk::SemanticTypes::CaseID caseID = mitk::GetCaseIDFromDataNode(imageNode);
  RemoveFromComboBox(caseID);
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