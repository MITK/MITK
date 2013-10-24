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

// Qmitk
#include "QmitkMultiLabelSegmentationOrganNamesHandling.cpp"
#include "QmitkRenderWindow.h"

// us
#include "usModuleResource.h"
#include "usModuleResourceStream.h"

// Qt
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView() :
m_Parent(NULL),
m_IRenderWindowPart(NULL),
m_ReferenceNode(NULL),
m_WorkingNode(NULL),
m_ToolManager(NULL),
m_MouseCursorSet(false)
{
//  RegisterSegmentationObjectFactory();

  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateProperty::New("visible", mitk::BoolProperty::New(true)));

  m_SurfacePredicate = mitk::NodePredicateAnd::New();
  m_SurfacePredicate->AddPredicate(mitk::NodePredicateDataType::New("Surface"));
  m_SurfacePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  mitk::TNodePredicateDataType<mitk::Image>::Pointer isImage = mitk::TNodePredicateDataType<mitk::Image>::New();
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isMask = mitk::NodePredicateAnd::New(isBinary, isImage);

  m_MaskPredicate = mitk::NodePredicateAnd::New();
  m_MaskPredicate->AddPredicate(isMask);
  m_MaskPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

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
  m_Controls.m_ManualToolSelectionBox2D->setEnabled(false);
  m_Controls.m_ManualToolSelectionBox3D->setEnabled( false );
  m_ToolManager->ActivateTool(-1);
/*
  m_Controls.m_SliceBasedInterpolator->EnableInterpolation(false);
  ctkPluginContext* context = mitk::PluginActivator::getContext();
  ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
  mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
  service->RemoveAllPlanePositions();
  context->ungetService(ppmRef);
*/
  m_ToolManager->SetReferenceData(NULL);
  m_ToolManager->SetWorkingData(NULL);
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
  m_Controls.m_LabelSetWidget->SetPreferences( this->GetPreferences() );

  m_Controls.m_cbSurfaceNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbSurfaceNodeSelector->SetPredicate( m_SurfacePredicate );

  m_Controls.m_cbMaskNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbMaskNodeSelector->SetPredicate( m_MaskPredicate );

  // all part of open source MITK
  m_Controls.m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox2D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer2D );
  //m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Add Subtract Correction Paint Wipe 'Region Growing' Fill Erase 'Live Wire' 'FastMarching2D'");
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Add Subtract Fill Erase 'Region Growing' 'FastMarching2D'");
  m_Controls.m_ManualToolSelectionBox2D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );
  connect( m_Controls.m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)) );

  //setup 3D Tools
  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer3D );
  //specify tools to be added to 3D Tool area
//  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("Threshold 'Two Thresholds' Otsu FastMarching3D RegionGrowing Watershed");
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("Threshold FastMarching3D MedianTool3D");
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(2);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingData );

  //Hide 3D selection box, show 2D selection box
  m_Controls.m_ManualToolSelectionBox3D->hide();
  m_Controls.m_ManualToolSelectionBox2D->show();

  connect( m_Controls.m_cbReferenceNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
       this, SLOT( OnReferenceSelectionChanged( const mitk::DataNode* ) ) );

  connect( m_Controls.m_cbWorkingNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
       this, SLOT( OnSegmentationSelectionChanged( const mitk::DataNode* ) ) );

  //connect( m_Controls.m_LabelSetWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SLOT(OnGoToLabel(const mitk::Point3D&)) );

  connect( m_Controls.tabWidgetSegmentationTools, SIGNAL(currentChanged(int)), this, SLOT(OnTabWidgetChanged(int)));

  connect(m_Controls.m_pbSurfaceStamp, SIGNAL(clicked()), this, SLOT(OnSurfaceStamp()));
  connect(m_Controls.m_pbMaskStamp, SIGNAL(clicked()), this, SLOT(OnMaskStamp()));

  this->OnReferenceSelectionChanged( m_Controls.m_cbReferenceNodeSelector->GetSelectedNode() );

  m_IRenderWindowPart = this->GetRenderWindowPart();
  if (m_IRenderWindowPart)
  {
    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(m_IRenderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SliceBasedInterpolator->Initialize(controllers, this->GetDataStorage());
    m_Controls.m_LabelSetWidget->SetRenderWindowPart(this->m_IRenderWindowPart);
  }
}

void QmitkMultiLabelSegmentationView::SetFocus ()
{

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
    m_Controls.m_SliceBasedInterpolator->Initialize(controllers, this->GetDataStorage());
    m_Controls.m_LabelSetWidget->SetRenderWindowPart(this->m_IRenderWindowPart);
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
    return 200;
  }
  else
  {
    return preferredResult;
  }
}

void QmitkMultiLabelSegmentationView::NodeAdded(const mitk::DataNode* node)
{
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject) return;

  if (m_ReferenceNode.IsNotNull() && dynamic_cast<mitk::LabelSetImage*>(node->GetData()))
  {
    if ( !this->GetDataStorage()->Exists(node) )
      this->GetDataStorage()->Add( const_cast<mitk::DataNode*> (node) );
  }
}

void QmitkMultiLabelSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject) return;

  mitk::DataStorage::SetOfObjects::ConstPointer segNodes = this->GetDataStorage()->GetDerivations(node);
  for(mitk::DataStorage::SetOfObjects::const_iterator iter = segNodes->begin(); iter != segNodes->end(); ++iter)
  {
    mitk::DataNode* _segNode = *iter;
    if (_segNode)
    {
      this->GetDataStorage()->Remove(_segNode);
    }
  }

  if (m_ReferenceNode.IsNotNull() && dynamic_cast<mitk::LabelSetImage*>(node->GetData()))
  {
    //First of all remove all possible contour markers of the segmentation
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

  if (!node)
  {
    m_ToolManager->SetReferenceData(NULL);
    m_Controls.m_LabelSetWidget->setEnabled(false);
    return;
  }

  if (m_ReferenceNode.IsNotNull())
  {
    mitk::DataStorage::SetOfObjects::ConstPointer segNodes = this->GetDataStorage()->GetDerivations(m_ReferenceNode);
    for(mitk::DataStorage::SetOfObjects::const_iterator iter = segNodes->begin(); iter != segNodes->end(); ++iter)
    {
      mitk::DataNode* _segNode = *iter;
      _segNode->SetVisibility(false);
    }
    m_ReferenceNode->SetVisibility(false);
  }

  m_ReferenceNode = const_cast<mitk::DataNode*>(node);

  m_ToolManager->SetReferenceData(m_ReferenceNode);
  m_Controls.m_LabelSetWidget->setEnabled(true);

  m_ReferenceNode->SetVisibility(true);

  mitk::DataStorage::SetOfObjects::ConstPointer segNodes = this->GetDataStorage()->GetDerivations(m_ReferenceNode);
  for(mitk::DataStorage::SetOfObjects::const_iterator iter = segNodes->begin(); iter != segNodes->end(); ++iter)
  {
    mitk::DataNode* _segNode = *iter;
    if (_segNode)
    {
      _segNode->SetVisibility(true);
    }
  }

  m_Controls.m_cbWorkingNodeSelector->Reset();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::OnSegmentationSelectionChanged(const mitk::DataNode *node)
{
  if (!node)
  {
    m_ToolManager->SetWorkingData(NULL);
    m_Controls.m_SliceBasedInterpolator->setEnabled(false);
    m_Controls.m_gbSurfaceStamp->setEnabled(false);
    m_Controls.m_gbMaskStamp->setEnabled(false);
    return;
  }
/*
  if (m_WorkingNode.IsNotNull())
  {
    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( m_WorkingNode->GetData() );
    assert(workingImage);
    workingImage->SetActiveLabel(0,true);
  }
*/
  m_WorkingNode = const_cast<mitk::DataNode*>(node);

  m_ToolManager->SetWorkingData(m_WorkingNode);

  int layer(0);
  m_ReferenceNode->GetIntProperty("layer", layer);
  int othersLayer = layer+1;

  mitk::DataStorage::SetOfObjects::ConstPointer otherNodes = this->GetDataStorage()->GetDerivations(m_ReferenceNode, m_SegmentationPredicate);
  for(mitk::DataStorage::SetOfObjects::const_iterator iter = otherNodes->begin(); iter != otherNodes->end(); ++iter)
  {
    mitk::DataNode* _otherNode = *iter;
    if (_otherNode != m_WorkingNode)
    {
      _otherNode->SetIntProperty("layer", othersLayer);
      othersLayer++;
    }
  }

  //finally, set the layer to the current working node
  m_WorkingNode->SetIntProperty("layer", othersLayer);

  // and enable GUI controls
  m_Controls.m_SliceBasedInterpolator->setEnabled(true);
  m_Controls.m_gbSurfaceStamp->setEnabled(true);
  m_Controls.m_gbMaskStamp->setEnabled(true);

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::OnTabWidgetChanged(int id)
{
  //always disable tools on tab changed
  m_ToolManager->ActivateTool(-1);

  if (id == 0)
  {
    //Hide 3D selection box, show 2D selection box
    m_Controls.m_ManualToolSelectionBox3D->hide();
    m_Controls.m_ManualToolSelectionBox2D->show();
  }
  else
  {
    m_Controls.m_ManualToolSelectionBox2D->hide();
    m_Controls.m_ManualToolSelectionBox3D->show();
  }
}

void QmitkMultiLabelSegmentationView::OnManualTool2DSelected(int id)
{
  if (id >= 0)
  {
    std::string text = "Active Tool: \"";
    text += m_ToolManager->GetToolById(id)->GetName();
    text += "\"";
    mitk::StatusBar::GetInstance()->DisplayText(text.c_str());

    us::ModuleResource resource = m_ToolManager->GetToolById(id)->GetCursorIconResource();
    this->SetMouseCursor(resource, 0, 0);
  }
  else
  {
    this->ResetMouseCursor();
    mitk::StatusBar::GetInstance()->DisplayText("");
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

void QmitkMultiLabelSegmentationView::OnMaskStamp()
{
  mitk::DataNode* maskNode = m_Controls.m_cbMaskNodeSelector->GetSelectedNode();

  if (!maskNode)
  {
    QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a mask before starting some action.");
    return;
  }

  m_ToolManager->ActivateTool(-1);

  mitk::Image* mask = dynamic_cast<mitk::Image*>(maskNode->GetData() );
  if (!mask)
  {
    QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a mask before starting some action.");
    return;
  }

  if (m_WorkingNode.IsNull())
  {
   QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a segmentation before starting some action.");
   return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( m_WorkingNode->GetData() );

  if (!workingImage)
  {
    QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  this->WaitCursorOn();
  try
  {
    workingImage->MaskStamp( mask, m_Controls.m_chkMaskStampOverwrite->isChecked() );
  }
  catch ( mitk::Exception & e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information( m_Parent, "Mask Stamp", "Could not stamp the selected mask.\n See error log for details.\n");
    return;
  }

  this->WaitCursorOff();

  maskNode->SetVisibility(false);

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}

void QmitkMultiLabelSegmentationView::OnSurfaceStamp()
{
  mitk::DataNode* surfaceNode = m_Controls.m_cbSurfaceNodeSelector->GetSelectedNode();

  if (!surfaceNode)
  {
    QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a surface before starting some action.");
    return;
  }

  m_ToolManager->ActivateTool(-1);

  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(surfaceNode->GetData() );
  if ( !surface )
  {
    QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a surface before starting some action.");
    return;
  }

  if (m_WorkingNode.IsNull())
  {
   QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a segmentation before starting some action.");
   return;
  }

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>( m_WorkingNode->GetData() );

  if (!workingImage)
  {
    QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  this->WaitCursorOn();

  try
  {
    workingImage->SurfaceStamp( surface, m_Controls.m_chkSurfaceStampOverwrite->isChecked() );
  }
  catch ( mitk::Exception & e )
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information( m_Parent, "Surface Stamp", "Could not stamp the selected surface.\n See error log for details.\n");
    return;
  }

  this->WaitCursorOff();

  this->RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_ALL);
}
