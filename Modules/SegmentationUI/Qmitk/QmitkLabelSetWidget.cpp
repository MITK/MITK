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

#include "QmitkLabelSetWidget.h"

// mitk
#include <mitkLabelSetImage.h>
#include <mitkToolManagerProvider.h>
#include <mitkShowSegmentationAsSurface.h>
#include <mitkLabelSetImageToSurfaceThreadedFilter.h>
#include <mitkRenderingManager.h>
#include <mitkStatusBar.h>
#include <mitkIOUtil.h>
#include <mitkCoreObjectFactory.h>
#include <mitkAutoCropImageFilter.h>
#include <mitkSurfaceInterpolationController.h>
#include <mitkSegmentationInterpolationController.h>

// Qmitk
#include <QmitkDataStorageComboBox.h>
#include <QmitkNewSegmentationDialog.h>
#include <QmitkSearchLabelDialog.h>

// Qt
#include <QCompleter>
#include <QStringListModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>

// itk
#include <itksys/SystemTools.hxx>

// todo:
// berry
//#include <berryIPreferencesService.h>

QmitkLabelSetWidget::QmitkLabelSetWidget(QWidget* parent)
: QWidget(parent)
, m_ToolManager(NULL)
, m_DataStorage(NULL)
, m_Completer(NULL)
{
  m_Controls.setupUi(this);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(activeLabelChanged(int)), this, SLOT(OnActiveLabelChanged(int)) );
  //connect( m_Controls.m_LabelSetTableWidget, SIGNAL(importSegmentation()), this, SLOT( OnImportSegmentation()) );
  //connect( m_Controls.m_LabelSetTableWidget, SIGNAL(importLabeledImage()), this, SLOT( OnImportLabeledImage()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(renameLabel(int, const mitk::Color&, const std::string&)), this, SLOT(OnRenameLabel(int, const mitk::Color&, const std::string&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createSurface(int, bool)), this, SLOT(OnCreateSurface(int, bool)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(toggleOutline(bool)), this, SLOT(OnToggleOutline(bool)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SIGNAL(goToLabel(const mitk::Point3D&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateSurface( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createMask(int)), this, SLOT(OnCreateMask(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createCroppedMask(int)), this, SLOT(OnCreateCroppedMask(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateMask( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_btAddLayer, SIGNAL(clicked()), this, SLOT( OnAddLayer()) );
  connect( m_Controls.m_btDeleteLayer, SIGNAL(clicked()), this, SLOT( OnDeleteLayer()) );
  connect( m_Controls.m_btPreviousLayer, SIGNAL(clicked()), this, SLOT( OnPreviousLayer()) );
  connect( m_Controls.m_btNextLayer, SIGNAL(clicked()), this, SLOT( OnNextLayer()) );

  connect( m_Controls.m_btLockExterior, SIGNAL(toggled(bool)), this, SLOT( OnLockExteriorToggled(bool)) );
  connect( m_Controls.m_btDeactivateTool, SIGNAL(clicked()), this, SLOT( OnDeactivateActiveTool()) );

  connect( m_Controls.m_cbActiveLayer, SIGNAL(currentIndexChanged(int)), this, SLOT( OnChangeLayer(int)) );

  m_Controls.m_LabelSearchBox->setAlwaysShowClearIcon(true);
  m_Controls.m_LabelSearchBox->setShowSearchIcon(true);

  QStringList completionList;
  completionList << "";
  m_Completer = new QCompleter(completionList, this);
  m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.m_LabelSearchBox->setCompleter(m_Completer);

  connect( m_Controls.m_LabelSearchBox, SIGNAL(returnPressed()), this, SLOT(OnSearchLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(labelListModified(const QStringList&)), this, SLOT( OnLabelListModified(const QStringList&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(mergeLabel(int)), this, SLOT( OnMergeLabel(int)) );

  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(m_Controls.m_LabelSetTableWidget->GetLabelStringList());

  m_Controls.m_LabelSearchBox->setEnabled(false);
  m_Controls.m_btLockExterior->setEnabled(false);
  m_Controls.m_btDeactivateTool->setEnabled(false);
  m_Controls.m_btAddLayer->setEnabled(false);
  m_Controls.m_btDeleteLayer->setEnabled(false);
  m_Controls.m_btPreviousLayer->setEnabled(false);
  m_Controls.m_btNextLayer->setEnabled(false);

  m_Controls.m_lblCaption->setText("");
}

QmitkLabelSetWidget::~QmitkLabelSetWidget()
{
}

void QmitkLabelSetWidget::setEnabled(bool enabled)
{
  QWidget::setEnabled(enabled);

  this->UpdateControls();
}

void QmitkLabelSetWidget::SetDataStorage( mitk::DataStorage* storage )
{
  m_DataStorage = storage;
}

void QmitkLabelSetWidget::OnSearchLabel()
{
  m_Controls.m_LabelSetTableWidget->SetActiveLabel(m_Controls.m_LabelSearchBox->text().toStdString());
}

void QmitkLabelSetWidget::OnLabelListModified(const QStringList& list)
{
  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(list);
}

void QmitkLabelSetWidget::OnMergeLabel(int index)
{
  QmitkSearchLabelDialog dialog(this);
  dialog.setWindowTitle("Select a second label..");
  dialog.SetLabelSuggestionList(m_Controls.m_LabelSetTableWidget->GetLabelStringList());
  int dialogReturnValue = dialog.exec();
  if ( dialogReturnValue == QDialog::Rejected ) return;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->MergeLabel(dialog.GetLabelIndex());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::SetOrganColors(const QStringList& organColors)
{
  m_OrganColors = organColors;
}

void QmitkLabelSetWidget::OnRenameLabel(int index, const mitk::Color& color, const std::string& name)
{
  QmitkNewSegmentationDialog dialog(this);
  dialog.setWindowTitle("Rename Label");
  dialog.SetSuggestionList( m_OrganColors );
  dialog.SetColor(color);
  dialog.SetSegmentationName(name);

  int dialogReturnValue = dialog.exec();

  if ( dialogReturnValue == QDialog::Rejected ) return;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->RenameLabel(index, dialog.GetSegmentationName().toStdString(), dialog.GetColor());

  this->UpdateControls();
}

void QmitkLabelSetWidget::OnPreviousLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  try
  {
    this->WaitCursorOn();
    workingImage->SetActiveLayer( workingImage->GetActiveLayer() - 1 );
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Previous Layer", "Could not change to previous layer. See error log for details.\n");
    return;
  }

  m_Controls.m_LabelSetTableWidget->Reset();

  this->UpdateControls();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnNextLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  try
  {
    this->WaitCursorOn();
    workingImage->SetActiveLayer( workingImage->GetActiveLayer() + 1 );
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Next Layer", "Could not change to next layer. See error log for details.\n");
    return;
  }

  m_Controls.m_LabelSetTableWidget->Reset();

  this->UpdateControls();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnChangeLayer(int layer)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  try
  {
    this->WaitCursorOn();
    workingImage->SetActiveLayer( layer );
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Next Layer", "Could not change to next layer. See error log for details.\n");
    return;
  }

  m_Controls.m_LabelSetTableWidget->Reset();

  this->UpdateControls();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnDeleteLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  if (workingImage->GetNumberOfLayers() < 2)
    return;

  QString question = "Do you really want to delete the current layer?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Delete layer",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes) return;

  try
  {
    this->WaitCursorOn();
    workingImage->RemoveLayer();
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Delete Layer", "Could not delete the currently active layer. See error log for details.\n");
    return;
  }

  m_Controls.m_LabelSetTableWidget->Reset();
  m_Controls.m_cbActiveLayer->clear();
  for (int lidx=0; lidx<workingImage->GetNumberOfLayers(); ++lidx)
  {
    m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
  }

  this->UpdateControls();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnAddLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  QString question = "Do you really want to add a layer to the current segmentation session?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Add layer",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes) return;

  try
  {
    this->WaitCursorOn();
    workingImage->AddLayer();
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Add Layer", "Could not add a new layer. See error log for details.\n");
    return;
  }

  m_Controls.m_cbActiveLayer->clear();
  for (int lidx=0; lidx<workingImage->GetNumberOfLayers(); ++lidx)
  {
    m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
  }

  this->OnChangeLayer(workingImage->GetNumberOfLayers()-1);

  QmitkNewSegmentationDialog dialog(this);
  dialog.setWindowTitle("New Label");
  dialog.SetSuggestionList( m_OrganColors );
  dialog.SetSegmentationName("unnamed");

  int dialogReturnValue = dialog.exec();

  if ( dialogReturnValue == QDialog::Rejected ) return;

  workingImage->AddLabel(dialog.GetSegmentationName().toStdString(), dialog.GetColor());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnDeactivateActiveTool()
{
  m_ToolManager->ActivateTool(-1);
}

void QmitkLabelSetWidget::OnLockExteriorToggled(bool checked)
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->SetLabelLocked(0, checked);
}

void QmitkLabelSetWidget::UpdateControls()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  bool hasWorkingData = (workingNode != NULL);

  m_Controls.m_LabelSetTableWidget->setEnabled(hasWorkingData);
  m_Controls.m_LabelSearchBox->setEnabled(hasWorkingData);
  m_Controls.m_btLockExterior->setEnabled(hasWorkingData);
  m_Controls.m_btDeactivateTool->setEnabled(hasWorkingData);
  m_Controls.m_btAddLayer->setEnabled(hasWorkingData);
  m_Controls.m_btDeleteLayer->setEnabled(hasWorkingData);
  m_Controls.m_btPreviousLayer->setEnabled(hasWorkingData);
  m_Controls.m_btNextLayer->setEnabled(hasWorkingData);

  if (!hasWorkingData) return;

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  int activeLayer = workingImage->GetActiveLayer();
  int numberOfLayers = workingImage->GetNumberOfLayers();
  m_Controls.m_btAddLayer->setEnabled(true);
  m_Controls.m_btDeleteLayer->setEnabled(numberOfLayers>1);
  m_Controls.m_btPreviousLayer->setEnabled(activeLayer>0);
  m_Controls.m_btNextLayer->setEnabled(activeLayer!=numberOfLayers-1);
  m_Controls.m_cbActiveLayer->setEnabled(numberOfLayers>1);
  m_Controls.m_cbActiveLayer->setCurrentIndex(activeLayer);
  m_Controls.m_btLockExterior->setChecked(workingImage->GetLabelLocked(0));

  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(m_Controls.m_LabelSetTableWidget->GetLabelStringList());

  std::stringstream captionText;
  captionText << "Number of labels: " << workingImage->GetNumberOfLabels() - 1;
  m_Controls.m_lblCaption->setText(captionText.str().c_str());
}

void QmitkLabelSetWidget::OnActiveLabelChanged(int activeLabel)
{
  mitk::SurfaceInterpolationController* interpolator = mitk::SurfaceInterpolationController::GetInstance();
  if (interpolator)
  {
    interpolator->SetActiveLabel(activeLabel);
  }
}

void QmitkLabelSetWidget::OnCreateCroppedMask(int index)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  mitk::Image::Pointer maskImage;

  try
  {
    this->WaitCursorOn();

    mitk::AutoCropImageFilter::Pointer cropFilter = mitk::AutoCropImageFilter::New();
    cropFilter->SetInput( workingImage->CreateLabelMask(index) );
    cropFilter->SetBackgroundValue( 0 );
    cropFilter->SetMarginFactor(1.15);
    cropFilter->Update();

    maskImage = cropFilter->GetOutput();

    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
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
  std::string name = workingImage->GetLabelName(index, workingImage->GetActiveLayer());
  name += "-mask";
  maskNode->SetName(name);
  maskNode->SetData(maskImage);
  maskNode->SetBoolProperty("binary", true);
  maskNode->SetBoolProperty("outline binary", true);
  maskNode->SetBoolProperty("outline binary shadow", true);
  maskNode->SetFloatProperty("outline width", 2.0);
  maskNode->SetColor(workingImage->GetLabelColor(index));
  maskNode->SetOpacity(1.0);

  m_DataStorage->Add(maskNode, workingNode);
}

void QmitkLabelSetWidget::OnCreateMask(int index)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  mitk::Image::Pointer maskImage;

  try
  {
    this->WaitCursorOn();
    maskImage = workingImage->CreateLabelMask(index);
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
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
  std::string name = workingImage->GetLabelName(index, workingImage->GetActiveLayer());
  name += "-mask";
  maskNode->SetName(name);
  maskNode->SetData(maskImage);
  maskNode->SetBoolProperty("binary", true);
  maskNode->SetBoolProperty("outline binary", true);
  maskNode->SetBoolProperty("outline binary shadow", true);
  maskNode->SetFloatProperty("outline width", 2.0);
  maskNode->SetColor(workingImage->GetLabelColor(index));
  maskNode->SetOpacity(1.0);

  m_DataStorage->Add(maskNode, workingNode);
}

void QmitkLabelSetWidget::OnToggleOutline(bool value)
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  workingNode->SetBoolProperty( "labelset.contour.all", value);
  workingNode->GetData()->Modified(); // fixme: workaround to force data-type rendering (and not only property-type)
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnCreateSurface(int index, bool smooth)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode::Pointer workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode.IsNotNull());

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  mitk::LabelSetImageToSurfaceThreadedFilter::Pointer surfaceFilter = mitk::LabelSetImageToSurfaceThreadedFilter::New();

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer successCommand = itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  successCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  surfaceFilter->AddObserver(mitk::ResultAvailable(), successCommand);

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer errorCommand = itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  errorCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  surfaceFilter->AddObserver(mitk::ProcessingError(), errorCommand);

  // todo: get smoothing and decimation parameters from preferences
  /*
  berry::IPreferencesService::Pointer prefService = Platform::GetServiceRegistry().GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);
  berry::IPreferences::Pointer segPref = prefService->GetSystemPreferences()->Node("/org.mitk.views.multilabelsegmentation");

  mitk::ScalarType smoothing = segPref->GetDouble("smoothing value", 0.1);
  mitk::ScalarType decimation = segPref->GetDouble("decimation rate", 0.5);
  */
  mitk::DataNode::Pointer groupNode = workingNode;
  surfaceFilter->SetPointerParameter("Group node", groupNode);
  surfaceFilter->SetPointerParameter("Input", workingImage);
  surfaceFilter->SetParameter("RequestedLabel", index);
  surfaceFilter->SetParameter("Smooth", smooth);
  surfaceFilter->SetDataStorage( *m_DataStorage );

  mitk::StatusBar::GetInstance()->DisplayText("Surface creation is running in background...");

  try
  {
    surfaceFilter->StartAlgorithm();
  }
  catch ( mitk::Exception & e )
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Create Surface", "Could not create a surface mesh out of the selected label. See error log for details.\n");
  }

}

void QmitkLabelSetWidget::OnImportLabeledImage()
{
/*
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  assert(referenceNode);

  // Ask the user for a list of files to open
  QStringList fileNames = QFileDialog::getOpenFileNames( this, "Open Image", m_LastFileOpenPath,
                                                        mitk::CoreObjectFactory::GetInstance()->GetFileExtensions());

  if (fileNames.empty())
    return;

  try
  {
    this->WaitCursorOn();
    mitk::Image::Pointer image = mitk::IOUtil::LoadImage( fileNames.front().toStdString() );
    if (image.IsNull())
    {
      this->WaitCursorOff();
      QMessageBox::information(this, "Import Labeled Image", "Could not load the selected segmentation.\n");
      return;
    }

    mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
    newImage->InitializeByLabeledImage(image);
    this->WaitCursorOff();

    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    std::string newName = referenceNode->GetName();
    newName += "-labels";
    newNode->SetName(newName);
    newNode->SetData(newImage);
    m_DataStorage->Add(newNode, referenceNode);
  }
  catch (mitk::Exception & e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Import Labeled Image", "Could not load the selected segmentation. See error log for details.\n");
    return;
   }

  this->UpdateControls();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  */
}

void QmitkLabelSetWidget::OnImportSegmentation()
{
/*
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  std::string fileExtensions("Segmentation files (*.lset);;");
  QString qfileName = QFileDialog::getOpenFileName(this, "Import Segmentation", m_LastFileOpenPath, fileExtensions.c_str() );
  if (qfileName.isEmpty() ) return;

  mitk::NrrdLabelSetImageReader::Pointer reader = mitk::NrrdLabelSetImageReader::New();
  reader->SetFileName(qfileName.toLatin1());

  try
  {
    this->WaitCursorOn();
    reader->Update();
    mitk::LabelSetImage::Pointer newImage = reader->GetOutput();
    workingImage->Concatenate(newImage);
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Import Segmentation", "Could not import the selected segmentation session.\n See error log for details.\n");
  }
*/
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges )
{

}

void QmitkLabelSetWidget::OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges )
{

}

void QmitkLabelSetWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkLabelSetWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

void QmitkLabelSetWidget::OnThreadedCalculationDone()
{
  mitk::StatusBar::GetInstance()->Clear();
}

