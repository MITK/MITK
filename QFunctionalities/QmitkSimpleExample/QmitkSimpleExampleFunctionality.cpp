#include "QmitkSimpleExampleFunctionality.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"

#include "QmitkSimpleExampleControls.h"

#include <qaction.h>
#include <qslider.h>

#include "example.xpm"

#include "PlaneGeometry.h"
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>

#include <EventMapper.h>
#include <GlobalInteraction.h>
#include <mitkCoordinateSupplier.h>
#include <mitkPointOperation.h>
#include <mitkDisplayCoordinateOperation.h>
#include <mitkDisplayVectorInteractor.h>
#include <BaseRenderer.h>
#include <mitkInteractionConst.h>

#include <algorithm>

#if (defined(_MSC_VER) && (_MSC_VER <= 1200))
  #include <xutility>
#else
  #define _MAX	max
  #define _MIN	min
#endif

QmitkSimpleExampleFunctionality::QmitkSimpleExampleFunctionality(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIterator * it) : QmitkFunctionality(parent, name, it) ,
controls(NULL), multiWidget(mitkStdMultiWidget), opacityprop(NULL)
{
    setAvailability(true);

    mitk::GlobalInteraction* globalInteraction = dynamic_cast<mitk::GlobalInteraction*>(mitk::EventMapper::GetGlobalStateMachine());
    if(globalInteraction!=NULL)
    {
	    globalInteraction->AddStateMachine(new mitk::CoordinateSupplier("navigation", this));//sends PointOperations
	    globalInteraction->AddStateMachine(new mitk::DisplayVectorInteractor("move", this));//sends DisplayCoordinateOperation
	    globalInteraction->AddStateMachine(new mitk::DisplayVectorInteractor("zoom", this));//sends DisplayCoordinateOperation
    }
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
        //connect(slider, SIGNAL(valueChanged(int)), lcd, SLOT(display(int)));
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
    action = new QAction( tr( "Left" ), QPixmap::fromMimeSource( "slicer.xpm" ), tr( "&Left" ), CTRL + Key_L, parent, "simple example" );
    //    action = new QAction( tr( "Left" ), QPixmap::fromMimeSource( "textleft.xpm" ), tr( "&Left" ), CTRL + Key_L, parent, "simple example" );
    return action;
}
void QmitkSimpleExampleFunctionality::activated()
{
    assert( multiWidget != NULL );
    // init widget 4 as a 3D widget
    multiWidget->mitkWidget4->GetRenderer()->SetMapperID(2);
}

//void QmitkSimpleExampleFunctionality::deactivated()
//{
//
//}

void QmitkSimpleExampleFunctionality::selectSliceWidgetXY( int z )
{
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget1->GetRenderer()->GetWorldGeometry();
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
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget3->GetRenderer()->GetWorldGeometry();
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
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget2->GetRenderer()->GetWorldGeometry();
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
	if(opacityprop!=NULL) {
		opacityprop->SetValue(p/100.0);
		multiWidget->updateMitkWidgets();
	}
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
		if((data!=NULL) && (dynamic_cast<mitk::Geometry2DData*>(node->GetData()) == NULL ))
		{
			// get 
			if (data->GetUpdatedGeometry() != NULL	) 
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
			// init pic color props
			if (count == 1) 
            {
				float color[3] = {1.0f,1.0f,1.0f};
				node->SetColor(color);
			}
			else 
            {
				float color[3] = {1.0f,0.0f,0.0f};
				node->SetColor(color);
			}
			
			// init pic opacity props
			if (opacityprop == NULL) {
				opacityprop = new mitk::FloatProperty(1.0f);
			}
			if (count == 1) {
				node->GetPropertyList()->SetProperty("opacity", new mitk::FloatProperty(1.0f) );
			}
			else {
				opacityprop->SetValue(0.5f);
				node->GetPropertyList()->SetProperty("opacity", opacityprop);
			}
		}
	}
	delete it;
}

void QmitkSimpleExampleFunctionality::ExecuteOperation(mitk::Operation* operation)
{
    bool ok;//as return type

    mitk::PointOperation* pointoperation=dynamic_cast<mitk::PointOperation*>(operation);
    if(pointoperation!=NULL)
    {
    /****NAVIGATION inside the volume****/
        switch (operation->GetOperationType())
        {
            case OpMOVE:
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
                        g2d = multiWidget->mitkWidget2->GetRenderer()->GetWorldGeometry();
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
                        g2d = multiWidget->mitkWidget3->GetRenderer()->GetWorldGeometry();
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
                        g2d = multiWidget->mitkWidget1->GetRenderer()->GetWorldGeometry();
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
            case OpNOTHING:
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
            case OpMOVE :
                {
                    renderer->GetDisplayGeometry()->MoveBy(dcOperation->GetLastToCurrentDisplayVector()*(-1.0));
                    renderer->GetRenderWindow()->Update();
                    ok = true;
                }
                break;
            case OpZOOM :
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