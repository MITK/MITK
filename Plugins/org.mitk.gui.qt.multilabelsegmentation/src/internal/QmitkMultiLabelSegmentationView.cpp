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
#include "mitkSurfaceToLabelFilter.h"

// Qmitk
#include "QmitkMultiLabelSegmentationOrganNamesHandling.cpp"
#include "QmitkRenderWindow.h"

// us
#include "mitkGetModuleContext.h"
#include "mitkModule.h"
#include "mitkModuleRegistry.h"
#include "mitkModuleResource.h"

// Qt
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

const std::string QmitkMultiLabelSegmentationView::VIEW_ID = "org.mitk.views.multilabelsegmentation";

QmitkMultiLabelSegmentationView::QmitkMultiLabelSegmentationView() :
m_Parent(NULL),
m_IRenderWindowPart(NULL),
m_DataSelectionChanged(false),
m_MouseCursorSet(false)
{
  RegisterSegmentationObjectFactory();

  m_SegmentationPredicate = mitk::NodePredicateAnd::New();
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateDataType::New("LabelSetImage"));
  m_SegmentationPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  m_SurfacePredicate = mitk::NodePredicateAnd::New();
  m_SurfacePredicate->AddPredicate(mitk::NodePredicateDataType::New("Surface"));
  m_SurfacePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  mitk::NodePredicateDataType::Pointer isImage = mitk::NodePredicateDataType::New("Image");
  mitk::NodePredicateProperty::Pointer isBinary = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
  mitk::NodePredicateAnd::Pointer isBinaryImage = mitk::NodePredicateAnd::New(isBinary, isImage);

  m_MaskPredicate = mitk::NodePredicateAnd::New();
  m_MaskPredicate->AddPredicate(isBinaryImage);
  m_MaskPredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));

  mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
  mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
  mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");

  mitk::NodePredicateOr::Pointer validImages = mitk::NodePredicateOr::New();
  validImages->AddPredicate( mitk::TNodePredicateDataType<mitk::Image>::New() );
  validImages->AddPredicate(isDwi);
  validImages->AddPredicate(isDti);
  validImages->AddPredicate(isQbi);

  m_ReferencePredicate = mitk::NodePredicateAnd::New();
  m_ReferencePredicate->AddPredicate(validImages);
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New( m_SegmentationPredicate));
  m_ReferencePredicate->AddPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object")));
}

QmitkMultiLabelSegmentationView::~QmitkMultiLabelSegmentationView()
{
}

void QmitkMultiLabelSegmentationView::CreateQtPartControl(QWidget* parent)
{
  // setup the basic GUI of this view
  m_Parent = parent;
  m_Controls.setupUi(parent);

  m_Controls.m_cbReferenceNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbReferenceNodeSelector->SetPredicate(m_ReferencePredicate);

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert ( toolManager );
  toolManager->SetDataStorage( *(this->GetDataStorage()) );
  toolManager->InitializeTools();

  //use the same ToolManager instance for our 3D Tools
  m_Controls.m_ManualToolSelectionBox3D->SetToolManager(*toolManager);

  m_Controls.m_LabelSetWidget->SetDataStorage( *(this->GetDataStorage()) );
  m_Controls.m_LabelSetWidget->SetPreferences( this->GetPreferences() );
  m_Controls.m_LabelSetWidget->SetPredicate( m_SegmentationPredicate );

  m_Controls.m_cbSurfaceNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbSurfaceNodeSelector->SetPredicate( m_SurfacePredicate );

  m_Controls.m_cbMaskNodeSelector->SetDataStorage(this->GetDataStorage());
  m_Controls.m_cbMaskNodeSelector->SetPredicate( m_MaskPredicate );

  // all part of open source MITK
  m_Controls.m_ManualToolSelectionBox2D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox2D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer2D );
  //m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Add Subtract Correction Paint Wipe 'Region Growing' Fill Erase 'Live Wire' 'FastMarching2D'");
  m_Controls.m_ManualToolSelectionBox2D->SetDisplayedToolGroups("Add Subtract Fill Erase 'Region Growing' 'FastMarching2D'");
  m_Controls.m_ManualToolSelectionBox2D->SetLayoutColumns(3);
  m_Controls.m_ManualToolSelectionBox2D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible );
  connect( m_Controls.m_ManualToolSelectionBox2D, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)) );

  //setup 3D Tools
  m_Controls.m_ManualToolSelectionBox3D->SetGenerateAccelerators(true);
  m_Controls.m_ManualToolSelectionBox3D->SetToolGUIArea( m_Controls.m_ManualToolGUIContainer3D );
  //specify tools to be added to 3D Tool area
//  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("Threshold 'Two Thresholds' Otsu FastMarching3D RegionGrowing Watershed");
  m_Controls.m_ManualToolSelectionBox3D->SetDisplayedToolGroups("Threshold FastMarching3D");
  m_Controls.m_ManualToolSelectionBox3D->SetLayoutColumns(3);
  m_Controls.m_ManualToolSelectionBox3D->SetEnabledMode( QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible );

  //Hide 3D selection box, show 2D selection box
  m_Controls.m_ManualToolSelectionBox3D->hide();
  m_Controls.m_ManualToolSelectionBox2D->show();

  // create signal/slot connections

  connect( m_Controls.m_cbReferenceNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
       this, SLOT( OnReferenceSelectionChanged( const mitk::DataNode* ) ) );

  connect( m_Controls.m_LabelSetWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SLOT(OnGoToLabel(const mitk::Point3D&)) );

  connect( m_Controls.tabWidgetSegmentationTools, SIGNAL(currentChanged(int)), this, SLOT(OnTabWidgetChanged(int)));

  connect(m_Controls.m_SlicesInterpolator, SIGNAL(SignalShowMarkerNodes(bool)), this, SLOT(OnShowMarkerNodes(bool)));
  //connect(m_Controls.m_SlicesInterpolator, SIGNAL(Signal3DInterpolationEnabled(bool)), this, SLOT(On3DInterpolationEnabled(bool)));

  connect(m_Controls.m_pbSurfaceStamp, SIGNAL(clicked()), this, SLOT(OnSurfaceStamp()));
  connect(m_Controls.m_pbMaskStamp, SIGNAL(clicked()), this, SLOT(OnMaskStamp()));

  m_Controls.m_LabelSetWidget->ReInit();
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

    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    m_Controls.m_SlicesInterpolator->SetDataStorage( this->GetDataStorage());
    QList<mitk::SliceNavigationController*> controllers;
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("axial")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("sagittal")->GetSliceNavigationController());
    controllers.push_back(renderWindowPart->GetQmitkRenderWindow("coronal")->GetSliceNavigationController());
    m_Controls.m_SlicesInterpolator->Initialize( toolManager, controllers );
  }
}

void QmitkMultiLabelSegmentationView::RenderWindowPartDeactivated(mitk::IRenderWindowPart* /*renderWindowPart*/)
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);
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

void QmitkMultiLabelSegmentationView::NodeRemoved(const mitk::DataNode* node)
{
  bool isHelperObject(false);
  node->GetBoolProperty("helper object", isHelperObject);
  if (isHelperObject) return;

  if (dynamic_cast<mitk::LabelSetImage*>(node->GetData()))
  {
    //First of all remove all possible contour markers of the segmentation
    mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers =
        this->GetDataStorage()->GetDerivations(node, mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true)));

    // gets the context of the "Mitk" (Core) module (always has id 1)
    // TODO Workaround until CTK plugincontext is available
    mitk::ModuleContext* context = mitk::ModuleRegistry::GetModule(1)->GetModuleContext();
    // Workaround end
    mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::PlanePositionManagerService>();

    mitk::PlanePositionManagerService* service = dynamic_cast<mitk::PlanePositionManagerService*>(context->GetService(serviceRef));

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
    {
      std::string nodeName = node->GetName();
      unsigned int t = nodeName.find_last_of(" ");
      unsigned int id = atof(nodeName.substr(t+1).c_str())-1;

      service->RemovePlanePosition(id);

      this->GetDataStorage()->Remove(it->Value());
    }

 //   mitk::SurfaceInterpolationController::GetInstance()->RemoveSegmentationFromContourList(lsImage);
  }
}

void QmitkMultiLabelSegmentationView::OnReferenceSelectionChanged( const mitk::DataNode* node )
{
  if( node != NULL )
  {
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->ActivateTool(-1);
    mitk::DataNode* refNode = const_cast<mitk::DataNode*>(node);
    refNode->SetVisibility(true);
    toolManager->SetReferenceData(refNode);

    mitk::DataStorage::SetOfObjects::ConstPointer others = this->GetDataStorage()->GetSubset(m_ReferencePredicate);
    for(mitk::DataStorage::SetOfObjects::const_iterator iter = others->begin(); iter != others->end(); ++iter)
    {
      mitk::DataNode* _other = *iter;
      if (_other != refNode)
          _other->SetVisibility(false);
    }
  }
}

void QmitkMultiLabelSegmentationView::OnShowMarkerNodes (bool state)
{
  mitk::SegTool2D::Pointer manualSegmentationTool;
  unsigned int numberOfExistingTools = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetTools().size();
  for(unsigned int i = 0; i < numberOfExistingTools; i++)
  {
    manualSegmentationTool = dynamic_cast<mitk::SegTool2D*>(mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetToolById(i));
    if (manualSegmentationTool)
      manualSegmentationTool->SetShowMarkerNodes( state );
  }
}

void QmitkMultiLabelSegmentationView::OnTabWidgetChanged(int id)
{
  //always disable tools on tab changed
  mitk::ToolManagerProvider::GetInstance()->GetToolManager()->ActivateTool(-1);

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

bool QmitkMultiLabelSegmentationView::CheckForSameGeometry(const mitk::DataNode *node1, const mitk::DataNode *node2) const
{
  bool isSameGeometry(true);

  mitk::Image* image1 = dynamic_cast<mitk::Image*>(node1->GetData());
  mitk::Image* image2 = dynamic_cast<mitk::Image*>(node2->GetData());
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

void QmitkMultiLabelSegmentationView::OnManualTool2DSelected(int id)
{
  if (id >= 0)
  {
    std::string text = "Active Tool: \"";
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    text += toolManager->GetToolById(id)->GetName();
    text += "\"";
    mitk::StatusBar::GetInstance()->DisplayText(text.c_str());

    mitk::ModuleResource resource = toolManager->GetToolById(id)->GetCursorIconResource();
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

void QmitkMultiLabelSegmentationView::SetMouseCursor( const mitk::ModuleResource resource, int hotspotX, int hotspotY )
{
  // Remove previously set mouse cursor
  if ( m_MouseCursorSet )
  {
    mitk::ApplicationCursor::GetInstance()->PopCursor();
  }

  mitk::ApplicationCursor::GetInstance()->PushCursor( resource, hotspotX, hotspotY );
  m_MouseCursorSet = true;
}

void QmitkMultiLabelSegmentationView::OnGoToLabel(const mitk::Point3D& pos)
{
  if (m_IRenderWindowPart)
    m_IRenderWindowPart->SetSelectedPosition(pos);
}

void QmitkMultiLabelSegmentationView::OnMaskStamp()
{
  mitk::DataNode* maskNode = m_Controls.m_cbMaskNodeSelector->GetSelectedNode();

  if (!maskNode)
  {
    QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a mask before starting some action.");
    return;
  }

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::Image* mask = dynamic_cast<mitk::Image*>(maskNode->GetData() );
  if ( !mask )
  {
    QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a mask before starting some action.");
    return;
  }

  mitk::DataNode * workingNode = m_Controls.m_LabelSetWidget->GetActiveLabelSetNode();

  if (!workingNode)
  {
   QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a segmentation before starting some action.");
   return;
  }

  mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );

  if (!lsImage)
  {
    QMessageBox::information( m_Parent, "Mask Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  lsImage->PasteMaskOnActiveLabel( mask, m_Controls.m_chkMaskStampOverwrite->isChecked() );

  maskNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

}

void QmitkMultiLabelSegmentationView::OnSurfaceStamp()
{
  mitk::DataNode* surfaceNode = m_Controls.m_cbSurfaceNodeSelector->GetSelectedNode();

  if (!surfaceNode)
  {
    QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a surface before starting some action.");
    return;
  }

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::Surface* surface = dynamic_cast<mitk::Surface*>(surfaceNode->GetData() );
  if ( !surface )
  {
    QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a surface before starting some action.");
    return;
  }

  mitk::DataNode * workingNode = m_Controls.m_LabelSetWidget->GetActiveLabelSetNode();

  if (!workingNode)
  {
   QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a segmentation before starting some action.");
   return;
  }

  mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() );

  if (!lsImage)
  {
    QMessageBox::information( m_Parent, "Surface Stamp", "Please load and select a segmentation before starting some action.");
    return;
  }

  lsImage->PasteSurfaceOnActiveLabel( surface, m_Controls.m_chkSurfaceStampOverwrite->isChecked() );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
