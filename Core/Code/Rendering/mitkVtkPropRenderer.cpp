/*=========================================================================
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkVtkPropRenderer.h"

// MAPPERS
#include "mitkMapper.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkVtkMapper2D.h"
#include "mitkVtkMapper3D.h"
#include "mitkGeometry2DDataVtkMapper3D.h"
#include "mitkPointSetGLMapper2D.h"

#include "mitkImageSliceSelector.h"
#include "mitkRenderingManager.h"
#include "mitkGL.h"
#include "mitkGeometry3D.h"
#include "mitkDisplayGeometry.h"
#include "mitkLevelWindow.h"
#include "mitkCameraController.h"
#include "mitkVtkInteractorCameraController.h"
#include "mitkPlaneGeometry.h"
#include "mitkProperties.h"
#include "mitkSurface.h"
#include "mitkNodePredicateDataType.h"
#include "mitkVtkInteractorStyle.h"

// VTK
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkLight.h>
#include <vtkLightKit.h>
#include <vtkRenderWindow.h>
#include <vtkLinearTransform.h>
#include <vtkCamera.h>
#include <vtkWorldPointPicker.h>
#include <vtkPointPicker.h>
#include <vtkCellPicker.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkProp.h>
#include <vtkAssemblyPath.h>
#include <vtkAssemblyNode.h>
#include <vtkMapper.h>
#include <vtkSmartPointer.h>
#include <vtkTransform.h>
#include <vtkInteractorStyleTrackballCamera.h>


mitk::VtkPropRenderer::VtkPropRenderer( const char* name, vtkRenderWindow * renWin, mitk::RenderingManager* rm )
  : BaseRenderer(name,renWin, rm), 
  m_VtkMapperPresent(false), 
  m_NewRenderer(true),
  m_CameraInitializedForMapperID(0)
{
  didCount=false;

  m_WorldPointPicker = vtkWorldPointPicker::New();

  m_PointPicker = vtkPointPicker::New();
  m_PointPicker->SetTolerance( 0.0025 );

  m_CellPicker = vtkCellPicker::New();
  m_CellPicker->SetTolerance( 0.0025 );

  mitk::Geometry2DDataVtkMapper3D::Pointer geometryMapper = mitk::Geometry2DDataVtkMapper3D::New();
  m_CurrentWorldGeometry2DMapper = geometryMapper;
  m_CurrentWorldGeometry2DNode->SetMapper(2, geometryMapper);

  m_LightKit = vtkLightKit::New();
  m_LightKit->AddLightsToRenderer(m_VtkRenderer);
  m_PickingMode = WorldPointPicking;

  m_TextRenderer = vtkRenderer::New();
  m_TextRenderer->SetRenderWindow(renWin);
  m_TextRenderer->SetInteractive(0);
  m_TextRenderer->SetErase(0);
}

/*!
\brief Destructs the VtkPropRenderer.
*/
mitk::VtkPropRenderer::~VtkPropRenderer()
{
  // Workaround for GLDisplayList Bug
  {
    m_MapperID=0;
    checkState();
  }

  if (m_LightKit != NULL)
    m_LightKit->Delete();

  if (m_VtkRenderer!=NULL)
  {
    m_CameraController = NULL;

    m_VtkRenderer->Delete();

    m_VtkRenderer = NULL;
  }
  else
    m_CameraController = NULL;

  if (m_WorldPointPicker != NULL)
    m_WorldPointPicker->Delete();
  if (m_PointPicker != NULL)
    m_PointPicker->Delete();
  if (m_CellPicker != NULL)
    m_CellPicker->Delete();
  if (m_TextRenderer != NULL)
    m_TextRenderer->Delete();
}


void mitk::VtkPropRenderer::SetDataStorage(  mitk::DataStorage* storage  )
{
  if ( storage == NULL )
    return;

  BaseRenderer::SetDataStorage(storage);

  static_cast<mitk::Geometry2DDataVtkMapper3D*>(m_CurrentWorldGeometry2DMapper.GetPointer())->SetDataStorageForTexture( m_DataStorage.GetPointer() );

  // Compute the geometry from the current data tree bounds and set it as world geometry
  this->SetWorldGeometryToDataStorageBounds();
}


bool mitk::VtkPropRenderer::SetWorldGeometryToDataStorageBounds()
{
  if ( m_DataStorage.IsNull() )
    return false;

  //initialize world geometry
  mitk::TimeSlicedGeometry::Pointer geometry = m_DataStorage->ComputeVisibleBoundingGeometry3D( NULL, "includeInBoundingBox" );

  if ( geometry.IsNull() )
    return false;

  this->SetWorldGeometry(geometry);
  //this->GetDisplayGeometry()->SetSizeInDisplayUnits( this->m_TextRenderer->GetRenderWindow()->GetSize()[0], this->m_TextRenderer->GetRenderWindow()->GetSize()[1] );
  this->GetDisplayGeometry()->Fit();
  this->GetVtkRenderer()->ResetCamera();
  this->Modified();
  return true;
}


/*!
\brief 

Called by the vtkMitkRenderProp in order to start MITK rendering process.
*/
int mitk::VtkPropRenderer::Render(mitk::VtkPropRenderer::RenderType type)
{

  // Do we have objects to render?
  if ( this->GetEmptyWorldGeometry()) 
    return 0;

  if ( m_DataStorage.IsNull())
    return 0;

  // Update mappers and prepare mapper queue
  if (type == VtkPropRenderer::Opaque)
    this->PrepareMapperQueue();
  
  //go through the generated list and let the sorted mappers paint
  bool lastVtkBased = true;
  //bool sthVtkBased = false;
  
  for(MappersMapType::iterator it = m_MappersMap.begin(); it != m_MappersMap.end(); it++)
  {
    Mapper * mapper = (*it).second;
    if((mapper->IsVtkBased() == true) )
    {
      //sthVtkBased = true;
      mitk::VtkMapper3D::Pointer vtkMapper = dynamic_cast<mitk::VtkMapper3D*>(mapper);
      if(vtkMapper)
      {
        vtkMapper->GetVtkProp(this)->SetAllocatedRenderTime(5000,GetVtkRenderer()); //B/ ToDo: rendering time calculation
        //vtkMapper->GetVtkProp(this)->PokeMatrix(NULL); //B/ ToDo ??: VtkUserTransform
      }
      if(lastVtkBased == false)
      {
        Disable2DOpenGL();
        lastVtkBased = true;
      }
    }
    else
      if((mapper->IsVtkBased() == false) && (lastVtkBased == true))
      {
      Enable2DOpenGL();
      lastVtkBased = false;
    }

    //Workarround for bug GL_TEXTURE_2D (bug #8188)
    GLboolean mode;
    GLenum bit = GL_TEXTURE_2D;
    GLfloat lineWidth;
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);
    glGetBooleanv(bit, &mode);

    switch(type)
    {
    case mitk::VtkPropRenderer::Opaque: mapper->MitkRenderOpaqueGeometry(this); break;
    case mitk::VtkPropRenderer::Translucent: mapper->MitkRenderTranslucentGeometry(this); break;
    case mitk::VtkPropRenderer::Overlay:       mapper->MitkRenderOverlay(this); break;
    case mitk::VtkPropRenderer::Volumetric:    mapper->MitkRenderVolumetricGeometry(this); break;
    }
    if(mode)
      glEnable(bit);
    else
      glDisable(bit);

    glLineWidth(lineWidth);
    //end Workarround for bug GL_TEXTURE_2D (bug #8188)

  }
  
  if (lastVtkBased == false)
    Disable2DOpenGL();

  // Render text
  if (type == VtkPropRenderer::Overlay)
  {
    if (m_TextCollection.size() > 0)
    {
      for (TextMapType::iterator it = m_TextCollection.begin(); it != m_TextCollection.end() ; it++)
        m_TextRenderer->AddViewProp((*it).second);
      m_TextRenderer->Render();
    }
  }
  return 1;
}

/*!
\brief PrepareMapperQueue iterates the datatree

PrepareMapperQueue iterates the datatree in order to find mappers which shall be rendered. Also, it sortes the mappers wrt to their layer.
*/
void mitk::VtkPropRenderer::PrepareMapperQueue()
{
  // variable for counting LOD-enabled mappers
  m_NumberOfVisibleLODEnabledMappers = 0;

  // Do we have to update the mappers ?
  if ( m_LastUpdateTime < GetMTime() || m_LastUpdateTime < GetDisplayGeometry()->GetMTime() ) {
    Update();
  }
  else if (m_MapperID>=1 && m_MapperID < 6)
    Update();

  // remove all text properties before mappers will add new ones
  m_TextRenderer->RemoveAllViewProps();

  for ( unsigned int i=0; i<m_TextCollection.size(); i++ )
  {
    m_TextCollection[i]->Delete();
  }
  m_TextCollection.clear();

  // clear priority_queue
  m_MappersMap.clear();

  int mapperNo = 0;

  //DataStorage
  if( m_DataStorage.IsNull() )
    return;

  DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();

  for (DataStorage::SetOfObjects::ConstIterator it = allObjects->Begin();  it != allObjects->End(); ++it)
  {
    DataNode::Pointer node = it->Value();
    if ( node.IsNull() )
      continue;
    mitk::Mapper::Pointer mapper = node->GetMapper(m_MapperID);

    if ( mapper.IsNull() )
      continue;

    // The information about LOD-enabled mappers is required by RenderingManager
    if ( mapper->IsLODEnabled( this ) && mapper->IsVisible( this ) )
    {
      ++m_NumberOfVisibleLODEnabledMappers;
    }
    // mapper without a layer property get layer number 1
    int layer = 1;
    node->GetIntProperty("layer", layer, this);
    int nr = (layer<<16) + mapperNo;
    m_MappersMap.insert( std::pair< int, Mapper * >( nr, mapper ) );
    mapperNo++;
  }  
}

/*!
\brief 

Enable2DOpenGL() and Disable2DOpenGL() are used to switch between 2D rendering (orthographic projection) and 3D rendering (perspective projection)
*/
void mitk::VtkPropRenderer::Enable2DOpenGL()
{
  GLint iViewport[4];  

  // Get a copy of the viewport  
  glGetIntegerv( GL_VIEWPORT, iViewport );  

  // Save a copy of the projection matrix so that we can restore it  
  // when it's time to do 3D rendering again.  
  glMatrixMode( GL_PROJECTION );  
  glPushMatrix();
  glLoadIdentity();  

  // Set up the orthographic projection  
  glOrtho( 
      iViewport[0], iViewport[0]+iViewport[2],
      iViewport[1], iViewport[1]+iViewport[3],
      -1.0, 1.0
      );
  glMatrixMode( GL_MODELVIEW );  
  glPushMatrix();  
  glLoadIdentity();

  // Make sure depth testing and lighting are disabled for 2D rendering until  
  // we are finished rendering in 2D  
  glPushAttrib( GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT );
  glDisable( GL_DEPTH_TEST );  
  glDisable( GL_LIGHTING );
}

/*!
\brief Initialize the VtkPropRenderer

Enable2DOpenGL() and Disable2DOpenGL() are used to switch between 2D rendering (orthographic projection) and 3D rendering (perspective projection)
*/
void mitk::VtkPropRenderer::Disable2DOpenGL()
{
  glPopAttrib();
  glMatrixMode( GL_PROJECTION );  
  glPopMatrix();  
  glMatrixMode( GL_MODELVIEW );  
  glPopMatrix(); 
}

void mitk::VtkPropRenderer::Update(mitk::DataNode* datatreenode)
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
          VtkMapper2D* vtkmapper2d=dynamic_cast<VtkMapper2D*>(mapper.GetPointer());
          if(vtkmapper2d != NULL)
          {
            vtkmapper2d->Update(this);
            m_VtkMapperPresent=true;
          }
          else
            mapper2d->Update(this);
        }
      }
      else
      {
        VtkMapper3D* vtkmapper3d=dynamic_cast<VtkMapper3D*>(mapper.GetPointer());
        if(vtkmapper3d != NULL)
        {
          vtkmapper3d->Update(this);
          vtkmapper3d->UpdateVtkTransform(this);
          m_VtkMapperPresent=true;
        }
      }
    }
  }
}


void mitk::VtkPropRenderer::Update()
{
  if( m_DataStorage.IsNull() )
    return;

  m_VtkMapperPresent = false;
  mitk::DataStorage::SetOfObjects::ConstPointer all = m_DataStorage->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator it = all->Begin(); it != all->End(); ++it)
    Update(it->Value());

  Modified();
  m_LastUpdateTime = GetMTime();
}


/*!
\brief

This method is called from the two Constructors
*/
void mitk::VtkPropRenderer::InitRenderer(vtkRenderWindow* renderWindow)
{
  BaseRenderer::InitRenderer(renderWindow);

  if(renderWindow == NULL)
  {
    m_InitNeeded = false;
    m_ResizeNeeded = false;
    return;
  }

  m_InitNeeded = true;
  m_ResizeNeeded = true;

  m_LastUpdateTime = 0;

}


/*!
\brief Resize the OpenGL Window
*/
void mitk::VtkPropRenderer::Resize(int w, int h)
{
  BaseRenderer::Resize(w, h);
  m_RenderingManager->RequestUpdate(this->GetRenderWindow());
}


void mitk::VtkPropRenderer::InitSize(int w, int h)
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


void mitk::VtkPropRenderer::SetMapperID(const MapperSlotId mapperId)
{
  if(m_MapperID != mapperId)
    Superclass::SetMapperID(mapperId);

  // Workaround for GL Displaylist Bug
  checkState();
}


/*!
\brief Activates the current renderwindow.
*/
void mitk::VtkPropRenderer::MakeCurrent()
{
  if(m_RenderWindow!=NULL)
    m_RenderWindow->MakeCurrent();
}


void mitk::VtkPropRenderer::PickWorldPoint(const mitk::Point2D& displayPoint, mitk::Point3D& worldPoint) const
{
  if(m_VtkMapperPresent)
  {
    //m_WorldPointPicker->SetTolerance (0.0001);
    switch ( m_PickingMode )
    {
    case (WorldPointPicking) :
      {
        m_WorldPointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
        vtk2itk(m_WorldPointPicker->GetPickPosition(), worldPoint);
        break;
      }
    case (PointPicking) :
      {
        // create a new vtkRenderer
        // give it all necessary information (camera position, etc.)
        // get all surfaces from datastorage, get actors from them
        // add all those actors to the new renderer
        // give this new renderer to pointpicker
        /*
       vtkRenderer* pickingRenderer = vtkRenderer::New();
       pickingRenderer->SetActiveCamera( );

       DataStorage* dataStorage = m_DataStorage;
       TNodePredicateDataType<Surface> isSurface;

       DataStorage::SetOfObjects::ConstPointer allSurfaces = dataStorage->GetSubset( isSurface );
       MITK_INFO << "in picking: got " << allSurfaces->size() << " surfaces." << std::endl;

       for (DataStorage::SetOfObjects::const_iterator iter = allSurfaces->begin();
            iter != allSurfaces->end();
            ++iter)
       {
         const DataNode* currentNode = *iter;
         VtkMapper3D* baseVtkMapper3D = dynamic_cast<VtkMapper3D*>( currentNode->GetMapper( BaseRenderer::Standard3D ) );
         if ( baseVtkMapper3D )
         {
           vtkActor* actor = dynamic_cast<vtkActor*>( baseVtkMapper3D->GetViewProp() );
           if (actor)
           {
             MITK_INFO << "a" << std::flush;
             pickingRenderer->AddActor( actor );
           }
         }
       }

       MITK_INFO << ";" << std::endl;
       */
        m_PointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
        vtk2itk(m_PointPicker->GetPickPosition(), worldPoint);
        break;
      }
    }
  }
  else
  {
    Superclass::PickWorldPoint(displayPoint, worldPoint);
  }
}

mitk::DataNode *
    mitk::VtkPropRenderer::PickObject( const Point2D &displayPosition, Point3D &worldPosition ) const
{
  if ( m_VtkMapperPresent )
  {
    m_CellPicker->InitializePickList();

    // Iterate over all DataStorage objects to determine all vtkProps intended
    // for picking
    DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();
    for ( DataStorage::SetOfObjects::ConstIterator it = allObjects->Begin();
    it != allObjects->End();
    ++it )
    {
      DataNode *node = it->Value();
      if ( node == NULL )
        continue;

      bool pickable = false;
      node->GetBoolProperty( "pickable", pickable );
      if ( !pickable )
        continue;

      VtkMapper3D *mapper = dynamic_cast< VtkMapper3D * >
                            ( node->GetMapper( m_MapperID ) );
      if ( mapper == NULL )
        continue;

      vtkProp *prop = mapper->GetVtkProp( (mitk::BaseRenderer *)this );
      if ( prop == NULL )
        continue;

      m_CellPicker->AddPickList( prop );
    }  


    // Do the picking and retrieve the picked vtkProp (if any)
    m_CellPicker->PickFromListOn();
    m_CellPicker->Pick( displayPosition[0], displayPosition[1], 0.0, m_VtkRenderer );
    m_CellPicker->PickFromListOff();

    vtk2itk( m_CellPicker->GetPickPosition(), worldPosition );
    vtkProp *prop = m_CellPicker->GetViewProp();

    if ( prop == NULL )
    {
      return NULL;
    }

    // Iterate over all DataStorage objects to determine if the retrieved
    // vtkProp is owned by any associated mapper.
    for ( DataStorage::SetOfObjects::ConstIterator it = allObjects->Begin();
    it != allObjects->End();
    ++it)
    {
      DataNode::Pointer node = it->Value();
      if ( node.IsNull() )
        continue;

      mitk::Mapper::Pointer mapper = node->GetMapper( m_MapperID );
      if ( mapper.IsNull() )
        continue;

      if ( mapper->HasVtkProp( prop, const_cast< mitk::VtkPropRenderer * >( this ) ) )
      {
        return node;
      }
    }  

    return NULL;
  }
  else
  {
    return Superclass::PickObject( displayPosition, worldPosition ); 
  }
};



/*!
\brief Writes some 2D text as overlay. Function returns an unique int Text_ID for each call, which can be used via the GetTextLabelProperty(int text_id) function
in order to get a vtkTextProperty. This property enables the setup of font, font size, etc.
*/
int mitk::VtkPropRenderer::WriteSimpleText(std::string text, double posX, double posY, double color1, double color2, double color3)
{
  if(text.size() > 0)
  {
    vtkTextActor* textActor = vtkTextActor::New();

    textActor->SetPosition(posX,posY);
    textActor->SetInput(text.c_str());
    textActor->GetTextProperty()->SetColor(color1, color2, color3); //TODO: Read color from node property
    int text_id = m_TextCollection.size();
    m_TextCollection.insert(TextMapType::value_type(text_id,textActor));
    return text_id;
  }
  return -1;
}


/*!
\brief Can be used in order to get a vtkTextProperty for a specific text_id. This property enables the setup of font, font size, etc.
*/
vtkTextProperty* mitk::VtkPropRenderer::GetTextLabelProperty(int text_id)
{
  return this->m_TextCollection[text_id]->GetTextProperty();
}


void mitk::VtkPropRenderer::InitPathTraversal()
{
  if (m_DataStorage.IsNotNull())
  {
    m_PickingObjects = m_DataStorage->GetAll();
    m_PickingObjectsIterator = m_PickingObjects->begin();
  }
}


vtkAssemblyPath* mitk::VtkPropRenderer::GetNextPath()
{
  if (m_DataStorage.IsNull() )
  {
    return NULL;
  }

  if ( m_PickingObjectsIterator == m_PickingObjects->end() )
  {
    return NULL;
  }

  vtkAssemblyPath* returnPath = vtkAssemblyPath::New();
  //returnPath->Register(NULL);

  bool success = false;

  while (!success)
  {
    // loop until AddNode can be called successfully
    const DataNode* node = *m_PickingObjectsIterator;
    if (node)
    {
      Mapper* mapper = node->GetMapper( BaseRenderer::Standard3D );
      if (mapper)
      {
        VtkMapper3D* vtkmapper = dynamic_cast<VtkMapper3D*>( mapper );
        if (vtkmapper)
        {
          vtkProp* prop = vtkmapper->GetVtkProp(this);
          if ( prop && prop->GetVisibility() )
          {
            // add to assembly path
            returnPath->AddNode( prop, prop->GetMatrix() );
            success = true;
          }
        }
      }
    }

    ++m_PickingObjectsIterator;

    if ( m_PickingObjectsIterator == m_PickingObjects->end() ) break;
  }

  if ( success )
  {
    return returnPath;
  }
  else
  {
    return NULL;
  }
}


void mitk::VtkPropRenderer::ReleaseGraphicsResources(vtkWindow *renWin)
{
  if( m_DataStorage.IsNull() )
    return;

  DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();
  for (DataStorage::SetOfObjects::const_iterator iter = allObjects->begin(); iter != allObjects->end(); ++iter)
  {
    DataNode::Pointer node = *iter;
    if ( node.IsNull() )
      continue;

    Mapper::Pointer mapper = node->GetMapper(m_MapperID);
    if(mapper.IsNotNull())
      mapper->ReleaseGraphicsResources(renWin);
  } 
}


const vtkWorldPointPicker *mitk::VtkPropRenderer::GetWorldPointPicker() const
{
  return m_WorldPointPicker;
}


const vtkPointPicker *mitk::VtkPropRenderer::GetPointPicker() const
{
  return m_PointPicker;
}


const vtkCellPicker *mitk::VtkPropRenderer::GetCellPicker() const
{
  return m_CellPicker;
}


mitk::VtkPropRenderer::MappersMapType mitk::VtkPropRenderer::GetMappersMap() const
{
  return m_MappersMap;
}


// Workaround for GL Displaylist bug

static int glWorkAroundGlobalCount = 0;

bool mitk::VtkPropRenderer::useImmediateModeRendering()
{
  return glWorkAroundGlobalCount>1;
}

void mitk::VtkPropRenderer::checkState()
{
  if (m_MapperID == Standard3D)
  {
    if (!didCount)
    {
      didCount = true;
      glWorkAroundGlobalCount++;
      
      if (glWorkAroundGlobalCount == 2)
      {
        MITK_INFO << "Multiple 3D Renderwindows active...: turning Immediate Rendering ON for legacy mappers";
        //          vtkMapper::GlobalImmediateModeRenderingOn();
      }
      //MITK_INFO << "GLOBAL 3D INCREASE " << glWorkAroundGlobalCount << "\n";
      
    }
  }
  else
  {
    if(didCount)
    {
      didCount=false;
      glWorkAroundGlobalCount--;
      if(glWorkAroundGlobalCount==1)
      {
        MITK_INFO << "Single 3D Renderwindow active...: turning Immediate Rendering OFF for legacy mappers";
        //        vtkMapper::GlobalImmediateModeRenderingOff();
      }
      //MITK_INFO << "GLOBAL 3D DECREASE " << glWorkAroundGlobalCount << "\n";
    }
  }
}

//### Contains all methods which are neceassry before each VTK Render() call
void mitk::VtkPropRenderer::PrepareRender()
{
  if ( this->GetMapperID() != m_CameraInitializedForMapperID )
  {
    Initialize2DvtkCamera(); //Set parallel projection etc.
  }

  AdjustCameraToScene(); //Prepare camera for 2D render windows
}

bool mitk::VtkPropRenderer::Initialize2DvtkCamera()
{
  if ( this->GetMapperID() == Standard3D )
  {
    //activate parallel projection for 2D
    this->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(false);
    this->GetRenderWindow()->GetInteractor()->SetInteractorStyle( vtkInteractorStyleTrackballCamera::New() );
    m_CameraInitializedForMapperID = Standard3D;
  }
  else if( this->GetMapperID() == Standard2D)
  {
    //activate parallel projection for 2D
    this->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(true);
    //turn the light out in the scene in order to render correct grey values.
    //TODO Implement a property for light in the 2D render windows (in another method)
    this->GetVtkRenderer()->RemoveAllLights();

    this->GetRenderWindow()->GetInteractor()->SetInteractorStyle( mitkVtkInteractorStyle::New() );

    m_CameraInitializedForMapperID = Standard2D;
  }
  return true;
}

void mitk::VtkPropRenderer::AdjustCameraToScene(){
  if(this->GetMapperID() == Standard2D)
  {
    const mitk::DisplayGeometry* displayGeometry = this->GetDisplayGeometry();

    double objectHeightInMM = this->GetCurrentWorldGeometry2D()->GetExtentInMM(1);//the height of the current object slice in mm
    double displayHeightInMM = displayGeometry->GetSizeInMM()[1]; //the display height in mm (gets smaller when you zoom in)
    double zoomFactor = objectHeightInMM/displayHeightInMM; //displayGeometry->GetScaleFactorMMPerDisplayUnit()
    //determine how much of the object can be displayed

    Vector2D displayGeometryOriginInMM = displayGeometry->GetOriginInMM();  //top left of the render window (Origin)
    Vector2D displayGeometryCenterInMM = displayGeometryOriginInMM + displayGeometry->GetSizeInMM()*0.5; //center of the render window: (Origin + Size/2)

    //Scale the rendered object:
    //The image is scaled by a single factor, because in an orthographic projection sizes
    //are preserved (so you cannot scale X and Y axis with different parameters). The
    //parameter sets the size of the total display-volume. If you set this to the image
    //height, the image plus a border with the size of the image will be rendered.
    //Therefore, the size is imageHeightInMM / 2.
    this->GetVtkRenderer()->GetActiveCamera()->SetParallelScale(objectHeightInMM*0.5 );
    //zooming with the factor calculated by dividing displayHeight through imegeHeight. The factor is inverse, because the VTK zoom method is working inversely.
    this->GetVtkRenderer()->GetActiveCamera()->Zoom(zoomFactor);

    //the center of the view-plane
    double viewPlaneCenter[3];
    viewPlaneCenter[0] = displayGeometryCenterInMM[0];
    viewPlaneCenter[1] = displayGeometryCenterInMM[1];
    viewPlaneCenter[2] = 0.0; //the view-plane is located in the XY-plane with Z=0.0

    //define which direction is "up" for the ciamera (like default for vtk (0.0, 1.0, 0.0)
    double cameraUp[3];
    cameraUp[0] = 0.0;
    cameraUp[1] = 1.0;
    cameraUp[2] = 0.0;

    //the position of the camera (center[0], center[1], 900000)
    double cameraPosition[3];
    cameraPosition[0] = viewPlaneCenter[0];
    cameraPosition[1] = viewPlaneCenter[1];
    cameraPosition[2] = 900000.0; //Reason for 900000: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.

    //set the camera corresponding to the textured plane
    vtkSmartPointer<vtkCamera> camera = this->GetVtkRenderer()->GetActiveCamera();
    if (camera)
    {
      camera->SetPosition( cameraPosition ); //set the camera position on the textured plane normal (in our case this is the view plane normal)
      camera->SetFocalPoint( viewPlaneCenter ); //set the focal point to the center of the textured plane
      camera->SetViewUp( cameraUp ); //set the view-up for the camera
      //      double distance = sqrt((cameraPosition[2]-viewPlaneCenter[2])*(cameraPosition[2]-viewPlaneCenter[2]));
      //      camera->SetClippingRange(distance-50, distance+50); //Reason for huge range: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.
      camera->SetClippingRange(0.1, 1000000); //Reason for huge range: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.
    }

    const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( this->GetCurrentWorldGeometry2D() );
    if ( planeGeometry != NULL )
    {
      //Transform the camera to the current position (transveral, coronal and saggital plane).
      //This is necessary, because the SetUserTransform() method does not manipulate the vtkCamera.
      //(Without not all three planes would be visible).
      vtkSmartPointer<vtkTransform> trans = vtkSmartPointer<vtkTransform>::New();
      vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
      Point3D origin;
      Vector3D right, bottom, normal;

      origin = planeGeometry->GetOrigin();
      right  = planeGeometry->GetAxisVector( 0 ); // right = Extent of Image in mm (worldspace)
      bottom = planeGeometry->GetAxisVector( 1 );
      normal = planeGeometry->GetNormal();

      right.Normalize();
      bottom.Normalize();
      normal.Normalize();

      matrix->SetElement(0, 0, right[0]);
      matrix->SetElement(1, 0, right[1]);
      matrix->SetElement(2, 0, right[2]);
      matrix->SetElement(0, 1, bottom[0]);
      matrix->SetElement(1, 1, bottom[1]);
      matrix->SetElement(2, 1, bottom[2]);
      matrix->SetElement(0, 2, normal[0]);
      matrix->SetElement(1, 2, normal[1]);
      matrix->SetElement(2, 2, normal[2]);
      matrix->SetElement(0, 3, origin[0]);
      matrix->SetElement(1, 3, origin[1]);
      matrix->SetElement(2, 3, origin[2]);
      matrix->SetElement(3, 0, 0.0);
      matrix->SetElement(3, 1, 0.0);
      matrix->SetElement(3, 2, 0.0);
      matrix->SetElement(3, 3, 1.0);

      trans->SetMatrix(matrix);
      //Transform the camera to the current position (transveral, coronal and saggital plane).
      this->GetVtkRenderer()->GetActiveCamera()->ApplyTransform(trans);
    }
  }
}
