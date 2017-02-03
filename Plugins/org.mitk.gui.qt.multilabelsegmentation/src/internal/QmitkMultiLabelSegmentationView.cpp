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
#include <berryIWorkbenchPage.h>
#include <berryConstants.h>

// mitk
#include "mitkLabelSetImage.h"
#include "mitkStatusBar.h"
#include "mitkApplicationCursor.h"
#include "mitkToolManagerProvider.h"
//#include "mitkSegmentationObjectFactory.h"
#include "mitkSegTool2D.h"
#include "mitkPlanePositionManager.h"
#include "mitkPluginActivator.h"
#include "mitkInteractionEventObserver.h"

// Qmitk
#include "QmitkSegmentationOrganNamesHandling.cpp"
#include "QmitkRenderWindow.h"
#include "QmitkNewSegmentationDialog.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usModuleResourceStream.h>

// Qt
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QDateTime>

#include "tinyxml.h"

#include <itksys/SystemTools.hxx>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView() :
  m_Parent(NULL),
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
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
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
  //m_ToolManager->ActivateTool(-1);
  /*
  todo: check this
  m_Controls.m_SliceBasedInterpolatorWidget->EnableInterpolation(false);
  ctkPluginContext* context = mitk::PluginActivator::getContext();
  ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
  mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
  service->RemoveAllPlanePositions();
  context->ungetService(ppmRef);
*/
  //m_ToolManager->SetReferenceData(NULL);
  //m_ToolManager->SetWorkingData(NULL);

  //m_ServiceRegistration.Unregister();

  //Loose LabelSetConnections
  OnLooseLabelSetConnection();
}

void QmitkMultiLabelSegmentationView::CreateQtPartControl(QWidget* parent)
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

  connect( m_Controls.m_cbReferenceNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
           this, SLOT( OnReferenceSelectionChanged( const mitk::DataNode* ) ) );

  connect( m_Controls.m_cbWorkingNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
           this, SLOT( OnSegmentationSelectionChanged( const mitk::DataNode* ) ) );

  // *------------------------
  // * ToolManager
  // *------------------------

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);
  m_ToolManager->SetDataStorage( *(this->GetDataStorage()) );
  m_ToolManager->InitializeTools();
  //use the same ToolManager instance for our 3D Tools
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

  m_Controls.m_SurfaceBasedInterpolatorWidget->SetDataStorage( *(this->GetDataStorage()) );
  m_Controls.m_SliceBasedInterpolatorWidget->SetDataStorage( *(this->GetDataStorage()) );
  connect( m_Controls.m_cbInterpolation, SIGNAL( activated (int) ), this, SLOT( OnInterpolationSelectionChanged(int) ) );

  m_Controls.m_cbInterpolation->setCurrentIndex(0);
  m_Controls.m_swInterpolation->hide();

  // *------------------------
  // * ToolSelection 2D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox2D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer2D );
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Add Subtract Fill Erase Paint Wipe 'Region Growing' FastMarching2D Correction 'Live Wire'");// todo: "Correction 'Live Wire'"
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible );
  connect( m_Controls.m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)) );


  // *------------------------
  // * ToolSelection 3D
  // *------------------------

  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer3D );
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("Threshold 'Two Thresholds' 'Auto Threshold' 'Multiple Otsu'"); // todo add : FastMarching3D RegionGrowing Watershed
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible );

  // *------------------------*
  // * Connect PushButtons (pb)
  // *------------------------*

  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT( OnNewLabel()) );
  connect( m_Controls.m_pbNewSegmentationSession, SIGNAL(clicked()), this, SLOT( OnNewSegmentationSession()) );
  connect( m_Controls.m_pbShowLabelTable, SIGNAL(toggled(bool)), this, SLOT( OnShowLabelTable(bool)) );

  // *------------------------*
  // * Connect LabelSetWidget
  // *------------------------*

  connect(m_Controls.m_LabelSetWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SLOT(OnGoToLabel(const mitk::Point3D&)) );
  connect(m_Controls.m_LabelSetWidget, SIGNAL(resetView()), this, SLOT(OnResetView()) );


  // *------------------------*
  // * DATA SLECTION WIDGET
  // *------------------------*
  m_IRenderWindowPart = this->GetRenderWindowPart();
  if (m_IRenderWindowPart)
  {
    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
    //    m_Controls.m_LabelSetWidget->SetRenderWindowPart(this->m_IRenderWindowPart);
  }

//  this->InitializeListeners();

  connect( m_Controls.m_btAddLayer, SIGNAL(clicked()), this, SLOT( OnAddLayer()) );
  connect( m_Controls.m_btDeleteLayer, SIGNAL(clicked()), this, SLOT( OnDeleteLayer()) );
  connect( m_Controls.m_btPreviousLayer, SIGNAL(clicked()), this, SLOT( OnPreviousLayer()) );
  connect( m_Controls.m_btNextLayer, SIGNAL(clicked()), this, SLOT( OnNextLayer()) );
  connect( m_Controls.m_btLockExterior, SIGNAL(toggled(bool)), this, SLOT( OnLockExteriorToggled(bool)) );
  connect( m_Controls.m_cbActiveLayer, SIGNAL(currentIndexChanged(int)), this, SLOT( OnChangeLayer(int)) );

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
    us::Module* module = us::GetModuleContext()->GetModule();
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

    if (!m_Interactor->SetEventConfig ("ConfigSegmentation.xml", module))
    {
      MITK_WARN << "Error loading state machine configuration";
    }

    // Register as listener via micro services
    us::ServiceProperties props;
    props["name"] = std::string("SegmentationInteraction");
    m_ServiceRegistration = us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(m_Interactor.GetPointer(), props);
  }
}

void QmitkMultiLabelSegmentationView::SetFocus ()
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
    isSameGeometry = isSameGeometry && mitk::MatrixEqualElementWise(geo1->GetIndexToWorldTransform()->GetMatrix(), geo2->GetIndexToWorldTransform()->GetMatrix());

    return isSameGeometry;
  }
  else
  {
    return false;
  }
}

void QmitkMultiLabelSegmentationView::RenderWindowPartActivated(mitk::IRenderWindowPart* renderWindowPart)
{
  if (m_IRenderWindowPart != renderWindowPart)
  {
    m_IRenderWindowPart = renderWindowPart;
    m_Parent->setEnabled(true);

    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolatorWidget->SetSliceNavigationControllers(controllers);
  }
}

void QmitkMultiLabelSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  m_ToolManager->ActivateTool(-1);
  m_IRenderWindowPart = 0;
  m_Parent->setEnabled(false);
}

int QmitkMultiLabelSegmentationView::GetSizeFlags(bool width)
{
  if(!width)
  {
    return berry::Constants::MIN | berry::Constants::MAX | berry::Constants::FILL;
  }
  else
  {
    return 0;
  }
}

int QmitkMultiLabelSegmentationView::ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult)
{
  if(width==false)
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
  mitk::DataNode* referenceNode = m_ToolManager->GetReferenceData(0);
  bool hasReferenceNode = referenceNode != NULL;

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
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

  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);

  if(hasValidWorkingNode)
  {
    // TODO adapt tool manager so that this check is done there, e.g. convenience function
    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
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
      for (unsigned int lidx=0; lidx<workingImage->GetNumberOfLayers(); ++lidx)
        m_Controls.m_cbActiveLayer->addItem(QString::number(lidx));
      m_Controls.m_cbActiveLayer->setCurrentIndex(activeLayer);
      m_Controls.m_cbActiveLayer->blockSignals(false);

      m_Controls.m_cbActiveLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btDeleteLayer->setEnabled(numberOfLayers > 1);
      m_Controls.m_btPreviousLayer->setEnabled(activeLayer > 0);
      m_Controls.m_btNextLayer->setEnabled(activeLayer != numberOfLayers - 1);

      m_Controls.m_btLockExterior->setChecked(workingImage->GetLabel(0, activeLayer)->GetLocked());
      m_Controls.m_pbShowLabelTable->setChecked(workingImage->GetNumberOfLabels() > 1 /*1st is exterior*/);

      //MLI TODO
      //m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode(QmitkToolSelectionBox::EnabledWithWorkingDataVisible);
    }
  }

  if(hasValidWorkingNode && hasReferenceNode)
  {
    int layer = -1;
    referenceNode->GetIntProperty("layer", layer);
    workingNode->SetIntProperty("layer", layer + 1);
  }

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::OnNewSegmentationSession()
{
  mitk::DataNode* referenceNode = m_Controls.m_cbReferenceNodeSelector->GetSelectedNode();

  if (!referenceNode)
  {
    QMessageBox::information( m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  m_ToolManager->ActivateTool(-1);

  mitk::Image* referenceImage = dynamic_cast<mitk::Image*>( referenceNode->GetData() );
  assert(referenceImage);

  QString newName = QString::fromStdString(referenceNode->GetName());
  newName.append("-labels");

  bool ok = false;
  newName = QInputDialog::getText(m_Parent, "New Segmentation Session", "New name:", QLineEdit::Normal, newName, &ok);

  if(!ok) return;

  this->WaitCursorOn();

  mitk::LabelSetImage::Pointer workingImage = mitk::LabelSetImage::New();

  try
  {
    workingImage->Initialize(referenceImage);
  }
  catch ( mitk::Exception& e )
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
  workingImage->GetExteriorLabel()->SetProperty("name.parent",mitk::StringProperty::New(referenceNode->GetName().c_str()));
  workingImage->GetExteriorLabel()->SetProperty("name.image",mitk::StringProperty::New(newName.toStdString().c_str()));

  if (!this->GetDataStorage()->Exists(workingNode))
    this->GetDataStorage()->Add(workingNode, referenceNode);

  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();

  OnNewLabel();
}

void QmitkMultiLabelSegmentationView::OnNewLabel()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    QMessageBox::information(m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if (!workingImage)
  {
    QMessageBox::information(m_Parent, "New Segmentation Session", "Please load and select a patient image before starting some action.");
    return;
  }

  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( m_Parent );
  dialog->SetSuggestionList( mitk::OrganNamesHandling::GetDefaultOrganColorString() );
  dialog->setWindowTitle("New Label");

  int dialogReturnValue = dialog->exec();

  if ( dialogReturnValue == QDialog::Rejected ) return;

  QString segName = dialog->GetSegmentationName();
  if(segName.isEmpty()) segName = "Unnamed";
  workingImage->GetActiveLabelSet()->AddLabel(segName.toStdString(), dialog->GetColor());

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

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  OnChangeLayer(workingImage->GetActiveLayer() + 1 );
}

void QmitkMultiLabelSegmentationView::OnPreviousLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  OnChangeLayer(workingImage->GetActiveLayer() - 1 );
}


void QmitkMultiLabelSegmentationView::OnChangeLayer(int layer)
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
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

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  if (workingImage->GetNumberOfLayers() < 2)
    return;

  QString question = "Do you really want to delete the current layer?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( m_Controls.m_LabelSetWidget, "Delete layer",
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
    QMessageBox::information(m_Controls.m_LabelSetWidget, "Delete Layer", "Could not delete the currently active layer. See error log for details.\n");
    return;
  }

  UpdateControls();
  m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnAddLayer()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  QString question = "Do you really want to add a layer to the current segmentation session?";
  QMessageBox::StandardButton answerButton = QMessageBox::question( m_Controls.m_LabelSetWidget, "Add layer",
                                                                    question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton != QMessageBox::Yes) return;

  int newLabelSetId = -1;
  try
  {
    WaitCursorOn();
    newLabelSetId = workingImage->AddLayer();
    WaitCursorOff();
  }
  catch ( mitk::Exception& e )
  {
    WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(m_Controls.m_LabelSetWidget, "Add Layer", "Could not add a new layer. See error log for details.\n");
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
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  workingImage->GetLabel(0)->SetLocked(checked);
}

void QmitkMultiLabelSegmentationView::NodeAdded(const mitk::DataNode*)
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

void QmitkMultiLabelSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject) return;

  if (m_ReferenceNode.IsNotNull() && dynamic_cast<mitk::LabelSetImage*>(node->GetData()))
  {
    // remove all possible contour markers of the segmentation
    mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers =
        this->GetDataStorage()->GetDerivations(node, mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

    ctkPluginContext* context = mitk::PluginActivator::getContext();
    ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
    mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
    {
      std::string nodeName = node->GetName();
      unsigned int t = nodeName.find_last_of(" ");
      unsigned int id = atof(nodeName.substr(t+1).c_str())-1;

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
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);//OnToggleWidgetActivation(false);
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

void QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged( const mitk::DataNode* node )
{
  m_ToolManager->ActivateTool(-1);

  m_ReferenceNode = const_cast<mitk::DataNode*>(node);

  m_ToolManager->SetReferenceData(m_ReferenceNode);

  //check match of segmentation and reference image geometries
  if (node && m_WorkingNode.IsNotNull())
  {
    mitk::Image* workingImage = dynamic_cast<mitk::Image*>(m_WorkingNode->GetData());
    assert(workingImage);

    mitk::Image* refImage = dynamic_cast<mitk::Image*>(node->GetData());
    assert(refImage);

    if (!this->CheckForSameGeometry(refImage, workingImage))
      return;
  }

  this->UpdateControls();
  //m_Controls.m_LabelSetWidget->ResetAllTableWidgetItems();
}

void QmitkMultiLabelSegmentationView::OnEstablishLabelSetConnection()
{
  MITK_INFO << "Connection Established";
  if (m_WorkingNode.IsNull())
  {
    return;
  }
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
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
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
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

  if(m_WorkingNode.IsNotNull())
  {
    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
    assert(workingImage);

    //Loose LabelSetConnections
    OnLooseLabelSetConnection();
  }

  m_WorkingNode = const_cast<mitk::DataNode*>(node);

  if(m_WorkingNode.IsNotNull())
  {
    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
    assert(workingImage);

    //Establish LabelSetConnection
    OnEstablishLabelSetConnection();
  }

  m_ToolManager->SetWorkingData(m_WorkingNode);

  //check match of segmentation and reference image geometries
  if (node && m_ReferenceNode.IsNotNull())
  {
    mitk::Image* refImage = dynamic_cast<mitk::Image*>(m_ReferenceNode->GetData());
    assert(refImage);

    mitk::Image* workingImage = dynamic_cast<mitk::Image*>(node->GetData());
    assert(workingImage);

    if (!this->CheckForSameGeometry(refImage, workingImage))
      return;
  }

  if (m_WorkingNode.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer segNodes = this->GetDataStorage()->GetSubset(m_SegmentationPredicate);
    for(mitk::DataStorage::SetOfObjects::const_iterator iter = segNodes->begin(); iter != segNodes->end(); ++iter)
    {
      mitk::DataNode* _segNode = *iter;
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

void QmitkMultiLabelSegmentationView::OnPreferencesChanged(const berry::IBerryPreferences* prefs)
{
  if (m_Parent && m_WorkingNode.IsNotNull())
  {
    mitk::BoolProperty::Pointer drawOutline = mitk::BoolProperty::New(prefs->GetBool("draw outline", true));
    mitk::BoolProperty::Pointer volumeRendering = mitk::BoolProperty::New(prefs->GetBool("volume rendering", false));
    mitk::LabelSetImage* labelSetImage;
    mitk::DataNode* segmentation;

    // iterate all segmentations (binary (single label) and LabelSetImages)
    mitk::NodePredicateProperty::Pointer isBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    mitk::NodePredicateOr::Pointer allSegmentationsPredicate = mitk::NodePredicateOr::New(isBinaryPredicate, m_SegmentationPredicate);
    mitk::DataStorage::SetOfObjects::ConstPointer allSegmentations = GetDataStorage()->GetSubset(allSegmentationsPredicate);

    for (mitk::DataStorage::SetOfObjects::const_iterator it = allSegmentations->begin(); it != allSegmentations->end(); ++it)
    {
      segmentation = *it;
      labelSetImage = dynamic_cast<mitk::LabelSetImage*>(segmentation->GetData());
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
  if ( m_MouseCursorSet )
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}

void QmitkMultiLabelSegmentationView::SetMouseCursor( const us::ModuleResource resource, int hotspotX, int hotspotY )
{
  // Remove previously set mouse cursor
  if ( m_MouseCursorSet )
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
  }

  us::ModuleResourceStream cursor(resource, std::ios::binary);
  mitk::ApplicationCursor::GetInstance()->PushCursor( cursor, hotspotX, hotspotY );
  m_MouseCursorSet = true;
}

void QmitkMultiLabelSegmentationView::OnGoToLabel(const mitk::Point3D& pos)
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

void QmitkMultiLabelSegmentationView::SetLastFileOpenPath(const QString& path)
{
  this->GetPreferences()->Put("LastFileOpenPath", path);
  this->GetPreferences()->Flush();
}
