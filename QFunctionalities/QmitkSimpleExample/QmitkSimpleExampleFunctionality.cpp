#include "QmitkSimpleExampleFunctionality.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"

#include "QmitkSimpleExampleControls.h"

#include <qaction.h>
#include <qslider.h>

#include "slicer.xpm"

#include "mitkPlaneGeometry.h"
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkLookupTable.h>

#include <mitkEventMapper.h>
#include <mitkGlobalInteraction.h>
#include <mitkCoordinateSupplier.h>
#include <mitkPointOperation.h>
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>
#include <mitkBaseRenderer.h>
#include <mitkInteractionConst.h>

#include <QmitkStepperAdapter.h>

#include <algorithm>

#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
  #include <xutility>
#else
  #define _MAX	max
  #define _MIN	min
#endif

QmitkSimpleExampleFunctionality::QmitkSimpleExampleFunctionality(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIterator * it) : QmitkFunctionality(parent, name, it) ,
controls(NULL), multiWidget(mitkStdMultiWidget), opacityprop(NULL), lookupTableProp(NULL)
{
    setAvailability(true);

    mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
    if(globalInteraction!=NULL)
    {
	    globalInteraction->AddStateMachine(new mitk::CoordinateSupplier("navigation", this));//sends PointOperations
	    globalInteraction->AddStateMachine(new mitk::DisplayVectorInteractor("move", this));//sends DisplayCoordinateOperation
	    globalInteraction->AddStateMachine(new mitk::DisplayVectorInteractor("zoom", this));//sends DisplayCoordinateOperation
    }

    m_DataTreeIterator->getTree()->addTreeChangeListener(this);
}

QmitkSimpleExampleFunctionality::~QmitkSimpleExampleFunctionality()
{
}

QString QmitkSimpleExampleFunctionality::getFunctionalityName()
{
    return "simple example";
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

        sliceNavigatorTransversal = mitk::SliceNavigationController::New();
        sliceNavigatorTransversal->SetViewDirection(mitk::SliceNavigationController::Transversal);
        sliceNavigatorTransversal->ConnectGeometrySliceEvent(multiWidget->mitkWidget1->GetRenderer().GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorTransversal(), sliceNavigatorTransversal->GetSlice(), "sliceNavigatorTransversalFromSimpleExample");

        sliceNavigatorSagittal = mitk::SliceNavigationController::New();
        sliceNavigatorSagittal->SetViewDirection(mitk::SliceNavigationController::Sagittal);
        sliceNavigatorSagittal->ConnectGeometrySliceEvent(multiWidget->mitkWidget2->GetRenderer().GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorSagittal(), sliceNavigatorSagittal->GetSlice(), "sliceNavigatorSagittalFromSimpleExample");

        sliceNavigatorFrontal = mitk::SliceNavigationController::New();
        sliceNavigatorFrontal->SetViewDirection(mitk::SliceNavigationController::Frontal);
        sliceNavigatorFrontal->ConnectGeometrySliceEvent(multiWidget->mitkWidget3->GetRenderer().GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorFrontal(), sliceNavigatorFrontal->GetSlice(), "sliceNavigatorFrontalFromSimpleExample");

        sliceNavigatorTime = mitk::SliceNavigationController::New();
        sliceNavigatorTime->ConnectGeometryTimeEvent(multiWidget->mitkWidget1->GetRenderer().GetPointer());
        new QmitkStepperAdapter(controls->getSliceNavigatorTime(), sliceNavigatorTime->GetTime(), "sliceNavigatorTimeFromSimpleExample");
    }
    return controls;
}

void QmitkSimpleExampleFunctionality::createConnections()
{
    connect(controls, SIGNAL(xySliderChanged(int)), this, SLOT(selectSliceWidgetXY(int)) );
    connect(controls, SIGNAL(yzSliderChanged(int)), this, SLOT(selectSliceWidgetYZ(int)) );
    connect(controls, SIGNAL(xzSliderChanged(int)), this, SLOT(selectSliceWidgetXZ(int)) );
    connect(controls, SIGNAL(fpSliderChanged(int)), this, SLOT(selectSliceWidgetFP(int)) );
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
    geometry->SetBoundingBox(boundingbox);

    //lets see if we have data with a limited live-span ...
    mitk::TimeBounds timebounds = mitk::DataTree::ComputeTimeBoundsInMS(m_DataTreeIterator, NULL, "includeInBoundingBox");
    if(timebounds[1]<mitk::ScalarTypeNumericTraits::max())
    {
      mitk::TimeSlicedGeometry::Pointer timegeometry = mitk::TimeSlicedGeometry::New();
      timegeometry->Initialize(10);
      timegeometry->SetTimeBoundsInMS(timebounds); //@bug really required?
      timegeometry->SetEvenlyTimed();
      mitk::ScalarType duration = timebounds[1]-timebounds[0];
      timebounds[1] = timebounds[0]+duration/10.0;

      timegeometry->SetGeometry3D(geometry, 0);
      geometry->SetTimeBoundsInMS(timebounds);

      geometry=timegeometry;
    }

    sliceNavigatorTransversal->SetInputWorldGeometry(geometry.GetPointer()); sliceNavigatorTransversal->Update();
    sliceNavigatorFrontal->SetInputWorldGeometry(geometry.GetPointer());     sliceNavigatorFrontal->Update();
    sliceNavigatorSagittal->SetInputWorldGeometry(geometry.GetPointer());    sliceNavigatorSagittal->Update();
    sliceNavigatorTime->SetInputWorldGeometry(geometry.GetPointer());        sliceNavigatorTime->Update();
  }
}

void QmitkSimpleExampleFunctionality::treeChanged(mitk::DataTreeIterator& itpos)
{
  if(isActivated())
    initNavigators();
}

void QmitkSimpleExampleFunctionality::activated()
{
  QmitkFunctionality::activated();
    assert( multiWidget != NULL );
    // init widget 4 as a 3D widget
    multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);

    initNavigators();
}

//void QmitkSimpleExampleFunctionality::deactivated()
//{
//
//}

void QmitkSimpleExampleFunctionality::selectSliceWidgetXY( int z )
{
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2D();
    const mitk::PlaneGeometry* pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
    mitk::PlaneView pv = pg->GetPlaneView();
    pv.point.z = z;
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();  
    plane->SetPlaneView(pv);
    multiWidget->mitkWidget1->GetRenderer()->SetWorldGeometry(plane);
    multiWidget->updateMitkWidgets();
    //std::cout << "Slider:" << z << std::endl;
}

void QmitkSimpleExampleFunctionality::selectSliceWidgetXZ(int y)
{
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2D();
    const mitk::PlaneGeometry* pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
    mitk::PlaneView pv = pg->GetPlaneView();
    pv.point.y=y;
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();  
    plane->SetPlaneView(pv);
    multiWidget->mitkWidget3->GetRenderer()->SetWorldGeometry(plane);
    multiWidget->updateMitkWidgets();
    //std::cout << "Slider:" << y << std::endl;
}

void QmitkSimpleExampleFunctionality::selectSliceWidgetYZ(int x)
{
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2D();
    const mitk::PlaneGeometry* pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
    mitk::PlaneView pv = pg->GetPlaneView();
    pv.point.x = x;
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();  
    plane->SetPlaneView(pv);
    multiWidget->mitkWidget2->GetRenderer()->SetWorldGeometry(plane);
    multiWidget->updateMitkWidgets();
    //std::cout << "Slider:" << x << std::endl;
}

void QmitkSimpleExampleFunctionality::selectSliceWidgetFP( int p )
{
  bool update=false;
  if( opacityprop.IsNotNull() ) 
  {
		opacityprop->SetValue(p/100.0);
    update = true;
	}
  if(lookupTableProp.IsNotNull())
  {
		lookupTableProp->GetLookupTable().ChangeOpacityForAll(p/100.0);
    update = true;
	}

  if(update)
    multiWidget->updateMitkWidgets();
}

void QmitkSimpleExampleFunctionality::initWidgets()
{
	int count = 0;
	mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
	while (it->hasNext()) {
		it->next();
		printf("\nrequesting boundingbox\n");   
		mitk::DataTreeNode::Pointer node = it->get();
		mitk::BaseData::Pointer data=node->GetData();
    if((data.IsNotNull()) && (dynamic_cast<mitk::Geometry2DData*>(node->GetData()) == NULL ))
		{
			// get 
			if ((data->GetUpdatedGeometry())!=NULL ) 
			{
				mitk::BoundingBox::ConstPointer bb = data->GetGeometry()->GetBoundingBox();
				printf("boundsArrayType\n");
				const mitk::BoundingBox::BoundsArrayType bounds = bb->GetBounds();
				printf("\nboundingbox\n");

				// init slider limits
				controls->sliderYZ->setMinValue(std::_MIN<int>(bounds[0],controls->sliderYZ->minValue()));
				controls->sliderYZ->setMaxValue(std::_MAX<int>(bounds[1],controls->sliderYZ->maxValue()));
				controls->sliderXZ->setMinValue(std::_MIN<int>(bounds[2],controls->sliderXZ->minValue()));
				controls->sliderXZ->setMaxValue(std::_MAX<int>(bounds[3],controls->sliderXZ->maxValue()));
				controls->sliderXY->setMinValue(std::_MIN<int>(bounds[4],controls->sliderXY->minValue()));
				controls->sliderXY->setMaxValue(std::_MAX<int>(bounds[5],controls->sliderXY->maxValue()));
				count++;
			}

      lookupTableProp=NULL;
		}
	}
	delete it;
}


void QmitkSimpleExampleFunctionality::ExecuteOperation(mitk::Operation* operation)
{
    bool ok;//as return

    mitk::PointOperation* pointoperation=dynamic_cast<mitk::PointOperation*>(operation);
    if(pointoperation!=NULL)
    {
    /****NAVIGATION inside the volume****/
        switch (operation->GetOperationType())
        {
        case mitk::OpMOVE:
                {
                    mitk::ITKPoint3D point;
                    mitk::Point3D seed;

                    point = pointoperation->GetPoint();
                    mitk::itk2vm(point, seed);

                    const mitk::Geometry2D* g2d;
                    const mitk::PlaneGeometry* pg;
                    mitk::PlaneView pv;
                    mitk::PlaneGeometry::Pointer plane;
            
                    int v;
                    v=(int)seed.x;
                    if(controls->getSliderYZ()->value()!=v)
                    {
                        g2d = multiWidget->mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2D();
                        pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
                        pv = pg->GetPlaneView();
                        pv.point.x = v; 
                        plane = mitk::PlaneGeometry::New();  
                        plane->SetPlaneView(pv);
                        multiWidget->mitkWidget2->GetRenderer()->SetWorldGeometry(plane);
                        controls->getSliderYZ()->blockSignals(true);
                        controls->getSliderYZ()->setValue(v);
                        controls->getSliderYZ()->blockSignals(false);
                    }
                    v=(int)seed.y;
                    if(controls->getSliderXZ()->value()!=v)
                    {
                        g2d = multiWidget->mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2D();
                        pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
                        pv = pg->GetPlaneView();
                        pv.point.y = v;
                        plane  = mitk::PlaneGeometry::New();  
                        plane->SetPlaneView(pv);
                        multiWidget->mitkWidget3->GetRenderer()->SetWorldGeometry(plane);
                        controls->getSliderXZ()->blockSignals(true);
                        controls->getSliderXZ()->setValue(v);
                        controls->getSliderXZ()->blockSignals(false);
                    }

                    v=(int)seed.z;
                    if(controls->getSliderXY()->value()!=v)
                    {
                        g2d = multiWidget->mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2D();
                        pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
                        pv = pg->GetPlaneView();
                        pv.point.z = v;
                        plane = mitk::PlaneGeometry::New();  
                        plane->SetPlaneView(pv);
                        multiWidget->mitkWidget1->GetRenderer()->SetWorldGeometry(plane);
                        controls->getSliderXY()->blockSignals(true);
                        controls->getSliderXY()->setValue(v);
                        controls->getSliderXY()->blockSignals(false);
                    }
                    multiWidget->updateMitkWidgets();
                    ok = true;
                }
                break;
            case mitk::OpNOTHING:
                break;
            default:
                ;
        }
    }
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
                    renderer->GetRenderWindow()->Update();
                    ok = true;
                }
                break;
            case mitk::OpZOOM :
                {
                    float distance = dcOperation->GetLastToCurrentDisplayVector().y;
                    distance = (distance > 0 ? 1 : (distance < 0 ? -1 : 0));
                    float factor= 1.0 + distance * 0.05;
                    renderer->GetDisplayGeometry()->Zoom(factor, dcOperation->GetStartDisplayCoordinate());
                    renderer->GetRenderWindow()->Update();
                    ok = true;
                }
                break;
            default:
                ;
        }
    }
}
