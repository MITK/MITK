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
    hlayout=new QHBoxLayout(container);
    
    // create Renderer
    renderer=mitk::OpenGLRenderer::New();
    
    // create widget
    QString composedName("QmitkSelectableGLWidget::");
    if(name()!=NULL)
        composedName+=name();
    else
        composedName+="QmitkGLWidget";
    QWidget *mitkWidget = new mitk::QmitkRenderWindow(renderer, container, composedName);
    hlayout->addWidget(mitkWidget);
}


mitk::OpenGLRenderer::Pointer QmitkSelectableGLWidget::GetRenderer()
{
    return renderer;
}


QFrame* QmitkSelectableGLWidget::GetSelectionFrame()
{
    return SelectionFrame;
}
