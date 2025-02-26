/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelManager.h>

// mitk
#include <mitkAutoCropImageFilter.h>
#include <mitkCoreObjectFactory.h>
#include <mitkIOUtil.h>
#include <mitkLabelSetImage.h>
#include <mitkLabelSetImageToSurfaceThreadedFilter.h>
#include <mitkLabelSetImageConverter.h>
#include <mitkRenderingManager.h>
#include <mitkStatusBar.h>
#include <mitkToolManagerProvider.h>

// Qmitk
#include <QmitkStyleManager.h>
#include <QmitkMultiLabelPresetHelper.h>

// Qt
#include <QLabel>
#include <QWidgetAction>
#include <QColorDialog>
#include <QCompleter>
#include <QDateTime>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>
#include <QStringListModel>

// itk
#include <itksys/SystemTools.hxx>

#include <ui_QmitkMultiLabelManagerControls.h>


QmitkMultiLabelManager::QmitkMultiLabelManager(QWidget *parent)
  : QWidget(parent), m_Controls(new Ui::QmitkMultiLabelManagerControls), m_Completer(nullptr), m_ProcessingManualSelection(false), m_DataStorage(nullptr)
{
  m_Controls->setupUi(this);

  m_Controls->labelSearchBox->setAlwaysShowClearIcon(true);
  m_Controls->labelSearchBox->setShowSearchIcon(true);

  QStringList completionList;
  completionList << "";
  m_Completer = new QCompleter(completionList, this);
  m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_Controls->labelSearchBox->setCompleter(m_Completer);

  m_Controls->labelInspector->SetAllowLabelModification(true);

  connect(m_Controls->labelSearchBox, SIGNAL(returnPressed()), this, SLOT(OnSearchLabel()));

  QStringListModel *completeModel = static_cast<QStringListModel *>(m_Completer->model());
  completeModel->setStringList(GetLabelStringList());

  // See T29549
  m_Controls->labelSearchBox->hide();

  m_Controls->btnSavePreset->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-save.svg")));
  m_Controls->btnLoadPreset->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/document-open.svg")));
  m_Controls->btnAddLabel->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_add.svg")));
  m_Controls->btnAddInstance->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_add_instance.svg")));
  m_Controls->btnAddGroup->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_group_add.svg")));
  m_Controls->btnRemoveLabel->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_delete.svg")));
  m_Controls->btnRemoveInstance->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_label_delete_instance.svg")));
  m_Controls->btnRemoveGroup->setIcon(QmitkStyleManager::ThemeIcon(QStringLiteral(":/Qmitk/icon_group_delete.svg")));

  connect(m_Controls->btnAddLabel, &QToolButton::clicked, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::AddNewLabel);
  connect(m_Controls->btnRemoveLabel, &QToolButton::clicked, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::DeleteLabel);
  connect(m_Controls->btnAddInstance, &QToolButton::clicked, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::AddNewLabelInstance);
  connect(m_Controls->btnRemoveInstance, &QToolButton::clicked, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::DeleteLabelInstance);
  connect(m_Controls->btnAddGroup, &QToolButton::clicked, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::AddNewGroup);
  connect(m_Controls->btnRemoveGroup, &QToolButton::clicked, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::RemoveGroup);
  connect(m_Controls->btnSavePreset, &QToolButton::clicked, this, &QmitkMultiLabelManager::OnSavePreset);
  connect(m_Controls->btnLoadPreset, &QToolButton::clicked, this, &QmitkMultiLabelManager::OnLoadPreset);

  connect(this->m_Controls->labelInspector, &QmitkMultiLabelInspector::GoToLabel, this, &QmitkMultiLabelManager::OnGoToLabel);
  connect(this->m_Controls->labelInspector, &QmitkMultiLabelInspector::LabelRenameRequested, this, &QmitkMultiLabelManager::OnLabelRenameRequested);
  connect(this->m_Controls->labelInspector, &QmitkMultiLabelInspector::CurrentSelectionChanged, this, &QmitkMultiLabelManager::OnSelectedLabelChanged);
  connect(this->m_Controls->labelInspector, &QmitkMultiLabelInspector::ModelUpdated, this, &QmitkMultiLabelManager::OnModelUpdated);
  connect(this->m_Controls->labelInspector, &QmitkMultiLabelInspector::SegmentationChanged, this, &QmitkMultiLabelManager::OnSegmentationChanged);

  auto* renameLabelShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_R), this);
  connect(renameLabelShortcut, &QShortcut::activated, this, &QmitkMultiLabelManager::OnRenameLabelShortcutActivated);

  auto* addLabelShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_A), this);
  connect(addLabelShortcut, &QShortcut::activated, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::AddNewLabel);

  auto* addLabelInstanceShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_I), this);
  connect(addLabelInstanceShortcut, &QShortcut::activated, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::AddNewLabelInstance);

  auto* deleteLabelShortcut = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key::Key_L, Qt::CTRL | Qt::Key::Key_D), this);
  connect(deleteLabelShortcut, &QShortcut::activated, this->m_Controls->labelInspector, &QmitkMultiLabelInspector::DeleteLabelInstance);

  this->UpdateControls();
}

QmitkMultiLabelManager::~QmitkMultiLabelManager()
{
  this->SetMultiLabelSegmentation(nullptr);
  delete m_Controls;
}

QmitkMultiLabelManager::LabelValueVectorType QmitkMultiLabelManager::GetSelectedLabels() const
{
  return m_Controls->labelInspector->GetSelectedLabels();
}


void QmitkMultiLabelManager::OnRenameLabelShortcutActivated()
{
  auto selectedLabels = this->GetSelectedLabels();

  for (auto labelValue : selectedLabels)
  {
    auto currentLabel = this->GetMultiLabelSegmentation()->GetLabel(labelValue);
    if (currentLabel.IsNull())
      continue;
    bool canceled = false;
    emit LabelRenameRequested(currentLabel, true, canceled);
  }
}

void QmitkMultiLabelManager::OnSelectedLabelChanged(const LabelValueVectorType& labels)
{
  this->UpdateControls();
  if (labels.empty() || labels.size() > 1) return;

  emit CurrentSelectionChanged(labels);
}

QStringList &QmitkMultiLabelManager::GetLabelStringList()
{
  return m_LabelStringList;
}

void QmitkMultiLabelManager::SetDefaultLabelNaming(bool defaultLabelNaming)
{
  this->m_Controls->labelInspector->SetDefaultLabelNaming(defaultLabelNaming);
}

void QmitkMultiLabelManager::setEnabled(bool enabled)
{
  QWidget::setEnabled(enabled);
  UpdateControls();
}

void QmitkMultiLabelManager::SetSelectedLabels(const LabelValueVectorType& selectedLabels)
{
  this->m_Controls->labelInspector->SetSelectedLabels(selectedLabels);
  UpdateControls();
}

void QmitkMultiLabelManager::SetSelectedLabel(mitk::MultiLabelSegmentation::LabelValueType selectedLabel)
{
  this->m_Controls->labelInspector->SetSelectedLabel(selectedLabel);
  UpdateControls();
}

void QmitkMultiLabelManager::SetMultiLabelSegmentation(mitk::MultiLabelSegmentation* segmentation)
{
  this->m_Controls->labelInspector->SetMultiLabelSegmentation(segmentation);

  //Update of controls and observers is done in OnSegmentationChanged()
  // which is triggered by the inspector when setting the segmentation or node 
}

mitk::MultiLabelSegmentation* QmitkMultiLabelManager::GetMultiLabelSegmentation() const
{
  return this->m_Controls->labelInspector->GetMultiLabelSegmentation();
}

void QmitkMultiLabelManager::SetMultiLabelNode(mitk::DataNode* node)
{
  this->m_Controls->labelInspector->SetMultiLabelNode(node);
}

mitk::DataNode* QmitkMultiLabelManager::GetMultiLabelNode() const
{
  return this->m_Controls->labelInspector->GetMultiLabelNode();
}

void QmitkMultiLabelManager::SetDataStorage(mitk::DataStorage *storage)
{
  m_DataStorage = storage;
}

void QmitkMultiLabelManager::OnSearchLabel()
{
  //std::string text = m_Controls->labelSearchBox->text().toStdString();
  //int pixelValue = -1;
  //int row = -1;
  //for (int i = 0; i < m_Controls->m_LabelSetTableWidget->rowCount(); ++i)
  //{
  //  if (m_Controls->m_LabelSetTableWidget->item(i, 0)->text().toStdString().compare(text) == 0)
  //  {
  //    pixelValue = m_Controls->m_LabelSetTableWidget->item(i, 0)->data(Qt::UserRole).toInt();
  //    row = i;
  //    break;
  //  }
  //}
  //if (pixelValue == -1)
  //{
  //  return;
  //}

  //GetWorkingImage()->GetActiveLabelSet()->SetActiveLabel(pixelValue);

  //QTableWidgetItem *nameItem = m_Controls->m_LabelSetTableWidget->item(row, NAME_COL);
  //if (!nameItem)
  //{
  //  return;
  //}

  //m_Controls->m_LabelSetTableWidget->clearSelection();
  //m_Controls->m_LabelSetTableWidget->selectRow(row);
  //m_Controls->m_LabelSetTableWidget->scrollToItem(nameItem);

  //GetWorkingImage()->GetActiveLabelSet()->SetActiveLabel(pixelValue);

  //this->WaitCursorOn();
  //mitk::Point3D pos =
  //  GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetCenterOfMassCoordinates();

  //m_ToolManager->WorkingDataChanged();

  //if (pos.GetVnlVector().max_value() > 0.0)
  //{
  //  emit goToLabel(pos);
  //}
  //else
  //{
  //  GetWorkingImage()->UpdateCenterOfMass(pixelValue, GetWorkingImage()->GetActiveLayer());
  //  mitk::Point3D pos =
  //    GetWorkingImage()->GetLabel(pixelValue, GetWorkingImage()->GetActiveLayer())->GetCenterOfMassCoordinates();
  //  emit goToLabel(pos);
  //}

  //this->WaitCursorOff();
}

void QmitkMultiLabelManager::UpdateControls()
{
  bool hasWorkingData = this->GetMultiLabelSegmentation() != nullptr;

  auto labels = this->m_Controls->labelInspector->GetSelectedLabels();
  bool hasMultipleInstances = this->m_Controls->labelInspector->GetLabelInstancesOfSelectedFirstLabel().size() > 1;
  m_Controls->labelSearchBox->setEnabled(hasWorkingData);
  m_Controls->btnAddGroup->setEnabled(hasWorkingData);
  m_Controls->btnAddInstance->setEnabled(hasWorkingData && labels.size()==1);
  m_Controls->btnAddLabel->setEnabled(hasWorkingData);
  m_Controls->btnLoadPreset->setEnabled(hasWorkingData);
  m_Controls->btnRemoveGroup->setEnabled(hasWorkingData && !labels.empty() && this->GetMultiLabelSegmentation()->GetNumberOfLayers()>1);
  m_Controls->btnRemoveLabel->setEnabled(hasWorkingData && !labels.empty());
  m_Controls->btnRemoveInstance->setEnabled(hasWorkingData && !labels.empty() && hasMultipleInstances);
  m_Controls->btnSavePreset->setEnabled(hasWorkingData);

  if (!hasWorkingData)
    return;

  QStringListModel *completeModel = dynamic_cast<QStringListModel *>(m_Completer->model());
  completeModel->setStringList(GetLabelStringList());
}

void QmitkMultiLabelManager::OnCreateCroppedMask(bool)
{
  mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

  mitk::Image::Pointer maskImage;
  auto currentLabel = this->GetMultiLabelSegmentation()->GetLabel(this->GetSelectedLabels().front());
  try
  {
    this->WaitCursorOn();

    if (currentLabel.IsNull())
      mitkThrow() << "Invalid state context menu action was triggered with an invalid label id. Label id: " << this->GetSelectedLabels().front();

    auto pixelValue = currentLabel->GetValue();

    mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
    cropFilter->SetInput(mitk::CreateLabelMask(this->GetMultiLabelSegmentation(),pixelValue));
    cropFilter->SetBackgroundValue(0);
    cropFilter->SetMarginFactor(1.15);
    cropFilter->Update();

    maskImage = cropFilter->GetOutput();

    this->WaitCursorOff();
  }
  catch (mitk::Exception &e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  if (maskImage.IsNull())
  {
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  mitk::DataNode::Pointer maskNode = mitk::DataNode::New();
  std::string name = currentLabel->GetName();
  name += "-mask";
  maskNode->SetName(name);
  maskNode->SetData(maskImage);
  maskNode->SetBoolProperty("binary", true);
  maskNode->SetBoolProperty("outline binary", true);
  maskNode->SetBoolProperty("outline binary shadow", true);
  maskNode->SetFloatProperty("outline width", 2.0);
  maskNode->SetColor(currentLabel->GetColor());
  maskNode->SetOpacity(1.0);

  m_DataStorage->Add(maskNode, this->GetMultiLabelNode());
}

void QmitkMultiLabelManager::OnCreateMask(bool /*triggered*/)
{
  mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

  auto currentLabel = this->GetMultiLabelSegmentation()->GetLabel(this->GetSelectedLabels().front());
  mitk::Image::Pointer maskImage;
  try
  {
    this->WaitCursorOn();

    if (currentLabel.IsNull())
      mitkThrow() << "Invalid state context menu action was triggered with an invalid label id. Label id: " << this->GetSelectedLabels().front();
    auto pixelValue = currentLabel->GetValue();

    maskImage = mitk::CreateLabelMask(GetMultiLabelSegmentation(),pixelValue);
    this->WaitCursorOff();
  }
  catch (mitk::Exception &e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  if (maskImage.IsNull())
  {
    QMessageBox::information(this, "Create Mask", "Could not create a mask out of the selected label.\n");
    return;
  }

  mitk::DataNode::Pointer maskNode = mitk::DataNode::New();
  std::string name = currentLabel->GetName();
  name += "-mask";
  maskNode->SetName(name);
  maskNode->SetData(maskImage);
  maskNode->SetBoolProperty("binary", true);
  maskNode->SetBoolProperty("outline binary", true);
  maskNode->SetBoolProperty("outline binary shadow", true);
  maskNode->SetFloatProperty("outline width", 2.0);
  maskNode->SetColor(currentLabel->GetColor());
  maskNode->SetOpacity(1.0);

  m_DataStorage->Add(maskNode, this->GetMultiLabelNode());
}

void QmitkMultiLabelManager::OnCreateSmoothedSurface(bool /*triggered*/)
{
  mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

  auto currentLabel = this->GetMultiLabelSegmentation()->GetLabel(this->GetSelectedLabels().front());

  try
  {
    if (currentLabel.IsNull())
      mitkThrow() << "Invalid state context menu action was triggered with an invalid label id. Label id: " << this->GetSelectedLabels().front();

    auto pixelValue = currentLabel->GetValue();

    mitk::LabelSetImageToSurfaceThreadedFilter::Pointer surfaceFilter = mitk::LabelSetImageToSurfaceThreadedFilter::New();

    itk::SimpleMemberCommand<QmitkMultiLabelManager>::Pointer successCommand =
      itk::SimpleMemberCommand<QmitkMultiLabelManager>::New();
    successCommand->SetCallbackFunction(this, &QmitkMultiLabelManager::OnThreadedCalculationDone);
    surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

    itk::SimpleMemberCommand<QmitkMultiLabelManager>::Pointer errorCommand =
      itk::SimpleMemberCommand<QmitkMultiLabelManager>::New();
    errorCommand->SetCallbackFunction(this, &QmitkMultiLabelManager::OnThreadedCalculationDone);
    surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

    mitk::DataNode::Pointer groupNode = this->GetMultiLabelNode();
    surfaceFilter->SetPointerParameter("Group node", groupNode);
    surfaceFilter->SetPointerParameter("Input", this->GetMultiLabelSegmentation());
    surfaceFilter->SetParameter("RequestedLabel", pixelValue);
    surfaceFilter->SetParameter("Smooth", true);
    surfaceFilter->SetDataStorage(*m_DataStorage);

    mitk::StatusBar::GetInstance()->DisplayText("Surface creation is running in background...");

    surfaceFilter->StartAlgorithm();
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this,
                             "Create Surface",
                             "Could not create a surface mesh out of the selected label. See error log for details.\n");
  }
}

void QmitkMultiLabelManager::OnCreateDetailedSurface(bool /*triggered*/)
{
  mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

  auto currentLabel = this->GetMultiLabelSegmentation()->GetLabel(this->GetSelectedLabels().front());

  try
  {
    if (currentLabel.IsNull())
      mitkThrow() << "Invalid state context menu action was triggered with an invalid label id. Label id: " << this->GetSelectedLabels().front();

    auto pixelValue = currentLabel->GetValue();

    mitk::LabelSetImageToSurfaceThreadedFilter::Pointer surfaceFilter = mitk::LabelSetImageToSurfaceThreadedFilter::New();

    itk::SimpleMemberCommand<QmitkMultiLabelManager>::Pointer successCommand =
      itk::SimpleMemberCommand<QmitkMultiLabelManager>::New();
    successCommand->SetCallbackFunction(this, &QmitkMultiLabelManager::OnThreadedCalculationDone);
    surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

    itk::SimpleMemberCommand<QmitkMultiLabelManager>::Pointer errorCommand =
      itk::SimpleMemberCommand<QmitkMultiLabelManager>::New();
    errorCommand->SetCallbackFunction(this, &QmitkMultiLabelManager::OnThreadedCalculationDone);
    surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

    mitk::DataNode::Pointer groupNode = this->GetMultiLabelNode();
    surfaceFilter->SetPointerParameter("Group node", groupNode);
    surfaceFilter->SetPointerParameter("Input", this->GetMultiLabelSegmentation());
    surfaceFilter->SetParameter("RequestedLabel", pixelValue);
    surfaceFilter->SetParameter("Smooth", false);
    surfaceFilter->SetDataStorage(*m_DataStorage);

    mitk::StatusBar::GetInstance()->DisplayText("Surface creation is running in background...");

    surfaceFilter->StartAlgorithm();
  }
  catch (mitk::Exception &e)
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this,
                             "Create Surface",
                             "Could not create a surface mesh out of the selected label. See error log for details.\n");
  }
}

void QmitkMultiLabelManager::OnSavePreset()
{
  QmitkSaveMultiLabelPreset(this->GetMultiLabelSegmentation());
}

void QmitkMultiLabelManager::OnLoadPreset()
{
  QmitkLoadMultiLabelPreset({ this->GetMultiLabelSegmentation() });
}

void QmitkMultiLabelManager::OnGoToLabel(mitk::MultiLabelSegmentation::LabelValueType label, const mitk::Point3D& position) const
{
  emit GoToLabel(label, position);
}

void QmitkMultiLabelManager::OnLabelRenameRequested(mitk::Label* label, bool rename, bool& canceled) const
{
  emit LabelRenameRequested(label, rename, canceled);
}

void QmitkMultiLabelManager::WaitCursorOn()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkMultiLabelManager::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkMultiLabelManager::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

void QmitkMultiLabelManager::OnThreadedCalculationDone()
{
  mitk::StatusBar::GetInstance()->Clear();
}

void QmitkMultiLabelManager::AddSegmentationObserver()
{
  if (this->GetMultiLabelSegmentation() != nullptr)
  {
    auto& widget = *this;
    m_LabelAddedObserver.Reset(this->GetMultiLabelSegmentation(), mitk::LabelAddedEvent(), [&widget](const itk::EventObject& event)
      {
        auto labelEvent = dynamic_cast<const mitk::AnyLabelEvent*>(&event);
        widget.OnLabelEvent(labelEvent->GetLabelValue());
      });
    m_LabelModifiedObserver.Reset(this->GetMultiLabelSegmentation(), mitk::LabelModifiedEvent(), [&widget](const itk::EventObject& event)
      {
        auto labelEvent = dynamic_cast<const mitk::AnyLabelEvent*>(&event);
        widget.OnLabelEvent(labelEvent->GetLabelValue());
      });
    m_LabelRemovedObserver.Reset(this->GetMultiLabelSegmentation(), mitk::LabelRemovedEvent(), [&widget](const itk::EventObject& event)
      {
        auto labelEvent = dynamic_cast<const mitk::AnyLabelEvent*>(&event);
        widget.OnLabelEvent(labelEvent->GetLabelValue());
      });

    m_GroupAddedObserver.Reset(this->GetMultiLabelSegmentation(), mitk::GroupAddedEvent(), [&widget](const itk::EventObject& event)
      {
        auto groupEvent = dynamic_cast<const mitk::AnyGroupEvent*>(&event);
        widget.OnGroupEvent(groupEvent->GetGroupID());
      });
    m_GroupModifiedObserver.Reset(this->GetMultiLabelSegmentation(), mitk::GroupModifiedEvent(), [&widget](const itk::EventObject& event)
      {
        auto groupEvent = dynamic_cast<const mitk::AnyGroupEvent*>(&event);
        widget.OnGroupEvent(groupEvent->GetGroupID());
      });
    m_GroupRemovedObserver.Reset(this->GetMultiLabelSegmentation(), mitk::GroupRemovedEvent(), [&widget](const itk::EventObject& event)
      {
        auto groupEvent = dynamic_cast<const mitk::AnyGroupEvent*>(&event);
        widget.OnGroupEvent(groupEvent->GetGroupID());
      });
  }
}

void QmitkMultiLabelManager::RemoveSegmentationObserver()
{
  m_LabelAddedObserver.Reset();
  m_LabelModifiedObserver.Reset();
  m_LabelRemovedObserver.Reset();
  m_GroupAddedObserver.Reset();
  m_GroupModifiedObserver.Reset();
  m_GroupRemovedObserver.Reset();
}

void QmitkMultiLabelManager::OnLabelEvent(mitk::MultiLabelSegmentation::LabelValueType /*labelValue*/)
{
  if (!m_Controls->labelInspector->GetModelManipulationOngoing())
    this->UpdateControls();
}

void QmitkMultiLabelManager::OnGroupEvent(mitk::MultiLabelSegmentation::GroupIndexType /*groupIndex*/)
{
  if (!m_Controls->labelInspector->GetModelManipulationOngoing())
    this->UpdateControls();
}

void QmitkMultiLabelManager::OnModelUpdated()
{
  this->UpdateControls();
}

void QmitkMultiLabelManager::OnSegmentationChanged()
{
    this->RemoveSegmentationObserver();
    this->AddSegmentationObserver();
    UpdateControls();
}
