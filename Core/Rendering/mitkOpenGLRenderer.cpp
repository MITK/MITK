#include "mitkOpenGLRenderer.h"
#include "Mapper.h"
#include "ImageMapper2D.h"
#include "BaseVtkMapper2D.h"
#include "BaseVtkMapper3D.h"
#include "LevelWindow.h"
#include "mitkVtkInteractorCameraController.h"
#include "mitkVtkRenderWindow.h"

#include <vtkRenderer.h>
#include <vtkLight.h>
#include <vtkLightKit.h>
#include <vtkRenderWindow.h>
#include <vtkTransform.h>

#include "PlaneGeometry.h"

//##ModelId=3E33ECF301AD
mitk::OpenGLRenderer::OpenGLRenderer() : m_VtkMapperPresent(NULL)
{
    m_CameraController=NULL;
//    m_CameraController = VtkInteractorCameraController::New();
//    m_CameraController->AddRenderer(this);
}

//##ModelId=3E3D28AB0018
void mitk::OpenGLRenderer::SetData(mitk::DataTreeIterator* iterator)
{
	
	if(iterator!=GetData())
	{
		
		BaseRenderer::SetData(iterator);
		
		if (iterator != NULL) {
			
			//initialize world geometry: use first slice of first node containing an image
			mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
			while(it->hasNext())
			{
				it->next();
				BaseData::Pointer data=it->get()->GetData();
				if(data.IsNotNull())
				{
					Image::Pointer image = dynamic_cast<Image*>(data.GetPointer());
					if(image.IsNotNull())
					{
						SetWorldGeometry(image->GetGeometry2D(0, 0));
						break;
					}
				}
			}
			delete it;
		}
		
		//update the vtk-based mappers
        Update(); //this is only called to check, whether we have vtk-based mappers!
		UpdateVtkActors();
		
		Modified();
	}
}

//##ModelId=3ED91D060305
void mitk::OpenGLRenderer::UpdateVtkActors()
{
    VtkInteractorCameraController* vicc=dynamic_cast<VtkInteractorCameraController*>(m_CameraController.GetPointer());

    if (m_VtkMapperPresent == false)
    {
        if(vicc!=NULL)
            vicc->GetVtkInteractor()->Disable();
        return;
    }

    if(vicc!=NULL)
        vicc->GetVtkInteractor()->Enable();

    m_LightKit->RemoveLightsFromRenderer(this->m_VtkRenderer);
	
    m_MitkVtkRenderWindow->RemoveRenderer(m_VtkRenderer);
    m_VtkRenderer->Delete();
	
    m_VtkRenderer = vtkRenderer::New();
    m_VtkRenderer->SetLayer(0);
    m_MitkVtkRenderWindow->AddRenderer( this->m_VtkRenderer );
	
    //strange: when using a simple light, the backface of the planes are not shown (regardless of SetNumberOfLayers)
    //m_Light->Delete();
    //m_Light = vtkLight::New();
    //m_VtkRenderer->AddLight( m_Light );
    m_LightKit = vtkLightKit::New();
    m_LightKit->AddLightsToRenderer(m_VtkRenderer);
	
    //    try
	if (m_DataTreeIterator != NULL) 
	{
		mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
		while(it->hasNext())
		{
			it->next();
			mitk::Mapper::Pointer mapper = it->get()->GetMapper(m_MapperID);
			if(mapper.IsNotNull())
			{
				BaseVtkMapper2D* anVtkMapper2D;
				anVtkMapper2D=dynamic_cast<BaseVtkMapper2D*>(mapper.GetPointer());
				if(anVtkMapper2D != NULL)
				{
					anVtkMapper2D->Update();
					m_VtkRenderer->AddProp(anVtkMapper2D->GetProp());
				}
				else
				{
					BaseVtkMapper3D* anVtkMapper3D;
					anVtkMapper3D=dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer());
					if(anVtkMapper3D != NULL)
					{
						anVtkMapper3D->Update();
						m_VtkRenderer->AddProp(anVtkMapper3D->GetProp());
					}
				}
			}
		}
		delete it;
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
}

//##ModelId=3E330D260255
void mitk::OpenGLRenderer::Update()
{
    if(m_DataTreeIterator == NULL) return;

    m_VtkMapperPresent=false;

    mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
    while(it->hasNext())
    {
        it->next();
        //      mitk::LevelWindow lw;
        unsigned int dummy[] = {10,10,10};
        //Geometry3D geometry(3,dummy);
        mitk::Mapper::Pointer mapper = it->get()->GetMapper(m_MapperID);
        if(mapper.IsNotNull())
        {
            Mapper2D* mapper2d=dynamic_cast<Mapper2D*>(mapper.GetPointer());
            if(mapper2d != NULL)
            {
                ImageMapper2D* imagemapper2d=dynamic_cast<ImageMapper2D*>(mapper.GetPointer());
                mapper2d->Update();
            }
            else
                if(dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer()))
                    mapper->Update();
			if(dynamic_cast<BaseVtkMapper2D*>(mapper.GetPointer()) || dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer()))
				m_VtkMapperPresent=true;
        }
    }
	
    delete it;
    Modified();
    m_LastUpdateTime=GetMTime();
}

//##ModelId=3E330D2903CC
void mitk::OpenGLRenderer::Render()
{
	//if we do not have any data, we do nothing else but clearing our window
    if(GetData() == NULL)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		return;
	}

	//has someone transformed our worldgeometry-node? if so, incorporate this transform into
	//the worldgeometry itself and reset the transform of the node to identity
/*	if(m_WorldGeometryTransformTime<m_WorldGeometryNode->GetVtkTransform()->GetMTime())
	{
		vtkTransform *i;
		m_WorldGeometry->TransformGeometry(m_WorldGeometryNode->GetVtkTransform());
		m_WorldGeometryNode->GetVtkTransform()->Identity();
		m_WorldGeometryTransformTime=GetWorldGeometryNode()->GetVtkTransform()->GetMTime();
	}
*/	
	//has the data tree been changed?
	if(dynamic_cast<mitk::DataTree*>(GetData()->getTree()) == NULL ) return;
	//	if(m_LastUpdateTime<((mitk::DataTree*)GetData()->getTree())->GetMTime()) 
	if(m_LastUpdateTime < dynamic_cast<mitk::DataTree*>(GetData()->getTree())->GetMTime() ) 
	{
		//yes: update vtk-actors
		UpdateVtkActors();
        Update();
    }
	else
	//has anything else changed (geometry to display, etc.)?
	if (m_LastUpdateTime<GetMTime() ||
		m_LastUpdateTime<GetDisplayGeometry()->GetMTime() ||
		m_LastUpdateTime<GetDisplayGeometry()->GetWorldGeometry()->GetMTime())
	{
		//std::cout << "OpenGLRenderer calling its update..." << std::endl;
		Update();
	}
	else
	if(m_MapperID==2) 
	{ //@todo in 3D mode wird sonst nix geupdated, da z.Z. weder camera noch Änderung des Baums beachtet wird!!!
		Update();
	}
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	PlaneGeometry* myPlaneGeom =
		dynamic_cast<PlaneGeometry *>((mitk::Geometry2D*)(GetWorldGeometry()));
	
	glViewport (0, 0, m_Size[0], m_Size[1]);
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluOrtho2D( 0.0, m_Size[0], 0.0, m_Size[1] );
	glMatrixMode( GL_MODELVIEW );
	
	mitk::DataTreeIterator* it=m_DataTreeIterator->clone();
	mitk::DataTree::Pointer tree = dynamic_cast <mitk::DataTree *> (it->getTree());
//	std::cout << "Render:: tree: " <<  *tree << std::endl;
	
	while(it->hasNext())
	{        
		it->next();
		
		mitk::DataTreeNode::Pointer node = it->get();
		mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);
if(dynamic_cast<mitk::Image*>(node->GetData())==NULL)
{
		if(mapper.IsNotNull())
		{
			GLMapper2D* mapper2d=dynamic_cast<GLMapper2D*>(mapper.GetPointer());
			if(mapper2d!=NULL) {
				mapper2d->Paint(this);
			}
		}
}
	}
	
	delete it;
	
	if(m_VtkMapperPresent) {
		m_MitkVtkRenderWindow->MitkRender();
	}
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
	
    m_MitkVtkRenderWindow = mitk::VtkRenderWindow::New();
    m_MitkVtkRenderWindow->SetMitkRenderer(this);
    /**@todo SetNumberOfLayers commented out, because otherwise the backface of the planes are not shown (only, when a light is added).
    * But we need SetNumberOfLayers(2) later, when we want to prevent vtk to clear the widget before it renders (i.e., when we render something in the scene before vtk).
    */
    //m_MitkVtkRenderWindow->SetNumberOfLayers(2);
	
    m_VtkRenderer = vtkRenderer::New();
    m_MitkVtkRenderWindow->AddRenderer( m_VtkRenderer );
	
    //strange: when using a simple light, the backface of the planes are not shown (regardless of SetNumberOfLayers)
    //m_Light = vtkLight::New();
    //m_VtkRenderer->AddLight( m_Light );
    m_LightKit = vtkLightKit::New();
    m_LightKit->AddLightsToRenderer(m_VtkRenderer);
	
    if(m_CameraController)
        ((VtkInteractorCameraController*)m_CameraController.GetPointer())->SetRenderWindow(m_MitkVtkRenderWindow);
	
    //we should disable vtk doublebuffering, but then it doesn't work
    //m_MitkVtkRenderWindow->SwapBuffersOff();
}

/*!
\brief Destructs the OpenGLRenderer.
*/
//##ModelId=3E33ECF301B7
mitk::OpenGLRenderer::~OpenGLRenderer() {
    m_VtkRenderer->Delete();
    m_MitkVtkRenderWindow->Delete();
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
void mitk::OpenGLRenderer::Resize(int w, int h) 
{
    BaseRenderer::Resize(w, h);
	
    glViewport (0, 0, w, h);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluOrtho2D( 0.0, w, 0.0, h );
    glMatrixMode( GL_MODELVIEW );
	
    GetDisplayGeometry()->SetSizeInDisplayUnits(w, h);
    GetDisplayGeometry()->Fit();
	
    Update();
    //    m_MitkVtkRenderWindow->SetSize(w,h); //FIXME?
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
    m_MitkVtkRenderWindow->SetWindowId( id );
}


//##ModelId=3E3799420227
void mitk::OpenGLRenderer::InitSize(int w, int h)
{
    m_MitkVtkRenderWindow->SetSize(w,h);
    GetDisplayGeometry()->Fit();
    Modified();
    Update();
}

void mitk::OpenGLRenderer::MakeCurrent()
{
    if(m_RenderWindow!=NULL)
    {
        m_RenderWindow->MakeCurrent();
    }
}