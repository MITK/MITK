/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "mitkProperties.h"
#include <mitkBoolProperty.h>
#include <mitkStringProperty.h>
#include <mitkGeometry2DDataVtkMapper3D.h>
#include <vtkRenderer.h>
#include <mitkOpenGLRenderer.h>

void QmitkStdMultiWidget::updateMitkWidgets()
{
  mitkWidget1->GetRenderer()->GetRenderWindow()->Repaint(); 
  mitkWidget2->GetRenderer()->GetRenderWindow()->Repaint(); 
  mitkWidget3->GetRenderer()->GetRenderWindow()->Repaint(); 
  mitkWidget4->GetRenderer()->GetRenderWindow()->Repaint();
}

void QmitkStdMultiWidget::init()
{
  mitkWidget1->GetSelectionFrame()->setPaletteBackgroundColor ("red");
  mitkWidget2->GetSelectionFrame()->setPaletteBackgroundColor ("green");
  mitkWidget3->GetSelectionFrame()->setPaletteBackgroundColor ("blue");
  mitkWidget4->GetSelectionFrame()->setPaletteBackgroundColor ("yellow");

  //transfer colors in WorldGeometry-Nodes of the associated Renderer
  QColor qcolor;
  float color[3] = {1.0f,1.0f,1.0f};
  mitk::DataTreeNode::Pointer planeNode;
  mitk::IntProperty::Pointer  layer;
  // ... of widget 1
  qcolor=mitkWidget1->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 2
  qcolor=mitkWidget2->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(2);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 3
  qcolor=mitkWidget3->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(3);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 4
  qcolor=mitkWidget4->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget4->GetRenderer()->GetCurrentWorldGeometry2DNode();
  planeNode->SetColor(color);
}

void QmitkStdMultiWidget::changeLayoutTo2DImagesUp()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to 2D images up... " << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0,02, "QmitkStdMultiWidgetLayout");

    QHBoxLayout  * layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    QGridLayout *layout3 = new QGridLayout( 0, 2, 1, 0, 6, "layout3");

    QHBoxLayout *TwoDLayout = new QHBoxLayout( 0, 0, 6, "2DLayout");
    
    mitkWidget1->setMaximumSize(2000,200);
    mitkWidget2->setMaximumSize(2000,200);
    mitkWidget3->setMaximumSize(2000,200);    
    
   TwoDLayout ->addWidget( mitkWidget1 );
   TwoDLayout ->addWidget( mitkWidget2 );
   TwoDLayout ->addWidget( mitkWidget3 );
   
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   if (mitkWidget2->isHidden()) mitkWidget2->show();
   if (mitkWidget3->isHidden()) mitkWidget3->show();
   if (mitkWidget4->isHidden()) mitkWidget4->show();
      
      
   layout3->addLayout( TwoDLayout , 0, 0 );
   layout3->addWidget( mitkWidget4, 1, 0 );
    
    
    layout4->addLayout( layout3 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}

void QmitkStdMultiWidget::changeLayoutTo2DImagesLeft()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to 2D images left... " << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0,0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    QGridLayout *layout3 = new QGridLayout( 0, 1, 2, 0, 6, "layout3");

    QVBoxLayout *TwoDLayout = new QVBoxLayout( 0, 0, 6, "2DLayout");
    
    mitkWidget1->setMaximumSize(300,300);
    mitkWidget2->setMaximumSize(300,300);
    mitkWidget3->setMaximumSize(300,300);    
    
   TwoDLayout ->addWidget( mitkWidget1 );
   TwoDLayout ->addWidget( mitkWidget2 );
   TwoDLayout ->addWidget( mitkWidget3 );
   
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   if (mitkWidget2->isHidden()) mitkWidget2->show();
   if (mitkWidget3->isHidden()) mitkWidget3->show();
   if (mitkWidget4->isHidden()) mitkWidget4->show();
   
   
   layout3->addLayout( TwoDLayout , 0, 0 );
   layout3->addWidget( mitkWidget4, 0,1 );
    
    
    layout4->addLayout( layout3 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}
    
void QmitkStdMultiWidget::changeLayoutToDefault()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to defualt... " << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    QGridLayout *layout3 = new QGridLayout( 0, 2, 2, 0, 6, "layout3");

   
    mitkWidget1->setMaximumSize(2000,2000);
    mitkWidget2->setMaximumSize(2000,2000);
    mitkWidget3->setMaximumSize(2000,2000);    
    
   layout3 ->addWidget( mitkWidget1,0,0 );
   layout3 ->addWidget( mitkWidget2,0,1 );
   layout3 ->addWidget( mitkWidget3,1,0 );
   
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   if (mitkWidget2->isHidden()) mitkWidget2->show();
   if (mitkWidget3->isHidden()) mitkWidget3->show();
   if (mitkWidget4->isHidden()) mitkWidget4->show();
   
   layout3->addWidget( mitkWidget4, 1,1 );
    
    
    layout4->addLayout( layout3 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}


void QmitkStdMultiWidget::changeLayoutToBig3D()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to big 3D ..." << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    mitkWidget1->hide();
    mitkWidget2->hide();
    mitkWidget3->hide();	
   if (mitkWidget4->isHidden()) mitkWidget4->show();
   
    layout4->addWidget( mitkWidget4 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}

void QmitkStdMultiWidget::changeLayoutToWidget1()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to big Widget1 ..." << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    
    mitkWidget1->setMaximumSize(2000,2000);    

   
    mitkWidget2->hide();
    mitkWidget3->hide();
    mitkWidget4->hide();	
   if (mitkWidget1->isHidden()) mitkWidget1->show();
   
    layout4->addWidget( mitkWidget1 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}
void QmitkStdMultiWidget::changeLayoutToWidget2()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to big Widget2 ..." << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    
    mitkWidget2->setMaximumSize(2000,2000);    
 
    
    mitkWidget1->hide();
    mitkWidget3->hide();
    mitkWidget4->hide();	
   if (mitkWidget2->isHidden()) mitkWidget2->show();    
   
    layout4->addWidget( mitkWidget2 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}

void QmitkStdMultiWidget::changeLayoutToWidget3()
{
   
    delete QmitkStdMultiWidgetLayout ;
    
    std::cout << "changing layout to big Widget1 ..." << std::endl;
    QmitkStdMultiWidgetLayout = new QHBoxLayout( this, 0, 0, "QmitkStdMultiWidgetLayout");

    QHBoxLayout *layout4 = new QHBoxLayout( 0, 0, 6, "layout4");
    
    mitkWidget3->setMaximumSize(2000,2000);    
 
   
    mitkWidget1->hide();
    mitkWidget2->hide();
    mitkWidget4->hide();	
  if (mitkWidget3->isHidden()) mitkWidget3->show();
  
    layout4->addWidget( mitkWidget3 );
    layout4->addWidget( levelWindowWidget );
    QmitkStdMultiWidgetLayout->addLayout( layout4 );

}

void QmitkStdMultiWidget::setData( mitk::DataTreeIterator * it )
{
  mitkWidget1->GetRenderer()->SetData(it);
  mitkWidget2->GetRenderer()->SetData(it);
  mitkWidget3->GetRenderer()->SetData(it);
  mitkWidget4->GetRenderer()->SetData(it);
}

void QmitkStdMultiWidget::fit()
{
  vtkRenderer * vtkrenderer;
  mitkWidget1->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget2->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget3->GetRenderer()->GetDisplayGeometry()->Fit();
  mitkWidget4->GetRenderer()->GetDisplayGeometry()->Fit();

  int w=vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget1->GetRenderer().GetPointer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget2->GetRenderer().GetPointer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget3->GetRenderer().GetPointer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkrenderer = ((mitk::OpenGLRenderer*)(mitkWidget4->GetRenderer().GetPointer()))->GetVtkRenderer();
  if(vtkrenderer!=NULL) vtkrenderer->ResetCamera();
  vtkObject::SetGlobalWarningDisplay(w);
}

void QmitkStdMultiWidget::initWidget(mitk::DataTreeIterator* it,
                                   QmitkSelectableGLWidget* widget,
		        const mitk::Vector3D& origin,
                                   const mitk::Vector3D& right,
                                   const mitk::Vector3D& bottom)
{

  mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();

  //ohne den Pointer-Umweg meckert gcc
  mitk::PlaneView* view = new mitk::PlaneView(origin,right,bottom);
  plane->SetPlaneView(*view);
  delete view;

  widget->GetRenderer()->SetWorldGeometry(plane);

  widget->update();
}

void QmitkStdMultiWidget::initWidgets( mitk::DataTreeIterator * it )
{
  const mitk::BoundingBox::BoundsArrayType bounds = mitk::DataTree::ComputeBoundingBox(it)->GetBounds();
  printf("\nboundingbox\n");

  setData(it);
  
  initWidget(it,
    mitkWidget1,
    Vector3f(bounds[0],bounds[3],bounds[5]),
    Vector3f(bounds[1],bounds[3],bounds[5]),
    Vector3f(bounds[0],bounds[2],bounds[5])
    );
  printf("\nw1 init\n");

  // YZ
  initWidget(it,mitkWidget2,
    Vector3f(bounds[0],bounds[2],bounds[4]),
    Vector3f(bounds[0],bounds[3],bounds[4]),
    Vector3f(bounds[0],bounds[2],bounds[5])
    );
  // XZ
  initWidget(it,
    mitkWidget3,
    Vector3f(bounds[0],bounds[2],bounds[4]),
    Vector3f(bounds[1],bounds[2],bounds[4]),
    Vector3f(bounds[0],bounds[2],bounds[5])
    );
  mitkWidget4->GetRenderer()->SetData(it);
  
  fit();
}

void QmitkStdMultiWidget::addPlaneSubTree(mitk::DataTreeIterator * it)
{
    // add the diplayed planes of the multiwidget to a node to which the subtree @a planesSubTree points ...
    it=it->clone();
    
    mitk::DataTreeNode::Pointer node=mitk::DataTreeNode::New();
    it->add(node);
    it->next();
    it->next();
    planesSubTree=dynamic_cast<mitk::DataTreeBase*>(it->getSubTree());

    float white[3] = {1.0f,1.0f,1.0f};
    mitk::DataTreeNode::Pointer planeNode;
    // ... of widget 1
    planeNode=mitkWidget1->GetRenderer()->GetCurrentWorldGeometry2DNode();
    planeNode->SetColor(white, mitkWidget4->GetRenderer());
    planeNode->SetProperty("fileName", new mitk::StringProperty("widget1Plane"));
    planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
    it->add(planeNode);
    // ... of widget 2
    planeNode=mitkWidget2->GetRenderer()->GetCurrentWorldGeometry2DNode();
    planeNode->SetColor(white, mitkWidget4->GetRenderer());
    planeNode->SetProperty("fileName", new mitk::StringProperty("widget2Plane"));
    planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
    it->add(planeNode);
    // ... of widget 3
    planeNode=mitkWidget3->GetRenderer()->GetCurrentWorldGeometry2DNode();
    planeNode->SetColor(white, mitkWidget4->GetRenderer());
    planeNode->SetProperty("fileName", new mitk::StringProperty("widget3Plane"));
    planeNode->SetProperty("includeInBoundingBox", new mitk::BoolProperty(false));
    it->add(planeNode);

    delete it;
}

void QmitkStdMultiWidget::texturizePlaneSubTree(mitk::DataTreeIterator * it)
{
  if(planesSubTree == NULL)
      return;

  mitk::DataTreeIterator *git = planesSubTree->inorderIterator();
  while(git->hasNext())
  {
    git->next();
    if(dynamic_cast<mitk::Geometry2DData*>(git->get()->GetData())!=NULL)
    {
      mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper;
      if(git->get()->GetMapper(2)==NULL)
      {
        geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
        it->get()->SetMapper(2, geometryMapper);
      }
      else
        geometryMapper = dynamic_cast<mitk::Geometry2DDataVtkMapper3D*>(git->get()->GetMapper(2));
      if(geometryMapper.IsNotNull())
        geometryMapper->SetDataIteratorForTexture(it);
    } 
  }
  delete git;
}
