#include "QmitkSimpleExampleFunctionality.h"
#include "QmitkSimpleExampleControls.h"

#include <qaction.h>
#include "slicer.xpm"

// for slice-navigation
#include <mitkEventMapper.h>
#include <mitkGlobalInteraction.h>
#include <mitkBaseRenderer.h>
#include "QmitkRenderWindow.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkStdMultiWidget.h"
#include <QmitkStepperAdapter.h>
#include "qpushbutton.h"

// for stereo setting
#include <mitkOpenGLRenderer.h>
#include <mitkVtkRenderWindow.h>
#include <vtkRenderWindow.h>

// for zoom/pan
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>
#include <mitkInteractionConst.h>

QmitkSimpleExampleFunctionality::QmitkSimpleExampleFunctionality(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIterator * it) : QmitkFunctionality(parent, name, it) ,
controls(NULL), multiWidget(mitkStdMultiWidget), m_NavigatorsInitialized(false)
{
    setAvailability(true);

    mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
    if(globalInteraction!=NULL)
    {
	    globalInteraction->AddListener(new mitk::DisplayVectorInteractor("moveNzoom", this));//sends DisplayCoordinateOperation
    }

    m_DataTreeIterator->getTree()->addTreeChangeListener(this);
}

QmitkSimpleExampleFunctionality::~QmitkSimpleExampleFunctionality()
{
}

QString QmitkSimpleExampleFunctionality::getFunctionalityName()
{
    //return "simple example";
		return name();
		
}

QWidget * QmitkSimpleExampleFunctionality::createMainWidget(QWidget *parent)
{
    if (multiWidget == NULL) 
    {
        return multiWidget = new QmitkStdMultiWidget(parent);
    }
    else
        return NULL;
}

QWidget * QmitkSimpleExampleFunctionality::createControlWidget(QWidget *parent)
{
    if (controls == NULL)
    {
        controls = new QmitkSimpleExampleControls(parent);

        multiplexUpdateController = new mitk::MultiplexUpdateController("navigation");
        multiplexUpdateController->AddRenderWindow(multiWidget->mitkWidget1->GetRenderer()->GetRenderWindow());
        multiplexUpdateController->AddRenderWindow(multiWidget->mitkWidget2->GetRenderer()->GetRenderWindow());
        multiplexUpdateController->AddRenderWindow(multiWidget->mitkWidget3->GetRenderer()->GetRenderWindow());
        multiplexUpdateController->AddRenderWindow(multiWidget->mitkWidget4->GetRenderer()->GetRenderWindow());

        sliceNavigatorTransversal = new mitk::SliceNavigationController("navigation");
        sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
        sliceNavigatorTransversal->ConnectGeometrySliceEvent(multiWidget->mitkWidget1->GetRenderer().GetPointer());
        sliceNavigatorTransversal->ConnectGeometrySendEvent(multiWidget->mitkWidget4->GetRenderer().GetPointer());
        sliceNavigatorTransversal->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorTransversal(), sliceNavigatorTransversal->GetSlice(), "sliceNavigatorTransversalFromSimpleExample");

        sliceNavigatorSagittal = new mitk::SliceNavigationController("navigation");
        sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
        sliceNavigatorSagittal->ConnectGeometrySliceEvent(multiWidget->mitkWidget2->GetRenderer().GetPointer());
        sliceNavigatorSagittal->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorSagittal(), sliceNavigatorSagittal->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");

        sliceNavigatorFrontal = new mitk::SliceNavigationController("navigation");
        sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);
        sliceNavigatorFrontal->ConnectGeometrySliceEvent(multiWidget->mitkWidget3->GetRenderer().GetPointer());
        sliceNavigatorFrontal->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorFrontal(), sliceNavigatorFrontal->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");

        sliceNavigatorTime = new mitk::SliceNavigationController(NULL);
        sliceNavigatorTime->ConnectGeometryTimeEvent(multiWidget->mitkWidget1->GetRenderer().GetPointer(), false);
        sliceNavigatorTime->ConnectGeometryTimeEvent(multiWidget->mitkWidget2->GetRenderer().GetPointer(), false);
        sliceNavigatorTime->ConnectGeometryTimeEvent(multiWidget->mitkWidget3->GetRenderer().GetPointer(), false);
        sliceNavigatorTime->ConnectGeometryTimeEvent(multiWidget->mitkWidget4->GetRenderer().GetPointer(), false);
        sliceNavigatorTime->ConnectRepaintRequest(multiplexUpdateController.GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorTime(), sliceNavigatorTime->GetTime(), "sliceNavigatorTimeFromSimpleExample");

        mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
        if(globalInteraction!=NULL)
        {
    	    globalInteraction->AddListener(multiplexUpdateController);
        }
    }
    return controls;
}

void QmitkSimpleExampleFunctionality::createConnections()
{
  if ( controls ) 
  {
    connect(controls->getStereoSelect(), SIGNAL(activated(int)), this, SLOT(stereoSelectionChanged(int)) );
    connect(controls->getReInitializeNavigatorsButton(), SIGNAL(clicked()), this, SLOT(initNavigators()) );
  }
}

QAction * QmitkSimpleExampleFunctionality::createAction(QActionGroup *parent)
{
    QAction* action;
    action = new QAction( tr( "Left" ), QPixmap((const char**)slicer_xpm), tr( "&Left" ), CTRL + Key_L, parent, "simple example" );
    //    action = new QAction( tr( "Left" ), QPixmap(textleft_xpm), tr( "&Left" ), CTRL + Key_L, parent, "simple example" );
    return action;
}

void QmitkSimpleExampleFunctionality::initNavigators()
{
  const mitk::BoundingBox::Pointer boundingbox = mitk::DataTree::ComputeVisibleBoundingBox(m_DataTreeIterator, NULL, "includeInBoundingBox");
  if(boundingbox->GetPoints()->Size()>0)
  {
    // const mitk::BoundingBox::Pointer bb=boundingbox;
    //const mitk::BoundingBox::BoundsArrayType bounds = bb->GetBounds();
    // float boundingbox[6]={-2*bounds[1],2*bounds[1],-2*bounds[3],2*bounds[3],-2*bounds[5],2*bounds[5]};
    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    geometry->Initialize();
    geometry->SetBounds(boundingbox->GetBounds());

    //lets see if we have data with a limited live-span ...
    mitk::TimeBounds timebounds = mitk::DataTree::ComputeTimeBoundsInMS(m_DataTreeIterator, NULL, "includeInBoundingBox");
    if(timebounds[1]<mitk::ScalarTypeNumericTraits::max())
    {
      mitk::ScalarType duration = timebounds[1]-timebounds[0];

      mitk::TimeSlicedGeometry::Pointer timegeometry = mitk::TimeSlicedGeometry::New();
      timegeometry->InitializeEvenlyTimed(geometry, duration);
      timegeometry->SetTimeBoundsInMS(timebounds); //@bug really required? FIXME

      timebounds[1] = timebounds[0]+1.0f;
      geometry->SetTimeBoundsInMS(timebounds);

      geometry=timegeometry;
    }

    if(const_cast<mitk::BoundingBox*>(geometry->GetBoundingBox())->GetDiagonalLength2()>=mitk::eps)
    {
      multiplexUpdateController->SetBlockUpdate(true);
        sliceNavigatorTransversal->SetInputWorldGeometry(geometry.GetPointer()); sliceNavigatorTransversal->Update();
        sliceNavigatorSagittal->SetInputWorldGeometry(geometry.GetPointer());    sliceNavigatorSagittal->Update();
        sliceNavigatorFrontal->SetInputWorldGeometry(geometry.GetPointer());     sliceNavigatorFrontal->Update();
        sliceNavigatorTime->SetInputWorldGeometry(geometry.GetPointer());        sliceNavigatorTime->Update();
      multiplexUpdateController->SetBlockUpdate(false);
      multiplexUpdateController->UpdateRequest();

      m_NavigatorsInitialized=true;
    }
  }
}

void QmitkSimpleExampleFunctionality::treeChanged(mitk::DataTreeIterator& itpos)
{
  if(m_NavigatorsInitialized==false)
    initNavigators();
}

void QmitkSimpleExampleFunctionality::activated()
{
  QmitkFunctionality::activated();
  assert( multiWidget != NULL );
  // init widget 4 as a 3D widget
  multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);

  if(m_NavigatorsInitialized)
  {
    multiplexUpdateController->SetBlockUpdate(true);
      sliceNavigatorTransversal->Update();
      sliceNavigatorSagittal->Update();
      sliceNavigatorFrontal->Update();
      sliceNavigatorTime->Update();
    multiplexUpdateController->SetBlockUpdate(false);
    multiplexUpdateController->UpdateRequest();
  }
}

//void QmitkSimpleExampleFunctionality::deactivated()
//{
//
//}

void QmitkSimpleExampleFunctionality::stereoSelectionChanged( int id )
{
  vtkRenderWindow * vtkrenderwindow =
    ((mitk::OpenGLRenderer*)(multiWidget->mitkWidget4->GetRenderer().GetPointer()))->GetVtkRenderWindow();
  switch(id)
  {
  case 0:
    vtkrenderwindow->StereoRenderOff();
    break;
  case 1:
    vtkrenderwindow->SetStereoTypeToDresden();
    vtkrenderwindow->StereoRenderOn();
    break;
  }
  multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);
  multiWidget->mitkWidget4->GetRenderer()->GetRenderWindow()->Repaint();
}

void QmitkSimpleExampleFunctionality::ExecuteOperation(mitk::Operation* operation)
{
  bool ok;//as return

  mitk::DisplayCoordinateOperation* dcOperation=dynamic_cast<mitk::DisplayCoordinateOperation*>(operation);
  if( dcOperation != NULL )
  {
  /****ZOOM & MOVE of the whole volume****/
      mitk::BaseRenderer* renderer = dcOperation->GetRenderer();
      if( renderer == NULL )
          return;
      switch (operation->GetOperationType())
      {
          case mitk::OpMOVE :
              {
                  renderer->GetDisplayGeometry()->MoveBy(dcOperation->GetLastToCurrentDisplayVector()*(-1.0));
                  renderer->GetRenderWindow()->Repaint();
                  ok = true;
              }
              break;
          case mitk::OpZOOM :
              {
                  float distance = dcOperation->GetLastToCurrentDisplayVector()[1];
                  distance = (distance > 0 ? 1 : (distance < 0 ? -1 : 0));
                  float factor= 1.0 + distance * 0.05;
                  renderer->GetDisplayGeometry()->Zoom(factor, dcOperation->GetStartDisplayCoordinate());
                  renderer->GetRenderWindow()->Repaint();
                  ok = true;
              }
              break;
          default:
              ;
      }
  }
}
