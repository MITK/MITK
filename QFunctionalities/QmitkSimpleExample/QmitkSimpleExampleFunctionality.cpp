#include "QmitkSimpleExampleFunctionality.h"
#include "QmitkSelectableGLWidget.h"
#include "PlaneGeometry.h"
#include <qaction.h>
#include <qslider.h>
#include "example.xpm"
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkFloatProperty.h>
#include <algorithm>

#if ((defined(_MSC_VER)) && (_MSC_VER <= 1200))
  #include <xutility>
#else
#ifndef _MAX
  #define _MAX	max
  #define _MIN	min
#endif
#endif

QmitkSimpleExampleFunctionality::QmitkSimpleExampleFunctionality(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIterator * it) : QmitkFunctionality(parent, name, it) ,
controls(NULL), multiWidget(mitkStdMultiWidget), opacityprop(NULL)
{
    setAvailability(true);
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
//void QmitkSimpleExampleFunctionality::activated()
//{
//
//}
//
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
    pv.point.y = y;
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
	mitk::DataTreeIterator* it=dataTree->clone();

	while (it->hasNext()) {

		it->next();
		printf("\nrequesting boundingbox\n");   



		mitk::DataTreeNode::Pointer node = it->get();

		

		if (node->GetData() != NULL ) {



			// get 

			if (node->GetData()->GetGeometry() != NULL	) {

				mitk::BoundingBox::ConstPointer bb = node->GetData()->GetGeometry()->GetBoundingBox();

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

			mitk::ColorProperty::Pointer colorprop;

			if (count == 1) {

				float color[3] = {1.0f,1.0f,1.0f};

				colorprop = new mitk::ColorProperty(color);

			}

			else {

				float color[3] = {1.0f,0.0f,0.0f};

				colorprop = new mitk::ColorProperty(color);

			}

			node->GetPropertyList()->SetProperty("color", colorprop);
			
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
