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

#include "QmitkMultiLabelSegmentationView.h"

// blueberry
#include <berryConstants.h>
#include <berryIWorkbenchPage.h>

// mitk
#include "mitkApplicationCursor.h"
#include "mitkLabelSetImage.h"
#include "mitkStatusBar.h"
#include "mitkToolManagerProvider.h"
//#include "mitkSegmentationObjectFactory.h"
#include "mitkDICOMTag.h"
#include "mitkDICOMTagPath.h"
#include "mitkIDICOMTagsOfInterest.h"
#include "mitkInteractionEventObserver.h"
#include "mitkPlanePositionManager.h"
#include "mitkPluginActivator.h"
#include "mitkPropertyNameHelper.h"
#include "mitkSegTool2D.h"

// Qmitk
#include "QmitkNewSegmentationDialog.h"
#include "QmitkRenderWindow.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

// Qt
#include <QDateTime>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "tinyxml.h"

#include <itksys/SystemTools.hxx>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView()
  : m_Parent(NULL),
    m_IRenderWindowPart(NULL),
    m_ReferenceNode(NULL),
    m_ToolManager(NULL),
    m_WorkingNode(NULL),
    m_MouseCursorSet(false)
{
  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateProperty::Pointer isBinary =
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isMask = mitk::NodePredicateAnd::New(isBinary, isImage);

  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate(isImage);
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isQbi);

  m_ReferencePredicate = mitk::NodePredicateAnd::New();
  m_ReferencePredicate->AddPredicate(validImages);
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(m_SegmentationPredicate));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(isMask));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
}

QmitkMultiLabelSegmentationView::~QmitkMultiLabelSegmentationView()
{
  // m_ToolManager->ActivateTool(-1);
  /*
  todo: check this
  m_Controls.m_SliceBasedInterpolatorWidget->EnableInterpolation(false);
  ctkPluginContext* context = mitk::PluginActivator::getContext();
  ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
  mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
  service->RemoveAllPlanePositions();
  context->ungetService(ppmRef);
  */
  // m_ToolManager->SetReferenceData(NULL);
  // m_ToolManager->SetWorkingData(NULL);

  // m_ServiceRegistration.Unregister();

  // Loose LabelSetConnections
  OnLooseLabelSetConnection();
}

void QmitkMultiLabelSegmentationView::CreateQtPartControl(QWidget *parent)
{
  // setup the basic GUI of this view
  m_Parent = parent;

  m_Controls.setupUi(parent);

  // *------------------------
  // * DATA SELECTION WIDGETS
  // *------------------------

  m_Controls.m_cbReferenceNodeSelector->SetAutoSelectNewItems(true);
  m_Controls.m_cbReferenceNodeSelector->SetPredicate(m_ReferencePredicate);
  m_Controls.m_cbReferenceNodeSelector->SetDataStorage(this->GetDataStorage());

  m_Controls.m_cbWorkingNodeSelector->SetAutoSelectNewItems(true);
  m_Controls.m_cbWorkingNodeSelector->SetPredicate(m_SegmentationPredicate);
  m_Controls.m_cbWorkingNodeSelector->SetDataStorage(this->GetDataStorage());

  connect(m_Controls.m_cbReferenceNodeSelector,
          SIGNAL(OnSelectionChanged(const mitk::DataNode *)),
          this,
          SLOT(OnReferenceSelectionChanged(const mitk::DataNode *)));

  connect(m_Controls.m_cbWorkingNodeSelector,
          SIGNAL(OnSelectionChanged(const mitk::DataNode *)),
          this,
          SLOT(OnSegmentationSelectionChanged(const mitk::DataNode *)));

  // *------------------------
  // * ToolManager
  // *------------------------

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);
  m_ToolManager->SetDataStorage(*(this->GetDataStorage()));
  m_ToolManager->InitializeTools();
  // use the same ToolManager instance for our 3D Tools
  m_Controls.m_ManualToolSelectionBox3D->SetToolManager(*m_ToolManager);

  // *------------------------
  // * LabelSetWidget
  // *------------------------

  m_Controls.m_LabelSetWidget->SetDataStorage(this->GetDataStorage());
  m_Controls.m_LabelSetWidget->SetOrganColors(mitk::OrganNamesHandling::GetDefaultOrganColorString());
  m_Controls.m_LabelSetWidget->hide();

  // *------------------------
  // * Interpolation
  // *------------------------

  m_Controls.m_SurfaceBasedInterpolatorWidget->SetDataStorage(*(this->GetDataStorage()));
  m_Controls.m_SliceBasedInterpolatorWidget->SetDataStorage(*(this->GetDataStorage()));
  connect(m_Controls.m_cbInterpolation, SIGNAL(activated(int)), this, SLOT(OnInterpolationSelectionChanged(int)));

  m_Controls.m_cbInterpolation->setCurrentIndex(0);
  m_Controls.m_swInterpolation->hide();

  // *------------------------
  // * ToolSelection 2D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox2D->SetToolGUIArea(m_Controls.m_ManualToolGUIContainer2D);
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups(
    "Add Subtract Fill Erase Paint Wipe 'Region Growing' FastMarching2D Correction 'Live Wire'"); // todo: "Correction
  // 'Live Wire'"
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
  connect(m_Controls.m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)));

  // *------------------------
  // * ToolSelection 3D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea(m_Controls.m_ManualToolGUIContainer3D);
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups(
    "Threshold 'Two Thresholds' 'Auto Threshold' 'Multiple Otsu'"); // todo add : FastMarching3D RegionGrowing Watershed
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

  // *------------------------*
  // * Connect PushButtons (pb)
  // *------------------------*

  connect(m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()));
  connect(m_Controls.m_pbNewSegmentationSession, SIGNAL(clicked()), this, SLOT(OnNewSegmentationSession()));
  connect(m_Controls.m_pbShowLabelTable, SIGNAL(toggled(bool)), this, SLOT(OnShowLabelTable(bool)));

  // *------------------------*
  // * Connect LabelSetWidget
  // *------------------------*

  connect(m_Controls.m_LabelSetWidget,
          SIGNAL(goToLabel(const mitk::Point3D &)),
          this,
          SLOT(OnGoToLabel(const mitk::Point3D &)));
  connect(m_Controls.m_LabelSetWidget, SIGNAL(resetView()), this, SLOT(OnResetView()));

  // *------------------------*
  // * DATA SLECTION WIDGET
  // *------------------------*
  m_IRenderWindowPart = this->GetRenderWindowPart();
  if (m_IRenderWindowPart)
  {
    QList<mitk::SliceNavigationController *> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
    //    m_Controls.m_LabelSetWidget->SetRenderWindowPart(this->m_IRenderWindowPart);
  }

  //  this->InitializeListeners();

  connect(m_Controls.m_btAddLayer, SIGNAL(clicked()), this, SLOT(OnAddLayer()));
  connect(m_Controls.m_btDeleteLayer, SIGNAL(clicked()), this, SLOT(OnDeleteLayer()));
  connect(m_Controls.m_btPreviousLayer, SIGNAL(clicked()), this, SLOT(OnPreviousLayer()));
  connect(m_Controls.m_btNextLayer, SIGNAL(clicked()), this, SLOT(OnNextLayer()));
  connect(m_Controls.m_btLockExterior, SIGNAL(toggled(bool)), this, SLOT(OnLockExteriorToggled(bool)));
  connect(m_Controls.m_cbActiveLayer, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChangeLayer(int)));

  m_Controls.m_btAddLayer->setEnabled(false);
  m_Controls.m_btDeleteLayer->setEnabled(false);
  m_Controls.m_btNextLayer->setEnabled(false);
  m_Controls.m_btPreviousLayer->setEnabled(false);
  m_Controls.m_cbActiveLayer->setEnabled(false);

  m_Controls.m_pbNewLabel->setEnabled(false);
  m_Controls.m_btLockExterior->setEnabled(false);
  m_Controls.m_pbShowLabelTable->setEnabled(false);

  // Make sure the GUI notices if appropriate data is already present on creation
  this->OnReferenceSelectionChanged(m_Controls.m_cbReferenceNodeSelector->GetSelectedNode());
  this->OnSegmentationSelectionChanged(m_Controls.m_cbWorkingNodeSelector->GetSelectedNode());
}

void QmitkMultiLabelSegmentationView::Activated()
{
  m_ToolManager->SetReferenceData(m_Controls.m_cbReferenceNodeSelector->GetSelectedNode());
  m_ToolManager->SetWorkingData(m_Controls.m_cbWorkingNodeSelector->GetSelectedNode());
}

void QmitkMultiLabelSegmentationView::Deactivated()
{
  // Not yet implemented
}

void QmitkMultiLabelSegmentationView::Visible()
{
  // Not yet implemented
}

void QmitkMultiLabelSegmentationView::Hidden()
{
  // Not yet implemented
}

void QmitkMultiLabelSegmentationView::InitializeListeners()
{
  if (m_Interactor.IsNull())
  {
    us::Module *module = us::GetModuleContext()->GetModule();
    std::vector<us::ModuleResource> resources = module->FindResources("/", "*", true);
    for (std::vector<us::ModuleResource>::iterator iter = resources.begin(); iter != resources.end(); ++iter)
    {
      MITK_INFO << iter->GetResourcePath();
    }

    m_Interactor = mitk::SegmentationInteractor::New();
    if (!m_Interactor->LoadStateMachine("SegmentationInteraction.xml", module))
    {
      MITK_WARN << "Error loading state machine";
    }

    if (!m_Interactor->SetEventConfig("ConfigSegmentation.xml", module))
    {
      MITK_WARN << "Error loading state machine configuration";
    }

    // Register as listener via micro services
    us::ServiceProperties props;
    props["name"] = std::string("SegmentationInteraction");
    m_ServiceRegistration =
      us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(m_Interactor.GetPointer(), props);
  }
}

void QmitkMultiLabelSegmentationView::SetFocus()
{
}

bool QmitkMultiLabelSegmentationView::CheckForSameGeometry(const mitk::Image *image1, const mitk::Image *image2) const
{
  bool isSameGeometry(true);

  if (image1 && image2)
  {
    mitk::BaseGeometry::Pointer geo1 = image1->GetGeometry();
    mitk::BaseGeometry::Pointer geo2 = image2->GetGeometry();

    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetOrigin(), geo2->GetOrigin());
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(0), geo2->GetExtent(0));
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(1), geo2->GetExtent(1));
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(2), geo2->GetExtent(2));
    isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetSpacing(), geo2->GetSpacing());
    isSameGeometry = isSameGeometry && mitk::MatrixEqualElementWise(geo1->GetIndexToWorldTransform()->GetMatrix(),
                                                                    geo2->GetIndexToWorldTransform()->GetMatrix());

    return isSameGeometry;
  }
  else
  {
    return false;
  }
}

void QmitkMultiLabelSegmentationView::RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart)
{
  if (m_IRenderWindowPart != renderWindowPart)
  {
    m_IRenderWindowPart = renderWindowPart;
    m_Parent->setEnabled(true);

    QList<mitk::SliceNavigationController *> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
  }
}

void QmitkMultiLabelSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart * /*renderWindowPart*/)
{
  m_ToolManager->ActivateTool(-1);
  m_IRenderWindowPart = 0;
  m_Parent->setEnabled(false);
}

int QmitkMultiLabelSegmentationView::GetSizeFlags(bool width)
{
  if (!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkMultiLabelSegmentationView::ComputePreferredSize(bool width,
                                                          int /*availableParallel*/,
                                                          int /*availablePerpendicular*/,
                                                          int preferredResult)
{
  if (width == false)
  {
    return 100;
  }
  else
  {
    return preferredResult;
  }
}

void QmitkMultiLabelSegmentationView::UpdateControls()
{
  mitk::DataNode *referenceNode = m_ToolManager->GetReferenceData(0);
  bool hasReferenceNode = referenceNode != NULL;

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  bool hasValidWorkingNode = workingNode != NULL;

  m_Controls.m_pbNewLabel->setEnabled(false);
  m_Controls.m_gbInterpolation->setEnabled(false);
  m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(false);
  m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(false);
  m_Controls.m_LabelSetWidget->setEnabled(false);
  m_Controls.m_btAddLayer->setEnabled(false);
  m_Controls.m_btDeleteLayer->setEnabled(false);
  m_Controls.m_cbActiveLayer->setEnabled(false);
  m_Controls.m_btPreviousLayer->setEnabled(false);
  m_Controls.m_btNextLayer->setEnabled(false);
  m_Controls.m_btLockExterior->setChecked(false);
  m_Controls.m_btLockExterior->setEnabled(false);
  m_Controls.m_pbShowLabelTable->setChecked(false);
  m_Controls.m_pbShowLabelTable->setEnabled(false);

  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(
    QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

  if (hasValidWorkingNode)
  {
    // TODO adapt tool manager so that this check is done there, e.g. convenience function
    mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
    hasValidWorkingNode = workingImage != nullptr;
    if (hasValidWorkingNode)
    {
      m_Controls.m_pbNewLabel->setEnabled(true);
      m_Controls.m_btLockExterior->setEnabled(true);
      m_Controls.m_pbShowLabelTable->setEnabled(true);
      m_Controls.m_gbInterpolation->setEnabled(true);
      m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(true);
      m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(true);
      m_Controls.m_LabelSetWidget->setEnabled(true);
      m_Controls.m_btAddLayer->setEnabled(true);

      int activeLayer = workingImage->GetActiveLayer();
      int numberOfLayers = workingImage->GetNumberOfLayers();

      m_Controls.m_cbActiveLayer->blockSignals(true);
      m_Controls.m_cbActiveLayer->clear();
      for (unsigned int lidx = 0; lidx < workingImage->GetNumberOfLayers(); ++lidx)
        m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
      m_Controls.m_cbActiveLayer->setCurrentIndex(activeLayer);
      m_Controls.m_cbActiveLayer->blockSignals(false);

      m_Controls.m_cbActiveLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btDeleteLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btPreviousLayer->setEnabled(activeLayer > 0);
      m_Controls.m_btNextLayer->setEnabled(activeLayer != numberOfLayers - 1);

      m_Controls.m_btLockExterior->setChecked(workingImage->GetLabel(0, activeLayer)->GetLocked());
      m_Controls.m_pbShowLabelTable->setChecked(workingImage->GetNumberOfLabels() > 1 /*1st is exterior*/);

      // MLI TODO
      // m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithWorkingDataVisible);
    }
  }

  if (hasValidWorkingNode && hasReferenceNode)
  {
    int layer = -1;
    referenceNode->GetIntProperty("layer", layer);
    workingNode->SetIntProperty("layer", layer + 1);
  }

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::OnNewSegmentationSession()
{
  mitk::DataNode *referenceNode = m_Controls.m_cbReferenceNodeSelector->GetSelectedNode();

  if (!referenceNode)
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  m_ToolManager->ActivateTool(-1);

  mitk::Image *referenceImage = dynamic_cast<mitk::Image *>(referenceNode->GetData());
  assert(referenceImage);

  QString newName = QString::fromStdString(referenceNode->GetName());
  newName.append("-labels");

  bool ok = false;
  newName = QInputDialog::getText(m_Parent, "New Segmentation Session", "New name:", QLineEdit::Normal, newName, &ok);

  if (!ok)
    return;

  this->WaitCursorOn();

  mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();

  try
  {
    workingImage->Initialize(referenceImage);
  }
  catch (mitk::Exception &e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(m_Parent, "New Segmentation Session", "Could not create a new segmentation session.\n");
    return;
  }

  this->WaitCursorOff();

  mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
  workingNode->SetData(workingImage);
  workingNode->SetName(newName.toStdString());
  workingImage->GetExteriorLabel()->SetProperty("name.parent",
                                                mitk::StringProperty::New(referenceNode->GetName().c_str()));
  workingImage->GetExteriorLabel()->SetProperty("name.image", mitk::StringProperty::New(newName.toStdString().c_str()));
  this->AddDICOMSegmentationProperties(workingImage, referenceImage);

  if (!this->GetDataStorage()->Exists(workingNode))
    this->GetDataStorage()->Add(workingNode, referenceNode);

  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();

  OnNewLabel();
}

void QmitkMultiLabelSegmentationView::OnNewLabel()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  if (!workingImage)
  {
    QMessageBox::information(
      m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  QmitkNewSegmentationDialog *dialog = new QmitkNewSegmentationDialog(m_Parent);
  dialog->SetSuggestionList(mitk::OrganNamesHandling::GetDefaultOrganColorString());
  dialog->setWindowTitle("New Label");

  int dialogReturnValue = dialog->exec();

  if (dialogReturnValue == QDialog::Rejected)
    return;

  QString segName = dialog->GetSegmentationName();
  if (segName.isEmpty())
    segName = "Unnamed";
  workingImage->GetActiveLabelSet()->AddLabel(segName.toStdString(), dialog->GetColor());
  this->AddDICOMSegmentProperties(workingImage->GetActiveLabel(workingImage->GetActiveLayer()));

  UpdateControls();

  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnShowLabelTable(bool value)
{
  if (value)
    m_Controls.m_LabelSetWidget->show();
  else
    m_Controls.m_LabelSetWidget->hide();
}

void QmitkMultiLabelSegmentationView::OnNextLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  OnChangeLayer(workingImage->GetActiveLayer() + 1);
}

void QmitkMultiLabelSegmentationView::OnPreviousLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  OnChangeLayer(workingImage->GetActiveLayer() - 1);
}

void QmitkMultiLabelSegmentationView::OnChangeLayer(int layer)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  this->WaitCursorOn();
  workingImage->SetActiveLayer(layer);
  this->WaitCursorOff();

  UpdateControls();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnDeleteLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  if (workingImage->GetNumberOfLayers() < 2)
    return;

  QString question = "Do you really want to delete the current layer?";

  QMessageBox::StandardButton answerButton = QMessageBox::question(
    m_Controls.m_LabelSetWidget, "Delete layer", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes)
    return;

  try
  {
    this->WaitCursorOn();
    workingImage->RemoveLayer();
    this->WaitCursorOff();
  }
  catch (mitk::Exception &e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(m_Controls.m_LabelSetWidget,
                             "Delete Layer",
                             "Could not delete the currently active layer. See error log for details.\n");
    return;
  }

  UpdateControls();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnAddLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  QString question = "Do you really want to add a layer to the current segmentation session?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    m_Controls.m_LabelSetWidget, "Add layer", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes)
    return;

  int newLabelSetId = -1;
  try
  {
    WaitCursorOn();
    newLabelSetId = workingImage->AddLayer();
    WaitCursorOff();
  }
  catch (mitk::Exception &e)
  {
    WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      m_Controls.m_LabelSetWidget, "Add Layer", "Could not add a new layer. See error log for details.\n");
    return;
  }

  // Update controls and label set list for direct response
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
  OnNewLabel();
  UpdateControls();
}

void QmitkMultiLabelSegmentationView::OnDeactivateActiveTool()
{
  m_ToolManager->ActivateTool(-1);
}

void QmitkMultiLabelSegmentationView::OnLockExteriorToggled(bool checked)
{
  mitk::DataNode *workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
  assert(workingImage);

  workingImage->GetLabel(0)->SetLocked(checked);
}

void QmitkMultiLabelSegmentationView::NodeAdded(const mitk::DataNode *)
{
  /*
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject) return;

  if (m_ReferenceNode.IsNotNull() && dynamic_cast<mitk::LabelSetImage*>(node->GetData()))
  {
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

  if (workingImage->GetNumberOfLabels() > 2)
  m_Controls.m_LabelSetWidget->show();
  else
  m_Controls.m_LabelSetWidget->hide();
  }
  */
}

void QmitkMultiLabelSegmentationView::NodeRemoved(const mitk::DataNode *node)
{
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject)
    return;

  if (m_ReferenceNode.IsNotNull() && dynamic_cast<mitk::LabelSetImage *>(node->GetData()))
  {
    // remove all possible contour markers of the segmentation
    mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = this->GetDataStorage()->GetDerivations(
      node, mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

    ctkPluginContext *context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService *service = context->getService<mitk::PlanePositionManagerService>(ppmRef);

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End();
         ++it)
    {
      std::string nodeName = node->GetName();
      unsigned int t = nodeName.find_last_of(" ");
      unsigned int id = atof(nodeName.substr(t + 1).c_str()) - 1;

      service->RemovePlanePosition(id);

      this->GetDataStorage()->Remove(it->Value());
    }

    context->ungetService(ppmRef);
    service = NULL;
  }
}

void QmitkMultiLabelSegmentationView::OnInterpolationSelectionChanged(int index)
{
  if (index == 1)
  {
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(
      false); // OnToggleWidgetActivation(false);
    m_Controls.m_swInterpolation->setCurrentIndex(0);
    m_Controls.m_swInterpolation->show();
  }
  else if (index == 2)
  {
    m_Controls.m_SliceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_swInterpolation->setCurrentIndex(1);
    m_Controls.m_swInterpolation->show();
  }
  else
  {
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_SliceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_swInterpolation->setCurrentIndex(2);
    m_Controls.m_swInterpolation->hide();
  }
}

void QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged(const mitk::DataNode *node)
{
  m_ToolManager->ActivateTool(-1);

  m_ReferenceNode = const_cast<mitk::DataNode *>(node);

  m_ToolManager->SetReferenceData(m_ReferenceNode);

  // check match of segmentation and reference image geometries
  if (node && m_WorkingNode.IsNotNull())
  {
    mitk::Image *workingImage = dynamic_cast<mitk::Image *>(m_WorkingNode->GetData());
    assert(workingImage);

    mitk::Image *refImage = dynamic_cast<mitk::Image *>(node->GetData());
    assert(refImage);

    if (!this->CheckForSameGeometry(refImage, workingImage))
      return;
  }

  this->UpdateControls();
  // m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnEstablishLabelSetConnection()
{
  MITK_INFO << "Connection Established";
  if (m_WorkingNode.IsNull())
  {
    return;
  }
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(m_WorkingNode->GetData());
  assert(workingImage);

  workingImage->GetActiveLabelSet()->AddLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->RemoveLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ModifyLabelEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->AllLabelsModifiedEvent += mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ActiveLabelEvent +=
    mitk::MessageDelegate1<QmitkLabelSetWidget, mitk::Label::PixelType>(m_Controls.m_LabelSetWidget,
                                                                        &QmitkLabelSetWidget::SelectLabelByPixelValue);
  workingImage->BeforeChangeLayerEvent += mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
    this, &QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection);
}

void QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection()
{
  MITK_INFO << "Connection Lost";
  if (m_WorkingNode.IsNull())
  {
    return;
  }
  mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(m_WorkingNode->GetData());
  assert(workingImage);

  // Reset LabelSetWidget Events
  workingImage->GetActiveLabelSet()->AddLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->RemoveLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::ResetAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ModifyLabelEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->AllLabelsModifiedEvent -= mitk::MessageDelegate<QmitkLabelSetWidget>(
    m_Controls.m_LabelSetWidget, &QmitkLabelSetWidget::UpdateAllTableWidgetItems);
  workingImage->GetActiveLabelSet()->ActiveLabelEvent -=
    mitk::MessageDelegate1<QmitkLabelSetWidget, mitk::Label::PixelType>(m_Controls.m_LabelSetWidget,
                                                                        &QmitkLabelSetWidget::SelectLabelByPixelValue);
  workingImage->BeforeChangeLayerEvent -= mitk::MessageDelegate<QmitkMultiLabelSegmentationView>(
    this, &QmitkMultiLabelSegmentationView::OnLooseLabelSetConnection);
}

void QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged(const mitk::DataNode *node)
{
  m_ToolManager->ActivateTool(-1);

  if (m_WorkingNode.IsNotNull())
  {
    mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(m_WorkingNode->GetData());
    assert(workingImage);

    // Loose LabelSetConnections
    OnLooseLabelSetConnection();
  }

  m_WorkingNode = const_cast<mitk::DataNode *>(node);

  if (m_WorkingNode.IsNotNull())
  {
    mitk::LabelSetImage *workingImage = dynamic_cast<mitk::LabelSetImage *>(m_WorkingNode->GetData());
    assert(workingImage);

    // Establish LabelSetConnection
    OnEstablishLabelSetConnection();
  }

  m_ToolManager->SetWorkingData(m_WorkingNode);

  // check match of segmentation and reference image geometries
  if (node && m_ReferenceNode.IsNotNull())
  {
    mitk::Image *refImage = dynamic_cast<mitk::Image *>(m_ReferenceNode->GetData());
    assert(refImage);

    mitk::Image *workingImage = dynamic_cast<mitk::Image *>(node->GetData());
    assert(workingImage);

    if (!this->CheckForSameGeometry(refImage, workingImage))
      return;
  }

  if (m_WorkingNode.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer segNodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
    for (mitk::DataStorage::SetOfObjects::const_iterator iter = segNodes->begin(); iter != segNodes->end(); ++iter)
    {
      mitk::DataNode *_segNode = *iter;
      _segNode->SetVisibility(false);
    }

    m_WorkingNode->SetVisibility(true);
  }

  this->UpdateControls();

  if (m_WorkingNode.IsNotNull())
  {
    m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
  }
}

void QmitkMultiLabelSegmentationView::OnManualTool2DSelected(int id)
{
  this->ResetMouseCursor();
  mitk::StatusBar::GetInstance()->DisplayText("");

  if (id >= 0)
  {
    std::string text = "Active Tool: \"";
    text += m_ToolManager->GetToolById(id)->GetName();
    text += "\"";
    mitk::StatusBar::GetInstance()->DisplayText(text.c_str());

    us::ModuleResource resource = m_ToolManager->GetToolById(id)->GetCursorIconResource();
    if (resource.IsValid())
      this->SetMouseCursor(resource, 0, 0);
  }
}

void QmitkMultiLabelSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences *prefs)
{
  if (m_Parent && m_WorkingNode.IsNotNull())
  {
    mitk::BoolProperty::Pointer drawOutline = mitk::BoolProperty::New(prefs->GetBool("draw outline", true));
    mitk::BoolProperty::Pointer volumeRendering = mitk::BoolProperty::New(prefs->GetBool("volume rendering", false));
    mitk::LabelSetImage *labelSetImage;
    mitk::DataNode *segmentation;

    // iterate all segmentations (binary (single label) and LabelSetImages)
    mitk::NodePredicateProperty::Pointer isBinaryPredicate =
      mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateOr::Pointer allSegmentationsPredicate =
      mitk::NodePredicateOr::New(isBinaryPredicate, m_SegmentationPredicate);
    mitk::DataStorage::SetOfObjects::ConstPointer allSegmentations =
      GetDataStorage()->GetSubset(allSegmentationsPredicate);

    for (mitk::DataStorage::SetOfObjects::const_iterator it = allSegmentations->begin(); it != allSegmentations->end();
         ++it)
    {
      segmentation = *it;
      labelSetImage = dynamic_cast<mitk::LabelSetImage *>(segmentation->GetData());
      if (nullptr != labelSetImage)
      {
        // segmentation node is a multi label segmentation
        segmentation->SetProperty("labelset.contour.active", drawOutline);
        segmentation->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
        segmentation->SetProperty("volumerendering", volumeRendering);
        // force render window update to show outline
        segmentation->GetData()->Modified();
      }
      else
      {
        // node is actually a 'single label' segmentation,
        // but its outline property can be set in the 'multi label' segmentation preference page as well
        bool isBinary = false;
        segmentation->GetBoolProperty("binary", isBinary);
        if (isBinary)
        {
          segmentation->SetProperty("outline binary", drawOutline);
          segmentation->SetProperty("outline width", mitk::FloatProperty::New(2.0));
          segmentation->SetProperty("opacity", mitk::FloatProperty::New(drawOutline->GetValue() ? 1.0f : 0.3f));
          segmentation->SetProperty("volumerendering", volumeRendering);
          // force render window update to show outline
          segmentation->GetData()->Modified();
        }
      }
    }
  }
}

void QmitkMultiLabelSegmentationView::ResetMouseCursor()
{
  if (m_MouseCursorSet)
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}

void QmitkMultiLabelSegmentationView::SetMouseCursor(const us::ModuleResource resource, int hotspotX, int hotspotY)
{
  // Remove previously set mouse cursor
  if (m_MouseCursorSet)
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
  }

  us::ModuleResourceStream cursor(resource, std::ios::binary);
  mitk::ApplicationCursor::GetInstance()->PushCursor(cursor, hotspotX, hotspotY);
  m_MouseCursorSet = true;
}

void QmitkMultiLabelSegmentationView::OnGoToLabel(const mitk::Point3D &pos)
{
  if (m_IRenderWindowPart)
    m_IRenderWindowPart->SetSelectedPosition(pos);
}

void QmitkMultiLabelSegmentationView::OnResetView()
{
  if (m_IRenderWindowPart)
    m_IRenderWindowPart->ForceImmediateUpdate();
}

QString QmitkMultiLabelSegmentationView::GetLastFileOpenPath()
{
  return this->GetPreferences()->Get("LastFileOpenPath", "");
}

void QmitkMultiLabelSegmentationView::SetLastFileOpenPath(const QString &path)
{
  this->GetPreferences()->Put("LastFileOpenPath", path);
  this->GetPreferences()->Flush();
}

void QmitkMultiLabelSegmentationView::AddDICOMSegmentationProperties(mitk::LabelSetImage *image, mitk::Image *reference)
{
  // Add DICOM Tag (0008, 0060) Modality "SEG"
  image->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(), mitk::StringProperty::New("SEG"));
  // Add DICOM Tag (0008,103E) Series Description
  image->SetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x103E).c_str(),
                     mitk::StringProperty::New("MITK Segmentation"));

  // Check if original image is a DICOM image; if so, store relevant DICOM Tags into the PropertyList of new
  // segmentation image
  bool parentIsDICOM = false;

  for (const auto &element : *(reference->GetPropertyList()->GetMap()))
  {
    if (element.first.find("DICOM") == 0)
    {
      parentIsDICOM = true;
      break;
    }
  }

  if (!parentIsDICOM)
    return;

  //====== Patient information ======

  // Add DICOM Tag (0010,0010) patient's name; default "No Name"
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0010, 0x0010), "NO NAME");
  // Add DICOM Tag (0010,0020) patient id; default "No Name"
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0010, 0x0020), "NO NAME");
  // Add DICOM Tag (0010,0030) patient's birth date; no default
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0010, 0x0030));
  // Add DICOM Tag (0010,0040) patient's sex; default "U" (Unknown)
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0010, 0x0040), "U");

  //====== General study ======

  // Add DICOM Tag (0020,000D) Study Instance UID; no default --> MANDATORY!
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0020, 0x000D));
  // Add DICOM Tag (0080,0020) Study Date; no default (think about "today")
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0080, 0x0020));
  // Add DICOM Tag (0008,0050) Accession Number; no default
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0008, 0x0050));
  // Add DICOM Tag (0008,1030) Study Description; no default
  this->SetReferenceDICOMProperty(reference, image, mitk::DICOMTag(0x0008, 0x1030));

  //====== Reference DICOM data ======

  // Add reference file paths to referenced DICOM data
  mitk::BaseProperty::Pointer dcmFilesProp = reference->GetProperty("files");
  if (dcmFilesProp.IsNotNull())
    image->SetProperty("referenceFiles", dcmFilesProp);
}

void QmitkMultiLabelSegmentationView::AddDICOMSegmentProperties(mitk::Label *label)
{
  mitk::AnatomicalStructureColorPresets::Category category;
  mitk::AnatomicalStructureColorPresets::Type type;
  mitk::AnatomicalStructureColorPresets *anatomicalStructureColorPresets = mitk::AnatomicalStructureColorPresets::New();
  anatomicalStructureColorPresets->LoadPreset();

  for (const auto &preset : anatomicalStructureColorPresets->GetCategoryPresets())
  {
    auto presetOrganName = preset.first;
    if (label->GetName().compare(presetOrganName) == 0)
    {
      category = preset.second;
      break;
    }
  }

  for (const auto &preset : anatomicalStructureColorPresets->GetTypePresets())
  {
    auto presetOrganName = preset.first;
    if (label->GetName().compare(presetOrganName) == 0)
    {
      type = preset.second;
      break;
    }
  }

  // Add Segment Sequence tags (0062, 0002)
  mitk::DICOMTagPath segmentSequencePath;
  segmentSequencePath.AddElement(0x0062, 0x0002);

  // Segment Number:Identification number of the segment.The value of Segment Number(0062, 0004) shall be unique within
  // the Segmentation instance in which it is created
  mitk::DICOMTagPath segmentNumberPath;
  segmentNumberPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0004);
  label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentNumberPath).c_str(),
                     mitk::StringProperty::New(std::to_string(label->GetValue())));

  // Segment Label: User-defined label identifying this segment.
  mitk::DICOMTagPath segmentLabelPath;
  segmentLabelPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0005);
  label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentLabelPath).c_str(),
                     mitk::StringProperty::New(label->GetName()));

  // Segment Algorithm Type: Type of algorithm used to generate the segment. AUTOMATIC SEMIAUTOMATIC MANUAL
  mitk::DICOMTagPath segmentAlgorithmTypePath;
  segmentAlgorithmTypePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0008);
  label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentAlgorithmTypePath).c_str(),
                     mitk::StringProperty::New("SEMIAUTOMATIC"));
  //------------------------------------------------------------
  // Add Segmented Property Category Code Sequence tags (0062, 0003): Sequence defining the general category of this
  // segment.
  mitk::DICOMTagPath segmentSegmentedPropertyCategorySequencePath;
  segmentSegmentedPropertyCategorySequencePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003);
  // (0008,0100) Code Value
  mitk::DICOMTagPath segmentCategoryCodeValuePath;
  segmentCategoryCodeValuePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003).AddElement(0x008, 0x0100);
  if (!category.codeValue.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentCategoryCodeValuePath).c_str(),
                       mitk::StringProperty::New(category.codeValue));

  // (0008,0102) Coding Scheme Designator
  mitk::DICOMTagPath segmentCategoryCodeSchemePath;
  segmentCategoryCodeSchemePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003).AddElement(0x008, 0x0102);
  if (!category.codeScheme.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentCategoryCodeSchemePath).c_str(),
                       mitk::StringProperty::New(category.codeScheme));

  // (0008,0104) Code Meaning
  mitk::DICOMTagPath segmentCategoryCodeMeaningPath;
  segmentCategoryCodeMeaningPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x0003).AddElement(0x008, 0x0104);
  if (!category.codeName.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentCategoryCodeMeaningPath).c_str(),
                       mitk::StringProperty::New(category.codeName));
  //------------------------------------------------------------
  // Add Segmented Property Type Code Sequence (0062, 000F): Sequence defining the specific property type of this
  // segment.
  mitk::DICOMTagPath segmentSegmentedPropertyTypeSequencePath;
  segmentSegmentedPropertyTypeSequencePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F);

  // (0008,0100) Code Value
  mitk::DICOMTagPath segmentTypeCodeValuePath;
  segmentTypeCodeValuePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F).AddElement(0x008, 0x0100);
  if (!type.codeValue.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentTypeCodeValuePath).c_str(),
                       mitk::StringProperty::New(type.codeValue));

  // (0008,0102) Coding Scheme Designator
  mitk::DICOMTagPath segmentTypeCodeSchemePath;
  segmentTypeCodeSchemePath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F).AddElement(0x008, 0x0102);
  if (!type.codeScheme.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentTypeCodeSchemePath).c_str(),
                       mitk::StringProperty::New(type.codeScheme));

  // (0008,0104) Code Meaning
  mitk::DICOMTagPath segmentTypeCodeMeaningPath;
  segmentTypeCodeMeaningPath.AddElement(0x0062, 0x0002).AddElement(0x0062, 0x000F).AddElement(0x008, 0x0104);
  if (!type.codeName.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentTypeCodeMeaningPath).c_str(),
                       mitk::StringProperty::New(type.codeName));
  //------------------------------------------------------------
  // Add Segmented Property Type Modifier Code Sequence (0062,0011): Sequence defining the modifier of the property type
  // of this segment.
  mitk::DICOMTagPath segmentSegmentedPropertyModifierSequencePath;
  segmentSegmentedPropertyModifierSequencePath.AddElement(0x0062, 0x0002)
    .AddElement(0x0062, 0x000F)
    .AddElement(0x0062, 0x0011);
  // (0008,0100) Code Value
  mitk::DICOMTagPath segmentModifierCodeValuePath;
  segmentModifierCodeValuePath.AddElement(0x0062, 0x0002)
    .AddElement(0x0062, 0x000F)
    .AddElement(0x0062, 0x0011)
    .AddElement(0x008, 0x0100);
  if (!type.modifier.codeValue.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentModifierCodeValuePath).c_str(),
                       mitk::StringProperty::New(type.modifier.codeValue));

  // (0008,0102) Coding Scheme Designator
  mitk::DICOMTagPath segmentModifierCodeSchemePath;
  segmentModifierCodeSchemePath.AddElement(0x0062, 0x0002)
    .AddElement(0x0062, 0x000F)
    .AddElement(0x0062, 0x0011)
    .AddElement(0x008, 0x0102);
  if (!type.modifier.codeScheme.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentModifierCodeSchemePath).c_str(),
                       mitk::StringProperty::New(type.modifier.codeScheme));

  // (0008,0104) Code Meaning
  mitk::DICOMTagPath segmentModifierCodeMeaningPath;
  segmentModifierCodeMeaningPath.AddElement(0x0062, 0x0002)
    .AddElement(0x0062, 0x000F)
    .AddElement(0x0062, 0x0011)
    .AddElement(0x008, 0x0104);
  if (!type.modifier.codeName.empty())
    label->SetProperty(mitk::DICOMTagPathToPropertyName(segmentModifierCodeMeaningPath).c_str(),
                       mitk::StringProperty::New(type.modifier.codeName));

  //============================TODO: Not here:-)
  mitk::IDICOMTagsOfInterest *toiService = nullptr;

  std::vector<us::ServiceReference<mitk::IDICOMTagsOfInterest>> toiRegisters =
    us::GetModuleContext()->GetServiceReferences<mitk::IDICOMTagsOfInterest>();
  if (!toiRegisters.empty())
  {
    if (toiRegisters.size() > 1)
      MITK_WARN << "Multiple DICOM tags of interest services found. Using just one.";
    toiService = us::GetModuleContext()->GetService<mitk::IDICOMTagsOfInterest>(toiRegisters.front());
  }

  if (toiService != nullptr)
  {
    toiService->AddTagOfInterest(segmentSequencePath);

    toiService->AddTagOfInterest(segmentNumberPath);
    toiService->AddTagOfInterest(segmentLabelPath);
    toiService->AddTagOfInterest(segmentAlgorithmTypePath);

    toiService->AddTagOfInterest(segmentSegmentedPropertyCategorySequencePath);
    toiService->AddTagOfInterest(segmentCategoryCodeValuePath);
    toiService->AddTagOfInterest(segmentCategoryCodeSchemePath);
    toiService->AddTagOfInterest(segmentCategoryCodeMeaningPath);

    toiService->AddTagOfInterest(segmentSegmentedPropertyTypeSequencePath);
    toiService->AddTagOfInterest(segmentTypeCodeValuePath);
    toiService->AddTagOfInterest(segmentTypeCodeSchemePath);
    toiService->AddTagOfInterest(segmentTypeCodeMeaningPath);

    toiService->AddTagOfInterest(segmentSegmentedPropertyModifierSequencePath);
    toiService->AddTagOfInterest(segmentModifierCodeValuePath);
    toiService->AddTagOfInterest(segmentModifierCodeSchemePath);
    toiService->AddTagOfInterest(segmentModifierCodeMeaningPath);
  }
}

void QmitkMultiLabelSegmentationView::SetReferenceDICOMProperty(mitk::Image *original,
                                                                mitk::Image *segmentation,
                                                                const mitk::DICOMTag &tag,
                                                                const std::string &defaultString)
{
  std::string tagString = mitk::GeneratePropertyNameForDICOMTag(tag.GetGroup(), tag.GetElement());

  // Get DICOM property from referenced image
  mitk::BaseProperty::Pointer originalProperty = original->GetProperty(tagString.c_str());

  // if property exists, copy the informtaion to the segmentation
  if (originalProperty.IsNotNull())
    segmentation->SetProperty(tagString.c_str(), originalProperty);
  else // use the default value, if there is one
  {
    if (!defaultString.empty())
      segmentation->SetProperty(tagString.c_str(), mitk::StringProperty::New(defaultString).GetPointer());
  }
}
