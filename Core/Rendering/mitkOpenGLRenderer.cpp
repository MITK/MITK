/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/


#include "mitkOpenGLRenderer.h"
#include "mitkMapper.h"
#include "mitkImageMapper2D.h"
#include "mitkBaseVtkMapper2D.h"
#include "mitkBaseVtkMapper3D.h"
#include "mitkLevelWindow.h"
#include "mitkVtkInteractorCameraController.h"
#include "mitkVtkRenderWindow.h"
#include "mitkVtkStencilRenderWindow.h"
#include "mitkRenderWindow.h"
#include "mitkGeometry2DDataVtkMapper3D.h"

#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkLight.h>
#include <vtkLightKit.h>
#include <vtkRenderWindow.h>
#include <vtkLinearTransform.h>
#include <vtkCamera.h>
#include <vtkWorldPointPicker.h>

#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include <queue>
#include <utility>

#include "mitkGL.h"
#include <GL/glu.h>


//##ModelId=3E33ECF301AD
mitk::OpenGLRenderer::OpenGLRenderer() : m_VtkMapperPresent(false),
    m_VtkRenderer(NULL), m_LastUpdateVtkActorsTime(0)
{
  m_CameraController=NULL;//\*todo remove line
  m_CameraController = new VtkInteractorCameraController();

  m_WorldPointPicker = vtkWorldPointPicker::New();

  m_DataChangedCommand = itk::MemberCommand<mitk::OpenGLRenderer>::New();
#ifdef WIN32
  m_DataChangedCommand->SetCallbackFunction(this, mitk::OpenGLRenderer::DataChangedEvent);
#else
  m_DataChangedCommand->SetCallbackFunction(this, &mitk::OpenGLRenderer::DataChangedEvent);
#endif

  mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
  m_CurrentWorldGeometry2DMapper = geometryMapper;
  m_CurrentWorldGeometry2DNode->SetMapper(2, geometryMapper);
}

//##ModelId=3E3D28AB0018
void mitk::OpenGLRenderer::SetData(const mitk::DataTreeIteratorBase* iterator)
{
  if(iterator!=GetData())
  {

    bool geometry_is_set=false;

    BaseRenderer::SetData(iterator);
    static_cast<mitk::Geometry2DDataVtkMapper3D*>(m_CurrentWorldGeometry2DMapper.GetPointer())->SetDataIteratorForTexture(m_DataTreeIterator.GetPointer());

    if (iterator != NULL)
    {
      //initialize world geometry: use first slice of first node containing an image
      mitk::DataTreeIteratorClone it=m_DataTreeIterator;
      for(;it->IsAtEnd()==false;++it)
      {
        mitk::DataTreeNode::Pointer node = it->Get();
        if(node.IsNull())
          continue;
        BaseData::Pointer data=node->GetData();
        if(data.IsNull())
          continue;
        Image::Pointer image = dynamic_cast<Image*>(data.GetPointer());
        if((image.IsNotNull()) && (image->GetUpdatedGeometry()!=NULL))
        {
          mitk::PlaneGeometry::Pointer planegeometry = mitk::PlaneGeometry::New();
          planegeometry->InitializeStandardPlane(
            image->GetGeometry(),
            PlaneGeometry::Transversal,
            image->GetGeometry()->GetExtent(2)-1, false);
          SetWorldGeometry(planegeometry);
          geometry_is_set=true;
          //@todo add connections
          //data->AddObserver(itk::EndEvent(), m_DataChangedCommand);
        }
      }
    }

    //update the vtk-based mappers
    UpdateIncludingVtkActors();

    Modified();
  }
}

//##ModelId=3ED91D060305
void mitk::OpenGLRenderer::UpdateIncludingVtkActors()
{
  Update();

  m_LastUpdateVtkActorsTime = GetMTime();

  VtkInteractorCameraController* vicc=dynamic_cast<VtkInteractorCameraController*>(m_CameraController.GetPointer());

  if (m_VtkMapperPresent == false)
  {
    if(vicc!=NULL)
      vicc->GetVtkInteractor()->Disable();
    return;
  }

  if(vicc!=NULL)
    vicc->GetVtkInteractor()->Enable();

  //    m_LightKit->RemoveLightsFromRenderer(this->m_VtkRenderer);

  //    m_RenderWindow->GetVtkRenderWindow()->RemoveRenderer(m_VtkRenderer);
  //    m_VtkRenderer->Delete();

  bool newRenderer = false;
  if(m_VtkRenderer==NULL)
  {
    m_VtkRenderer = vtkRenderer::New();
    m_VtkRenderer->SetLayer(0);
    m_RenderWindow->GetVtkRenderWindow()->AddRenderer( this->m_VtkRenderer );
    newRenderer = true;

    //strange: when using a simple light, the backface of the planes are not shown (regardless of SetNumberOfLayers)
    m_Light = vtkLight::New();
    m_Light->SetLightTypeToCameraLight();
    double pos[]= {0.5,0.2,1.0};
    m_Light->SetPosition(pos);

    m_VtkRenderer->AddLight( m_Light );
    //m_LightKit = vtkLightKit::New();
    //m_LightKit->AddLightsToRenderer(m_VtkRenderer);
  }

  m_VtkRenderer->RemoveAllProps();

  //strange: when using a simple light, the backface of the planes are not shown (regardless of SetNumberOfLayers)
  //m_Light->Delete();
  //m_Light = vtkLight::New();
  //m_VtkRenderer->AddLight( m_Light );
  //if(m_LightKit!=NULL)
  //{
  //  m_LightKit = vtkLightKit::New();
  //  m_LightKit->AddLightsToRenderer(m_VtkRenderer);
  // }

  // Layer sceme to be able to sort the painting order.
  // Nodes, without a layer are painted last, cause their order has no meaning.
  // Nodes with a high layer are painted first (e.g. data necessary for stencil-buffer)
  if (m_DataTreeIterator.IsNotNull())
  {
    mitk::DataTreeIteratorClone it=m_DataTreeIterator;
    typedef std::pair<int, vtkProp*> LayerMapperPair;
    std::priority_queue<LayerMapperPair> layers;
    int mapperNo = 0;

    for(;it->IsAtEnd()==false;++it)
    {
      mitk::DataTreeNode::Pointer node = it->Get();
      if(node.IsNull())
        continue;
      mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);
      if(mapper.IsNull())
        continue;
      // mapper without a layer property are painted last
      int layer=1;
      node->GetIntProperty("layer", layer, this);

      vtkProp *vtkprop = 0;

      BaseVtkMapper2D* anVtkMapper2D;
      anVtkMapper2D=dynamic_cast<BaseVtkMapper2D*>(mapper.GetPointer());
      if(anVtkMapper2D != NULL)
      {
        anVtkMapper2D->Update(this);
        vtkprop = anVtkMapper2D->GetProp();
      }
      else
      {
        BaseVtkMapper3D* anVtkMapper3D;
        anVtkMapper3D=dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer());
        if(anVtkMapper3D != NULL)
        {
          anVtkMapper3D->Update(this);
          vtkprop = anVtkMapper3D->GetProp();

        }
      }
      if(vtkprop!=NULL)
      {
        // pushing negative layer value, since default sort for
        // priority_queue is lessthan
        layers.push(LayerMapperPair(- (layer<<16) - mapperNo , vtkprop));
        ++mapperNo;
      }
    }

    while (!layers.empty())
    {
      m_VtkRenderer->AddProp(layers.top().second);
      layers.pop();
    }
  }

  if(newRenderer)
  {
    int w=vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();
    m_VtkRenderer->ResetCamera();
    m_VtkRenderer->GetActiveCamera()->Elevation(-90);
    vtkObject::SetGlobalWarningDisplay(w);
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

void mitk::OpenGLRenderer::Update(mitk::DataTreeNode* datatreenode)
{
  if(datatreenode!=NULL)
  {
    mitk::Mapper::Pointer mapper = datatreenode->GetMapper(m_MapperID);
    if(mapper.IsNotNull())
    {
      Mapper2D* mapper2d=dynamic_cast<Mapper2D*>(mapper.GetPointer());
      if(mapper2d != NULL)
      {
        if(GetDisplayGeometry()->IsValid())
        {
          BaseVtkMapper2D* vtkmapper2d=dynamic_cast<BaseVtkMapper2D*>(mapper.GetPointer());
          if(vtkmapper2d != NULL)
          {
            vtkmapper2d->Update(this);
            m_VtkMapperPresent=true;
          }
          else
            mapper2d->Update(this);
          //ImageMapper2D* imagemapper2d=dynamic_cast<ImageMapper2D*>(mapper.GetPointer());
        }
      }
      else
      {
        BaseVtkMapper3D* vtkmapper3d=dynamic_cast<BaseVtkMapper3D*>(mapper.GetPointer());
        if(vtkmapper3d != NULL)
        {
          vtkmapper3d->Update(this);
          m_VtkMapperPresent=true;
        }
      }
    }
  }
}

//##ModelId=3E330D260255
void mitk::OpenGLRenderer::Update()
{
  if(m_DataTreeIterator.IsNull()) return;

  m_VtkMapperPresent=false;

  mitk::DataTreeIteratorClone it=m_DataTreeIterator;

  while(!it->IsAtEnd())
  {
    Update(it->Get());
    ++it;
  }
  Modified();
  m_LastUpdateTime=GetMTime();
}

//##ModelId=3E330D2903CC
void mitk::OpenGLRenderer::Render()
//first render vtkActors, then render mitkMappers
{
  //if we do not have any data, we do nothing else but clearing our window
  if(GetData() == NULL)
  {
    glClear(GL_COLOR_BUFFER_BIT);
    if(((m_VtkRenderer==NULL) && (m_RenderWindow->GetVtkRenderWindow()->GetRenderers()->GetNumberOfItems()>0)) ||
        ((m_VtkRenderer!=NULL) && (m_RenderWindow->GetVtkRenderWindow()->GetRenderers()->GetNumberOfItems()>1)) )
      m_RenderWindow->GetVtkRenderWindow()->MitkRender();
    else
      if(m_VtkMapperPresent)
      {
        //    m_RenderWindow->GetVtkRenderWindow()->MitkRender();
      }
      else
        m_RenderWindow->SwapBuffers();

    return;
  }

  //has the data tree been changed?
  if(dynamic_cast<mitk::DataTree*>(GetData()->GetTree()) == NULL ) return;
  if(m_LastUpdateVtkActorsTime < static_cast<mitk::DataTree*>(GetData()->GetTree())->GetMTime() )
  {
    //yes: update vtk-actors
    UpdateIncludingVtkActors();
  }
  else
    //has anything else changed (geometry to display, etc.)?
    if ( m_LastUpdateTime < GetMTime() || m_LastUpdateTime < GetDisplayGeometry()->GetMTime() )
    {
      //std::cout << "OpenGLRenderer calling its update..." << std::endl;
      Update();
    }
    else
      if(m_MapperID>=2)
      { //@todo in 3D mode wird sonst nix geupdated, da z.Z. weder camera noch Änderung des Baums beachtet wird!!!
        Update();
      }

  glClear(GL_COLOR_BUFFER_BIT);

  //PlaneGeometry* myPlaneGeom =
  //  dynamic_cast<PlaneGeometry *>((mitk::Geometry2D*)(GetCurrentWorldGeometry2D()));

  glViewport (0, 0, m_Size[0], m_Size[1]);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0.0, m_Size[0], 0.0, m_Size[1] );
  glMatrixMode( GL_MODELVIEW );

  //------------------
  //rendering VTK-Mappers if present
  if(m_VtkMapperPresent)
  {
    mitk::VtkStencilRenderWindow *stencilRenderWindow = dynamic_cast<mitk::VtkStencilRenderWindow *>(m_RenderWindow->GetVtkRenderWindow());
    if (stencilRenderWindow)
      stencilRenderWindow->SetFinishRendering(false);

    //start vtk render process with the updated scenegraph
    m_RenderWindow->GetVtkRenderWindow()->MitkRender();
  }

  //------------------
  //preparing and gaining information about 2D rendering for OpenGL
  if(GetDisplayGeometry()->IsValid())
  {
    mitk::DataTreeIteratorClone it = m_DataTreeIterator;
    typedef std::pair<int, GLMapper2D*> LayerMapperPair;
    std::priority_queue<LayerMapperPair> layers;
    int mapperNo = 0;

    for(;it->IsAtEnd()==false;++it)
    {
      mitk::DataTreeNode::Pointer node = it->Get();
      if(node.IsNull())
        continue;
      mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);
      if(mapper.IsNull())
        continue;
      GLMapper2D* mapper2d=dynamic_cast<GLMapper2D*>(mapper.GetPointer());
      if(mapper2d!=NULL)
      {
        // mapper without a layer property are painted first
        int layer=-1;
        node->GetIntProperty("layer", layer, this);
        // pushing negative layer value, since default sort for
        // priority_queue is lessthan
        layers.push(LayerMapperPair(- (layer<<16) - mapperNo ,mapper2d));
        mapperNo++;
      }
    }

    //go through the generated list and let the sorted mappers paint
    while (!layers.empty())
    {
      layers.top().second->Paint(this);
      layers.pop();
    }
  }

  //swap buffers
  if (m_VtkMapperPresent)
  {
    mitk::VtkStencilRenderWindow *stencilRenderWindow = dynamic_cast<mitk::VtkStencilRenderWindow *>(m_RenderWindow->GetVtkRenderWindow());
    if (stencilRenderWindow)
      stencilRenderWindow->SetFinishRendering(true);//set an internal flag to only swap buffers when ready with rendering

    m_RenderWindow->GetVtkRenderWindow()->CopyResultFrame();
  }
  else
    m_RenderWindow->SwapBuffers();
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

  assert(renderwindow);
  /**@todo SetNumberOfLayers commented out, because otherwise the backface of the planes are not shown (only, when a light is added).
  * But we need SetNumberOfLayers(2) later, when we want to prevent vtk to clear the widget before it renders (i.e., when we render something in the scene before vtk).
  */
  //m_RenderWindow->GetVtkRenderWindow()->SetNumberOfLayers(2);

  if(m_CameraController != NULL)
  {
    VtkInteractorCameraController* vicc=dynamic_cast<VtkInteractorCameraController*>(m_CameraController.GetPointer());
    if(vicc!=NULL)
    {
      vicc->SetRenderer(this);
      vicc->GetVtkInteractor()->Disable();
    }
  }
  m_LastUpdateTime = 0;
  m_LastUpdateVtkActorsTime = 0;
  //we should disable vtk doublebuffering, but then it doesn't work
  //m_RenderWindow->GetVtkRenderWindow()->SwapBuffersOff();
  if( this->m_VtkRenderer !=NULL )
    m_RenderWindow->GetVtkRenderWindow()->AddRenderer( this->m_VtkRenderer );
}

/*!
\brief Destructs the OpenGLRenderer.
*/
//##ModelId=3E33ECF301B7
mitk::OpenGLRenderer::~OpenGLRenderer()
{
  if(m_VtkRenderer!=NULL)
  {
    m_RenderWindow->GetVtkRenderWindow()->RemoveRenderer(m_VtkRenderer);
    m_RenderWindow->GetVtkRenderWindow()->SetInteractor(NULL);
    m_CameraController = NULL;
    //VtkInteractorCameraController* vicc=dynamic_cast<VtkInteractorCameraController*>(m_CameraController.GetPointer());
    //if(vicc!=NULL)
    //{
    //  vicc->SetRenderWindow(NULL);
    //  vicc->GetVtkInteractor()->Disable();
    //}
    m_VtkRenderer->Delete();
  }
  else
    m_CameraController = NULL;
}

/*!
\brief Initialize the OpenGL Window
*/
//##ModelId=3E33145B0096
void mitk::OpenGLRenderer::Initialize( )
{

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glColor3f(1.0, 0.0, 0.0);

}

/*!
\brief Resize the OpenGL Window
*/
//##ModelId=3E33145B00D2
void mitk::OpenGLRenderer::Resize(int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D( 0.0, w, 0.0, h );
  glMatrixMode( GL_MODELVIEW );

  BaseRenderer::Resize(w, h);

  //GetVtkRenderWindow()->SetSize(w,h); // this was originally done by VtkInteractor via
  //                                    // m_CameraController->Resize(...) in BaseRenderer,
  //                                    // but bug #32 forced us to put it here (vtkSizeBug)

  Update();
}

/*!
\brief Render the scene
*/
//##ModelId=3E33145B005A
void mitk::OpenGLRenderer::Paint( )
{

  //  glFlush();
  //  m_RenderWindow->swapBuffers();
  Render();
}

//##ModelId=3E3799420227
void mitk::OpenGLRenderer::InitSize(int w, int h)
{
  m_RenderWindow->SetSize(w,h);
  Superclass::InitSize(w, h);
  Modified();
  Update();
  if(m_VtkRenderer!=NULL)
  {
    int w=vtkObject::GetGlobalWarningDisplay();
    vtkObject::GlobalWarningDisplayOff();
    m_VtkRenderer->ResetCamera();
    vtkObject::SetGlobalWarningDisplay(w);
  }
}

//##ModelId=3EF59AD20235
void mitk::OpenGLRenderer::SetMapperID(const MapperSlotId mapperId)
{
  Superclass::SetMapperID(mapperId);
  //ensure that UpdateIncludingVtkActors() is called at next
  //call of Render():
  m_LastUpdateVtkActorsTime = 0;
}

//##ModelId=3EF162760271
void mitk::OpenGLRenderer::MakeCurrent()
{
  if(m_RenderWindow!=NULL)
  {
    m_RenderWindow->MakeCurrent();
  }
}

void mitk::OpenGLRenderer::DataChangedEvent(const itk::Object *caller, const itk::EventObject &event)
{
  if(m_RenderWindow!=NULL)
  {
    m_RenderWindow->Update();
  }
}

void mitk::OpenGLRenderer::PickWorldPoint(const mitk::Point2D& displayPoint, mitk::Point3D& worldPoint) const
{
  if(m_VtkMapperPresent)
  {
    //m_WorldPointPicker->SetTolerance (0.0001);
    m_WorldPointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
    vtk2itk(m_WorldPointPicker->GetPickPosition(), worldPoint);
  }
  else
    Superclass::PickWorldPoint(displayPoint, worldPoint);
}

void mitk::OpenGLRenderer::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os,indent);
  os << indent << " VtkMapperPresent: " << m_VtkMapperPresent << std::endl;
  os << indent << " VtkRenderer: " << m_VtkRenderer << std::endl;
  os << indent << " LastUpdateVtkActorsTime: " << m_LastUpdateVtkActorsTime << std::endl;
}

