/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void QmitkSelectableGLWidget::init()
{	
    QHBoxLayout *hlayout;
    hlayout=new QHBoxLayout(SelectionFrame);
    
    // create Renderer
    renderer=mitk::OpenGLRenderer::New();
    
    // create widget
    QWidget *mitkWidget = new mitk::QmitkGLWidget(renderer, SelectionFrame,"QmitkSelectableGLWidget::QmitkGLWidget");
    hlayout->addWidget(mitkWidget);
}


mitk::OpenGLRenderer::Pointer QmitkSelectableGLWidget::GetRenderer()
{
    return renderer;
}
