/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include "mitkProperties.h"

void QmitkStdMultiWidget::updateMitkWidgets()
{
  mitkWidget1->GetRenderer()->GetRenderWindow()->updateGL(); 
  mitkWidget2->GetRenderer()->GetRenderWindow()->updateGL(); 
  mitkWidget3->GetRenderer()->GetRenderWindow()->updateGL(); 
  mitkWidget4->GetRenderer()->GetRenderWindow()->updateGL();
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
  planeNode=mitkWidget1->GetRenderer()->GetWorldGeometryNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(1);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 2
  qcolor=mitkWidget2->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget2->GetRenderer()->GetWorldGeometryNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(2);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 3
  qcolor=mitkWidget3->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget3->GetRenderer()->GetWorldGeometryNode();
  planeNode->SetColor(color);
  layer = new mitk::IntProperty(3);	
  planeNode->SetProperty("layer",layer); 	  
  // ... of widget 4
  qcolor=mitkWidget4->GetSelectionFrame()->paletteBackgroundColor();
  color[0]=qcolor.red()/255.0; color[1]=qcolor.green()/255.0; color[2]=qcolor.blue()/255.0;
  planeNode=mitkWidget4->GetRenderer()->GetWorldGeometryNode();
  planeNode->SetColor(color);

}
