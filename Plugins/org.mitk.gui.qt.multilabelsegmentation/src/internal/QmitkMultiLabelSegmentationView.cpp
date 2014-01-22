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
#include "mitkSegmentationObjectFactory.h"
#include "mitkSegTool2D.h"
#include "mitkPlanePositionManager.h"
#include "mitkPluginActivator.h"
#include "mitkInteractionEventObserver.h"

//move to core file load/save
#include "mitkLabelSetImageReader.h"
#include "mitkLabelSetImageWriter.h"

// Qmitk
#include "QmitkMultiLabelSegmentationOrganNamesHandling.cpp"
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

#include <itksys/SystemTools.hxx>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView() :
m_Parent(NULL),
m_IRenderWindowPart(NULL),
m_ReferenceNode(NULL),
m_WorkingNode(NULL),
m_ToolManager(NULL),
m_MouseCursorSet(false)
{
  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateProperty::New("visible", mitk::BoolProperty::New(true)));

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
  m_ToolManager->ActivateTool(-1);
/*
  todo: check this
  m_Controls.m_SliceBasedInterpolatorWidget->EnableInterpolation(false);
  ctkPluginContext* context = mitk::PluginActivator::getContext();
  ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
  mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
  service->RemoveAllPlanePositions();
  context->ungetService(ppmRef);
*/
  m_ToolManager->SetReferenceData(NULL);
  m_ToolManager->SetWorkingData(NULL);

  m_ServiceRegistration.Unregister();
}

void QmitkMultiLabelSegmentationView::CreateQtPartControl(QWidget* parent)
{
  // setup the basic GUI of this view
  m_Parent = parent;
  m_Controls.setupUi(parent);

  m_Controls.m_cbReferenceNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbReferenceNodeSelector->SetPredicate(m_ReferencePredicate);
  m_Controls.m_cbReferenceNodeSelector->SetAutoSelectNewItems(true);

  m_Controls.m_cbWorkingNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbWorkingNodeSelector->SetPredicate(m_SegmentationPredicate);
  m_Controls.m_cbWorkingNodeSelector->SetAutoSelectNewItems(true);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);

  m_ToolManager->SetDataStorage( *(this->GetDataStorage()) );
  m_ToolManager->InitializeTools();

  //use the same ToolManager instance for our 3D Tools
  m_Controls.m_ManualToolSelectionBox3D->SetToolManager(*m_ToolManager);

  m_Controls.m_LabelSetWidget->SetDataStorage( *(this->GetDataStorage()) );
  m_Controls.m_LabelSetWidget->SetOrganColors(this->GetDefaultOrganColorString());
  m_Controls.m_LabelSetWidget->SetLastFileOpenPath(this->GetLastFileOpenPath());
  m_Controls.m_LabelSetWidget->hide();

  m_Controls.m_SurfaceBasedInterpolatorWidget->SetDataStorage( *(this->GetDataStorage()) );
  m_Controls.m_SliceBasedInterpolatorWidget->SetDataStorage( *(this->GetDataStorage()) );

  // all part of open source MITK
  m_Controls.m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox2D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer2D );
  //m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Correction Paint Wipe 'Live Wire'");
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Add Subtract Fill Erase 'Region Growing' FastMarching2D");
  m_Controls.m_ManualToolSelectionBox2D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );
  connect( m_Controls.m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)) );

  //setup 3D Tools
  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer3D );
  //specify tools to be added to 3D Tool area
//  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("FastMarching3D RegionGrowing Watershed");
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("Median Dilate Erode Open Close 'Fill Holes' 'Keep N Largest' 'Split' 'Region Selector' Threshold 'Two Thresholds' 'Auto Threshold' 'Multiple Otsu'");
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );

  connect( m_Controls.m_cbReferenceNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
       this, SLOT( OnReferenceSelectionChanged( const mitk::DataNode* ) ) );

  connect( m_Controls.m_cbWorkingNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
       this, SLOT( OnSegmentationSelectionChanged( const mitk::DataNode* ) ) );

  connect( m_Controls.m_pbNewLabel, SIGNAL(clicked()), this, SLOT( OnNewLabel()) );
  connect( m_Controls.m_pbNewSegmentationSession, SIGNAL(clicked()), this, SLOT( OnNewSegmentationSession()) );
  connect( m_Controls.m_pbShowLabelTable, SIGNAL(toggled(bool)), this, SLOT( OnShowLabelTable(bool)) );

  connect(m_Controls.m_LabelSetWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SLOT(OnGoToLabel(const mitk::Point3D&)) );

  connect( m_Controls.m_cbInterpolation, SIGNAL( activated (int) ), this, SLOT( OnInterpolationSelectionChanged(int) ) );

  m_Controls.m_cbInterpolation->setCurrentIndex(0);

  this->OnReferenceSelectionChanged( m_Controls.m_cbReferenceNodeSelector->GetSelectedNode() );

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

  this->InitializeListeners();
}

void QmitkMultiLabelSegmentationView::InitializeListeners()
{
  if (m_Interactor.IsNull())
  {
    /*
    us::ModuleContext* moduleContext = us::GetModuleContext();
    m_Interactor = mitk::SegmentationInteractor::New();
    m_Interactor->LoadStateMachine( "SegmentationInteraction.xml", moduleContext->GetModule());
    m_Interactor->SetEventConfig ( "ConfigSegmentation.xml", moduleContext->GetModule());
    us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>( m_Interactor.GetPointer(), us::ServiceProperties() );
    */
    us::Module* module = us::GetModuleContext()->GetModule();
    std::vector<us::ModuleResource> resources = module->FindResources("/", "*", true);
    MITK_INFO << "number of resources found: " << resources.size();
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
    m_ServiceRegistration = us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(
        m_Interactor.GetPointer(),props);
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
      mitk::Geometry3D* geo1 = image1->GetGeometry();
      mitk::Geometry3D* geo2 = image2->GetGeometry();

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
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
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
  if (!referenceNode)
  {
    m_Controls.m_pbNewSegmentationSession->setEnabled(false);
    m_Controls.m_LabelSetWidget->setEnabled(false);
    m_Controls.m_pbNewLabel->setEnabled(false);
    m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(false);
    m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(false);
    return;
  }

  m_Controls.m_pbNewSegmentationSession->setEnabled(true);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  if (!workingNode)
  {
    m_Controls.m_LabelSetWidget->setEnabled(false);
    m_Controls.m_pbNewLabel->setEnabled(false);
    m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(false);
    m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(false);
    return;
  }

  m_Controls.m_LabelSetWidget->setEnabled(true);
  m_Controls.m_pbNewLabel->setEnabled(true);
  m_Controls.m_SliceBasedInterpolatorWidget->setEnabled(true);
  m_Controls.m_SurfaceBasedInterpolatorWidget->setEnabled(true);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  m_Controls.m_pbShowLabelTable->setChecked(workingImage->GetNumberOfLabels() > 2);
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

  mitk::DataNode::Pointer workingNode = mitk::DataNode::New();
  workingNode->SetData(workingImage);
  workingNode->SetName(newName.toStdString());
  workingImage->SetName(newName.toStdString());
  mitk::Color color;
  color.SetRed(1.0);
  color.SetGreen(0.0);
  color.SetBlue(0.0);
  workingImage->AddLabel("no_name", color);

  this->WaitCursorOff();

  if (!this->GetDataStorage()->Exists(workingNode))
    this->GetDataStorage()->Add(workingNode, referenceNode);
}

void QmitkMultiLabelSegmentationView::OnNewLabel()
{
  m_ToolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  assert(workingImage);

  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( m_Parent );
  dialog->SetSuggestionList( this->GetDefaultOrganColorString() );
  dialog->setWindowTitle("New Label");

  int dialogReturnValue = dialog->exec();

  if ( dialogReturnValue == QDialog::Rejected ) return;

  workingImage->AddLabel(dialog->GetSegmentationName().toStdString(), dialog->GetColor());

  m_Controls.m_pbShowLabelTable->setChecked(workingImage->GetNumberOfLabels() > 2);
}

void QmitkMultiLabelSegmentationView::OnShowLabelTable(bool value)
{
  if (value)
    m_Controls.m_LabelSetWidget->show();
  else
    m_Controls.m_LabelSetWidget->hide();
}

void QmitkMultiLabelSegmentationView::NodeAdded(const mitk::DataNode* node)
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

void QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged( const mitk::DataNode* node )
{
  m_ToolManager->ActivateTool(-1);

  m_ReferenceNode = const_cast<mitk::DataNode*>(node);

  m_ToolManager->SetReferenceData(m_ReferenceNode);

  this->UpdateControls();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::OnInterpolationSelectionChanged(int index)
{
  if (index == 1)
  {
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);//OnToggleWidgetActivation(false);
    m_Controls.m_swInterpolation->setCurrentIndex(0);
  }
  else if (index == 2)
  {
    m_Controls.m_SliceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_swInterpolation->setCurrentIndex(1);
  }
  else
  {
    m_Controls.m_SurfaceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_SliceBasedInterpolatorWidget->m_Controls.m_btStart->setChecked(false);
    m_Controls.m_swInterpolation->setCurrentIndex(2);
  }
}

void QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged(const mitk::DataNode *node)
{
  if (m_ReferenceNode.IsNull() ) return;

  m_ToolManager->ActivateTool(-1);

  //check image geometry
  if (node)
  {
    mitk::Image* refImage = dynamic_cast<mitk::Image*>(m_ReferenceNode->GetData());
    assert(refImage);

    mitk::Image* workingImage = dynamic_cast<mitk::Image*>(node->GetData());
    assert(workingImage);

    if (!this->CheckForSameGeometry(refImage, workingImage))
      return;
  }

  m_WorkingNode = const_cast<mitk::DataNode*>(node);

  m_ToolManager->SetWorkingData(m_WorkingNode);

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

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
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

QString QmitkMultiLabelSegmentationView::GetLastFileOpenPath()
{
  return QString::fromStdString(this->GetPreferences()->Get("LastFileOpenPath", ""));
}

void QmitkMultiLabelSegmentationView::SetLastFileOpenPath(const QString& path)
{
  this->GetPreferences()->Put("LastFileOpenPath", path.toStdString());
  this->GetPreferences()->Flush();
}
