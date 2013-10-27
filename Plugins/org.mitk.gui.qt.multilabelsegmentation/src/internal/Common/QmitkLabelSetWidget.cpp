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

// blueberry
#include <berryPlatform.h>

// mitk
#include <mitkIDataStorageService.h>
#include <mitkLabelSetImage.h>
#include <mitkToolManagerProvider.h>
#include <mitkLabelSetImageToSurfaceThreadedFilter.h>
#include <mitkNrrdLabelSetImageReader.h>
#include <mitkNrrdLabelSetImageWriter.h>
#include <mitkRenderingManager.h>
#include <mitkStatusBar.h>
#include <mitkIOUtil.h>
#include <mitkCoreObjectFactory.h>
#include <mitkSurfaceInterpolationController.h>
#include <mitkSegmentationInterpolationController.h>

// Qmitk
#include <QmitkDataStorageComboBox.h>
#include <QmitkNewSegmentationDialog.h>

// Qt
#include <QCompleter>
#include <QStringListModel>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>

#include <itksys/SystemTools.hxx>

QmitkLabelSetWidget::QmitkLabelSetWidget(QWidget* parent) : QWidget(parent),
m_ToolManager(NULL),
m_IRenderWindowPart(NULL)
{
  m_Controls.setupUi(this);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);

  // react whenever the set of selected segmentation changes
  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkLabelSetWidget>( this, &QmitkLabelSetWidget::OnToolManagerWorkingDataModified );

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(activeLabelChanged(int)), this, SLOT(OnActiveLabelChanged(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(newLabel()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(renameLabel(int, const mitk::Color&, const std::string&)), this, SLOT(OnRenameLabel(int, const mitk::Color&, const std::string&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createSurface(int)), this, SLOT(OnCreateSurface(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(smoothLabel(int)), this, SLOT(OnSmoothLabel(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(toggleOutline(bool)), this, SLOT(OnToggleOutline(bool)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SLOT(OnGoToLabel(const mitk::Point3D&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateSurface( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createMask(int)), this, SLOT(OnCreateMask(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateMask( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_btNewSegmentation, SIGNAL(clicked()), this, SLOT( OnNewSegmentation()) );
  connect( m_Controls.m_btSaveSegmentation, SIGNAL(clicked()), this, SLOT( OnSaveSegmentation()) );
  connect( m_Controls.m_btLoadSegmentation, SIGNAL(clicked()), this, SLOT( OnLoadSegmentation()) );
  connect( m_Controls.m_btDeleteSegmentation, SIGNAL(clicked()), this, SLOT( OnDeleteSegmentation()) );
  connect( m_Controls.m_btImportSegmentation, SIGNAL(clicked()), this, SLOT( OnImportSegmentation()) );
  connect( m_Controls.m_btImportLabeledImage, SIGNAL(clicked()), this, SLOT( OnImportLabeledImage()) );
  connect( m_Controls.m_btNewLabel, SIGNAL(clicked()), this, SLOT( OnNewLabel()) );
  connect( m_Controls.m_btAddLayer, SIGNAL(clicked()), this, SLOT( OnAddLayer()) );
  connect( m_Controls.m_btDeleteLayer, SIGNAL(clicked()), this, SLOT( OnDeleteLayer()) );
  connect( m_Controls.m_btPreviousLayer, SIGNAL(clicked()), this, SLOT( OnPreviousLayer()) );
  connect( m_Controls.m_btNextLayer, SIGNAL(clicked()), this, SLOT( OnNextLayer()) );
  connect( m_Controls.m_cbActiveLayer, SIGNAL(currentIndexChanged(int)), this, SLOT( OnChangetLayer(int)) );

  m_Controls.m_LabelSearchBox->setAlwaysShowClearIcon(true);
  m_Controls.m_LabelSearchBox->setShowSearchIcon(true);

  QStringList completionList;
  completionList << "";
  m_Completer = new QCompleter(completionList,this);
  m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.m_LabelSearchBox->setCompleter(m_Completer);

  connect( m_Controls.m_LabelSearchBox, SIGNAL(returnPressed()), this, SLOT(OnSearchLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(labelListModified(const QStringList&)), this, SLOT( OnLabelListModified(const QStringList&)) );

  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(m_Controls.m_LabelSetTableWidget->GetLabelList());

 // m_Controls.m_LabelSetTableWidget->setEnabled(false);
  m_Controls.m_LabelSearchBox->setEnabled(false);
  m_Controls.m_btNewSegmentation->setEnabled(false);
  m_Controls.m_btLoadSegmentation->setEnabled(false);
  m_Controls.m_btSaveSegmentation->setEnabled(false);
  m_Controls.m_btNewLabel->setEnabled(false);
  m_Controls.m_btAddLayer->setEnabled(false);
  m_Controls.m_btDeleteLayer->setEnabled(false);
  m_Controls.m_btPreviousLayer->setEnabled(false);
  m_Controls.m_btNextLayer->setEnabled(false);

  m_Controls.m_btDeleteSegmentation->setEnabled(false);
  m_Controls.m_btImportSegmentation->setEnabled(false);
  m_Controls.m_btImportLabeledImage->setEnabled(false);
}

QmitkLabelSetWidget::~QmitkLabelSetWidget()
{
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkLabelSetWidget>(this, &QmitkLabelSetWidget::OnToolManagerWorkingDataModified);
}

void QmitkLabelSetWidget::setEnabled(bool enabled)
{
  m_Controls.m_LabelSetTableWidget->setEnabled(enabled);
  m_Controls.m_btNewSegmentation->setEnabled(enabled);
  m_Controls.m_btLoadSegmentation->setEnabled(enabled);
  m_Controls.m_btImportLabeledImage->setEnabled(enabled);

  QWidget::setEnabled(enabled);
}

void QmitkLabelSetWidget::SetDataStorage( mitk::DataStorage& storage )
{
  m_DataStorage = &storage;
}

void QmitkLabelSetWidget::SetRenderWindowPart( mitk::IRenderWindowPart* renderWindowPart )
{
  m_IRenderWindowPart = renderWindowPart;
}

void QmitkLabelSetWidget::SetPreferences( berry::IPreferences::Pointer prefs )
{
  m_Preferences = prefs;
}

void QmitkLabelSetWidget::OnToolManagerWorkingDataModified()
{
  this->UpdateControls();
}

void QmitkLabelSetWidget::OnGoToLabel(const mitk::Point3D& pos)
{
  if (m_IRenderWindowPart)
    m_IRenderWindowPart->SetSelectedPosition(pos);
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

void QmitkLabelSetWidget::OnRenameLabel(int index, const mitk::Color& color, const std::string& name)
{
  m_ToolManager->ActivateTool(-1);

  // ask about the name and organ type of the new segmentation
  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );

  QString storedList = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
  QStringList organColors;
  if (storedList.isEmpty())
  {
    organColors = GetDefaultOrganColorString();
  }
  else
  {
    // recover string list from BlueBerry view's preferences
    QString storedString = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
    // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
    QRegExp onePart("(?:[^;]|\\\\;)*");
    int count = 0;
    int pos = 0;
    while( (pos = onePart.indexIn( storedString, pos )) != -1 )
    {
      ++count;
      int length = onePart.matchedLength();
      if (length == 0) break;
      QString matchedString = storedString.mid(pos, length);
      MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
      pos += length + 1; // skip separating ';'

      // unescape possible occurrences of '\;' in the string
      matchedString.replace("\\;", ";");

      // add matched string part to output list
      organColors << matchedString;
    }
  }

  dialog->setWindowTitle("Rename Label");
  dialog->SetSuggestionList( organColors );
  dialog->SetColor(color);
  dialog->SetSegmentationName(name);

  int dialogReturnValue = dialog->exec();

  if ( dialogReturnValue == QDialog::Rejected ) return;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->RenameLabel(index, dialog->GetSegmentationName().toStdString(), dialog->GetColor());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

void QmitkLabelSetWidget::OnChangetLayer(int layer)
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

  QString question = "Do you really want to add a layer the current segmentation session?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Add layer",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes) return;

  try
  {
    this->WaitCursorOn();
    workingImage->AddLayer();
    workingImage->SetActiveLayer( workingImage->GetActiveLayer() - 1 );
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Add Layer", "Could not add a new layer. See error log for details.\n");
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

void QmitkLabelSetWidget::UpdateControls()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  bool enabled = (workingNode != NULL);

  m_Controls.m_LabelSetTableWidget->setEnabled(enabled);
  m_Controls.m_LabelSearchBox->setEnabled(enabled);
  m_Controls.m_btSaveSegmentation->setEnabled(enabled);
  m_Controls.m_btNewLabel->setEnabled(enabled);
  m_Controls.m_btAddLayer->setEnabled(enabled);
  m_Controls.m_btDeleteLayer->setEnabled(enabled);
  m_Controls.m_btPreviousLayer->setEnabled(enabled);
  m_Controls.m_btNextLayer->setEnabled(enabled);
  m_Controls.m_btDeleteSegmentation->setEnabled(enabled);
  m_Controls.m_btImportSegmentation->setEnabled(enabled);

  if (!enabled) return;

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
}

void QmitkLabelSetWidget::OnActiveLabelChanged(int activeLabel)
{
  mitk::SurfaceInterpolationController* interpolator = mitk::SurfaceInterpolationController::GetInstance();
  if (interpolator)
    interpolator->SetActiveLabel(activeLabel);
}

void QmitkLabelSetWidget::OnNewLabel()
{
  m_ToolManager->ActivateTool(-1);

  // ask about the name and organ type of the new segmentation
  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );

  QString storedList = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
  QStringList organColors;
  if (storedList.isEmpty())
  {
    organColors = GetDefaultOrganColorString();
  }
  else
  {
    /*
    a couple of examples of how organ names are stored:

    a simple item is built up like 'name#AABBCC' where #AABBCC is the hexadecimal notation of a color as known from HTML

    items are stored separated by ';'
    this makes it necessary to escape occurrences of ';' in name.
    otherwise the string "hugo;ypsilon#AABBCC;eugen#AABBCC" could not be parsed as two organs
    but we would get "hugo" and "ypsilon#AABBCC" and "eugen#AABBCC"

    so the organ name "hugo;ypsilon" is stored as "hugo\;ypsilon"
    and must be unescaped after loading

    the following lines could be one split with Perl's negative lookbehind
    */

    // recover string list from BlueBerry view's preferences
    QString storedString = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
    MITK_DEBUG << "storedString: " << storedString.toStdString();
    // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
    QRegExp onePart("(?:[^;]|\\\\;)*");
    MITK_DEBUG << "matching " << onePart.pattern().toStdString();
    int count = 0;
    int pos = 0;
    while( (pos = onePart.indexIn( storedString, pos )) != -1 )
    {
      ++count;
      int length = onePart.matchedLength();
      if (length == 0) break;
      QString matchedString = storedString.mid(pos, length);
      MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
      pos += length + 1; // skip separating ';'

      // unescape possible occurrences of '\;' in the string
      matchedString.replace("\\;", ";");

      // add matched string part to output list
      organColors << matchedString;
    }
    MITK_DEBUG << "Captured " << count << " organ name/colors";
  }

  dialog->SetSuggestionList( organColors );

  dialog->setWindowTitle("New Label");

  int dialogReturnValue = dialog->exec();

  if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

  mitk::Color color = dialog->GetColor();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->AddLabel(dialog->GetSegmentationName().toStdString(), color);
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

  this->m_DataStorage->Add(maskNode, workingNode);
}

void QmitkLabelSetWidget::OnToggleOutline(bool value)
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  workingNode->SetBoolProperty( "labelset.contour.all", value);
  workingNode->GetData()->Modified(); // fixme: workaround to force data-type rendering (and not only property-type)
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnSmoothLabel(int index)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  try
  {
    this->WaitCursorOn();
    workingImage->SmoothLabel(index);
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Smooth Label", "Could not smooth the selected label. See error log for details.\n");
  }
}

void QmitkLabelSetWidget::OnCreateSurface(int index)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  mitk::LabelSetImageToSurfaceThreadedFilter::Pointer filter =
     mitk::LabelSetImageToSurfaceThreadedFilter::New();

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer successCommand = itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  successCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  filter->AddObserver(mitk::ResultAvailable(), successCommand);

  itk::SimpleMemberCommand<QmitkLabelSetWidget>::Pointer errorCommand = itk::SimpleMemberCommand<QmitkLabelSetWidget>::New();
  errorCommand->SetCallbackFunction(this, &QmitkLabelSetWidget::OnThreadedCalculationDone);
  filter->AddObserver(mitk::ProcessingError(), errorCommand);

  mitk::DataNode::Pointer groupNode = workingNode;
  filter->SetPointerParameter("Group node", groupNode);
  filter->SetPointerParameter("Input", workingImage);
  filter->SetParameter("RequestedLabel", index);
  filter->SetDataStorage( *m_DataStorage );

  mitk::StatusBar::GetInstance()->DisplayText("Surface creation is running in background...");

  try
  {
    filter->StartAlgorithm();
  }
  catch ( mitk::Exception & e )
  {
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Create Surface", "Could not create a surface mesh out of the selected label. See error log for details.\n");
  }
}

void QmitkLabelSetWidget::OnImportLabeledImage()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  assert(referenceNode);

  // Ask the user for a list of files to open
  QStringList fileNames = QFileDialog::getOpenFileNames( this, "Open Image", this->GetLastFileOpenPath(),
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

    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    std::string newName = referenceNode->GetName();
    newName += "-labels";
    newNode->SetName(newName);
    newNode->SetData(newImage);
    m_DataStorage->Add(newNode, referenceNode);
    this->WaitCursorOff();
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
}

void QmitkLabelSetWidget::OnImportSegmentation()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );
  assert(workingImage);

  std::string fileExtensions("Segmentation files (*.lset);;");
  QString qfileName = QFileDialog::getOpenFileName(this, "Import Segmentation", this->GetLastFileOpenPath(), fileExtensions.c_str() );
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

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnLoadSegmentation()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  assert(referenceNode);

  mitk::Image* referenceImage = dynamic_cast<mitk::Image*>( referenceNode->GetData() );
  assert(referenceImage);

  std::string fileExtensions("Segmentation files (*.lset);;");
  QString qfileName = QFileDialog::getOpenFileName(this, "Load Segmentation", this->GetLastFileOpenPath(), fileExtensions.c_str() );
  if (qfileName.isEmpty() ) return;

  mitk::NrrdLabelSetImageReader::Pointer reader = mitk::NrrdLabelSetImageReader::New();
  reader->SetFileName(qfileName.toLatin1());

  try
  {
    this->WaitCursorOn();
    reader->Update();
    this->WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Load Segmentation", "Could not load the selected file. See error log for details.\n");
  }

  this->SetLastFileOpenPath(qfileName);

  mitk::LabelSetImage::Pointer newImage = reader->GetOutput();

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData(newImage);
  newNode->SetName(newImage->GetName());

  m_DataStorage->Add(newNode,referenceNode);

  m_Controls.m_cbActiveLayer->clear();
  for (int lidx=0; lidx<newImage->GetNumberOfLayers(); ++lidx)
  {
    m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
  }

  this->UpdateControls();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnSaveSegmentation()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  // update the name in case has changed
  workingImage->SetName( workingNode->GetName() );

  //set last modification date and time
  QDateTime cTime = QDateTime::currentDateTime ();
  workingImage->SetLastModificationTime( cTime.toString().toStdString() );

  QString selected_suffix("segmentation files (*.lset)");
  QString qfileName = QFileDialog::getSaveFileName(this, "Save Segmentation", this->GetLastFileOpenPath(), selected_suffix);
  if (qfileName.isEmpty()) return;

  mitk::NrrdLabelSetImageWriter::Pointer writer = mitk::NrrdLabelSetImageWriter::New();
  writer->SetFileName(qfileName.toStdString());
  std::string newName = itksys::SystemTools::GetFilenameWithoutExtension(qfileName.toStdString());
  workingImage->SetName(newName);
  workingNode->SetName(newName);
  writer->SetInput(workingImage);

  try
  {
    this->WaitCursorOn();
    writer->Update();
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(this, "Save Segmenation", "Could not save the active segmentation. See error log for details.");
  }
}

void QmitkLabelSetWidget::OnDeleteSegmentation()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  m_DataStorage->Remove(workingNode);
}

void QmitkLabelSetWidget::OnNewSegmentation()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  assert(referenceNode);

  mitk::Image* referenceImage = dynamic_cast<mitk::Image*>( referenceNode->GetData() );
  assert(referenceImage);

  QString name = QString::fromStdString(referenceNode->GetName());
  name.append("-labels");

  mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
  newImage->Initialize(referenceImage);

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData(newImage);
  newNode->SetName(name.toStdString());
  newImage->SetName(name.toStdString());

  m_DataStorage->Add(newNode,referenceNode);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

//  this->OnNewLabel();
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

QString QmitkLabelSetWidget::GetLastFileOpenPath() const
{
  if(m_Preferences.IsNotNull())
  {
    return QString::fromStdString(m_Preferences->Get("LastFileOpenPath", ""));
  }
  return QString();
}

void QmitkLabelSetWidget::SetLastFileOpenPath(const QString& path) const
{
  if(m_Preferences.IsNotNull())
  {
    m_Preferences->Put("LastFileOpenPath", path.toStdString());
    m_Preferences->Flush();
  }
}
