#include "QmitkSimpleExampleFunctionality.h"
#include "QmitkSelectableGLWidget.h"
#include "PlaneGeometry.h"
#include <qaction.h>
#include "example.xpm"
#include <mitkSurfaceData.h>
#include <mitkColorProperty.h>
#include <mitkFloatProperty.h>

QmitkSimpleExampleFunctionality::QmitkSimpleExampleFunctionality(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget) : QmitkFunctionality(parent, name) ,
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
    if(opacityprop!=NULL)
        opacityprop->SetValue(p/100.0);
    multiWidget->updateMitkWidgets();
    return;
    const mitk::Geometry2D* g2d = multiWidget->mitkWidget4->GetRenderer()->GetWorldGeometry();
    const mitk::PlaneGeometry* pg = dynamic_cast<const mitk::PlaneGeometry*>(g2d);
    mitk::PlaneView pv = pg->GetPlaneView();
    pv.point = pv.normal*p;
    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();  
    plane->SetPlaneView(pv);
    multiWidget->mitkWidget4->GetRenderer()->SetWorldGeometry(plane);
    multiWidget->updateMitkWidgets();
    //std::cout << "Slider:" << p << std::endl;
}
