#include "mitkOpenGLRenderer.h"
#include "Mapper.h"
#include "ImageMapper2D.h"
#include "BaseVtkMapper2D.h"
#include "BaseVtkMapper3D.h"
#include "LevelWindow.h"

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkLight.h>

//##ModelId=3E33ECF301AD
mitk::OpenGLRenderer::OpenGLRenderer() : m_IilImage(0)
{

}

//##ModelId=3E3D28AB0018
void mitk::OpenGLRenderer::SetData(mitk::DataTreeIterator* iterator)
{
    mitk::BaseRenderer::SetData(iterator);

    m_VtkRenderWindow->RemoveRenderer(m_VtkRenderer);
    m_VtkRenderer->Delete();

    m_VtkRenderer = vtkRenderer::New();
    m_VtkRenderWindow->AddRenderer( this->m_VtkRenderer );

    m_Light->Delete();
    m_Light = vtkLight::New();
    m_VtkRenderer->AddLight( m_Light );

    bool first=true; //FIXME provisorium
//    try
    mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
    {
        while(it->hasNext())
        {
            if(first)
            {
                BaseData::Pointer data=it->get()->GetData();
                if(data!=NULL)
                {
                    Image::Pointer image = dynamic_cast<Image*>(data.GetPointer());
                    if(image!=NULL)
                    {
                        m_DisplayGeometry->SetWorldGeometry(image->GetGeometry2D(0, 0));
                        first=false;
                    }
                }
            }
//            mitk::LevelWindow lw;
            unsigned int dummy[] = {10,10,10};
            //Geometry3D geometry(3,dummy);
            mitk::Mapper::Pointer mapper = it->next()->GetMapper(defaultMapper);
            if(mapper!=NULL)
            {
//                mapper->Update();
                BaseVtkMapper2D* anVtkMapper2D;
                anVtkMapper2D=dynamic_cast<BaseVtkMapper2D*>(mapper.GetPointer());
                if(anVtkMapper2D!=NULL)
                {
                    anVtkMapper2D->Update();
                    m_VtkRenderer->AddProp(anVtkMapper2D->GetProp());
                }
                else
                {
                    BaseVtkMapper3D* anVtkMapper3D;
                    anVtkMapper3D=dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer());
                    if(anVtkMapper3D!=NULL)
                    {
                        anVtkMapper3D->Update();
                        m_VtkRenderer->AddProp(anVtkMapper3D->GetProp());
                    }
                }
            }
        }
    }
//    catch( ::itk::ExceptionObject ee)
//    {
//        printf("%s\n",ee.what());
////        itkGenericOutputMacro(ee->what());
//    }
//    catch( ...)
//    {
//            printf("test\n");
//    }
    delete it;

    Modified();
}

//##ModelId=3E330D260255
void mitk::OpenGLRenderer::Update()
{
    if(m_DataTreeIterator==NULL) return;
    mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
    while(it->hasNext())
    {
  //      mitk::LevelWindow lw;
        unsigned int dummy[] = {10,10,10};
        //Geometry3D geometry(3,dummy);
        mitk::Mapper::Pointer mapper = it->next()->GetMapper(defaultMapper);
        if(mapper!=NULL)
        {
            Mapper2D* mapper2d=dynamic_cast<Mapper2D*>(mapper.GetPointer());
            if(mapper2d!=NULL)
            {
								ImageMapper2D* imagemapper2d=dynamic_cast<ImageMapper2D*>(mapper.GetPointer());
								//								imagemapper2d->m_IilImage = &m_IilImage;
								//								mapper2d->SetDisplayGeometry(m_DisplayGeometry);
								mapper2d->Update();
            }
            else
            if(dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer()))
                mapper->Update();
        }
    }

    delete it;
    m_LastUpdateTime=GetMTime();
}
//##ModelId=3E330D2903CC
void mitk::OpenGLRenderer::Render()
{
// TODO: 
    if (m_LastUpdateTime<GetMTime() ||
    	m_LastUpdateTime<GetDisplayGeometry()->GetMTime() ||
     m_LastUpdateTime<GetDisplayGeometry()->GetWorldGeometry()->GetMTime() ) {
				std::cout << "OpenGLRenderer calling its update..." << std::endl;
         Update();
    }
    glClear(GL_COLOR_BUFFER_BIT);

    if(m_DataTreeIterator==NULL) return;
    mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
    while(it->hasNext())
    {
      mitk::Mapper::Pointer mapper = it->next()->GetMapper(defaultMapper);
      if(mapper!=NULL)
        {
	  ImageMapper2D* imagemapper2d=dynamic_cast<ImageMapper2D*>(mapper.GetPointer());
	  if(imagemapper2d!=NULL) {
	    imagemapper2d->Paint(this);
	  }
        }
    }
    
    delete it;
    //    m_VtkRenderWindow->Render();
}

/*!
\brief Initialize the OpenGLRenderer

This method is called from the two Constructors
*/
void mitk::OpenGLRenderer::InitRenderer(mitk::RenderWindow* renderwindow) 
{
    BaseRenderer::InitRenderer(renderwindow);

    m_InitNeeded = true;
    m_ResizeNeeded = true;
    m_VtkRenderWindow = vtkRenderWindow::New();
    m_VtkRenderer = vtkRenderer::New();
    m_VtkRenderWindow->AddRenderer( m_VtkRenderer );
    m_Light = vtkLight::New();
    m_VtkRenderer->AddLight( m_Light );
	//we should disable vtk doublebuffering, but then it doesn't work
    //m_VtkRenderWindow->SwapBuffersOff();
}

/*!
\brief Destructs the OpenGLRenderer.
*/
//##ModelId=3E33ECF301B7
mitk::OpenGLRenderer::~OpenGLRenderer() {
    m_VtkRenderer->Delete();
    m_VtkRenderWindow->Delete();
}

/*!
\brief Initialize the OpenGL Window
*/
//##ModelId=3E33145B0096
void mitk::OpenGLRenderer::Initialize( ) {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glColor3f(1.0, 0.0, 0.0);
}

/*!
\brief Resize the OpenGL Window
*/
//##ModelId=3E33145B00D2
void mitk::OpenGLRenderer::Resize( int w, int h) {
    glViewport (0, 0, w, h);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, w, 0.0, h );
    glMatrixMode( GL_MODELVIEW );

    m_DisplayGeometry->SetSizeInDisplayUnits(w, h);
    m_DisplayGeometry->Fit();
    Update();
//    m_VtkRenderWindow->SetSize(w,h); //FIXME?
}

/*!
\brief Render the scene
*/
//##ModelId=3E33145B005A
void mitk::OpenGLRenderer::Paint( )
{
    Render();
	glFlush();
}


//##ModelId=3E3314B0005C
void mitk::OpenGLRenderer::SetWindowId(void * id)
{
    m_VtkRenderWindow->SetWindowId( id );
}


//##ModelId=3E3799420227
void mitk::OpenGLRenderer::InitSize(int w, int h)
{
    m_VtkRenderWindow->SetSize(w,h);
     m_DisplayGeometry->Fit();
    Modified();
    Update();
}

