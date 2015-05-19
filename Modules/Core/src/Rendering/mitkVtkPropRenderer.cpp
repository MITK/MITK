/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkVtkPropRenderer.h"

// MAPPERS
#include "mitkMapper.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkVtkMapper.h"
#include "mitkPlaneGeometryDataVtkMapper3D.h"

#include <mitkImageSliceSelector.h>
#include <mitkRenderingManager.h>
#include <mitkGeometry3D.h>
#include <mitkDisplayGeometry.h>
#include <mitkLevelWindow.h>
#include <mitkCameraController.h>
#include <mitkVtkInteractorCameraController.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkSurface.h>
#include <mitkNodePredicateDataType.h>
#include <mitkVtkInteractorStyle.h>
#include <mitkAbstractTransformGeometry.h>

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

mitk::VtkPropRenderer::VtkPropRenderer( const char* name, vtkRenderWindow * renWin, mitk::RenderingManager* rm, mitk::BaseRenderer::RenderingMode::Type renderingMode )
  : BaseRenderer(name,renWin, rm, renderingMode ),
    m_CameraInitializedForMapperID(0)
{
  didCount=false;

  m_WorldPointPicker = vtkWorldPointPicker::New();

  m_PointPicker = vtkPointPicker::New();
  m_PointPicker->SetTolerance( 0.0025 );

  m_CellPicker = vtkCellPicker::New();
  m_CellPicker->SetTolerance( 0.0025 );

  mitk::PlaneGeometryDataVtkMapper3D::Pointer geometryMapper = mitk::PlaneGeometryDataVtkMapper3D::New();
  m_CurrentWorldPlaneGeometryMapper = geometryMapper;
  m_CurrentWorldPlaneGeometryNode->SetMapper(2, geometryMapper);

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

  static_cast<mitk::PlaneGeometryDataVtkMapper3D*>(m_CurrentWorldPlaneGeometryMapper.GetPointer())->SetDataStorageForTexture( m_DataStorage.GetPointer() );

  // Compute the geometry from the current data tree bounds and set it as world geometry
  this->SetWorldGeometryToDataStorageBounds();
}

bool mitk::VtkPropRenderer::SetWorldGeometryToDataStorageBounds()
{
  if ( m_DataStorage.IsNull() )
    return false;

  //initialize world geometry
  mitk::TimeGeometry::Pointer geometry = m_DataStorage->ComputeVisibleBoundingGeometry3D( NULL, "includeInBoundingBox" );

  if ( geometry.IsNull() )
    return false;

  this->SetWorldTimeGeometry(geometry);
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
  //Update all overlays in any case
  this->UpdateOverlays();
  // Do we have objects to render?
  if ( this->GetEmptyWorldGeometry())
    return 0;

  if ( m_DataStorage.IsNull())
    return 0;

  // Update mappers and prepare mapper queue
  if (type == VtkPropRenderer::Opaque)
    this->PrepareMapperQueue();

  //go through the generated list and let the sorted mappers paint
  for(MappersMapType::iterator it = m_MappersMap.begin(); it != m_MappersMap.end(); it++)
  {
    Mapper * mapper = (*it).second;
    mapper->MitkRender(this, type);
  }

  //Update overlays in case a mapper has changed them
  this->UpdateOverlays();

  // Render text
  if (type == VtkPropRenderer::Overlay)
  {
    if (m_TextCollection.size() > 0)
    {
      m_TextRenderer->SetViewport( this->GetVtkRenderer()->GetViewport() );
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

    bool visible = true;
    node->GetVisibility(visible, this, "visible");

    // The information about LOD-enabled mappers is required by RenderingManager
    if ( mapper->IsLODEnabled( this ) && visible )
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

void mitk::VtkPropRenderer::Update(mitk::DataNode* datatreenode)
{
  if(datatreenode!=NULL)
  {
    mitk::Mapper::Pointer mapper = datatreenode->GetMapper(m_MapperID);
    if(mapper.IsNotNull())
    {
      if(GetDisplayGeometry()->IsValid())
      {
        mapper->Update(this);
        {
          VtkMapper* vtkmapper=dynamic_cast<VtkMapper*>(mapper.GetPointer());
          if(vtkmapper != NULL)
          {
            vtkmapper->UpdateVtkTransform(this);
          }
        }
      }
    }
  }
}

void mitk::VtkPropRenderer::Update()
{
  if( m_DataStorage.IsNull() )
    return;

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

  vtkCallbackCommand *renderCallbackCommand = vtkCallbackCommand::New();
  renderCallbackCommand->SetCallback(
    VtkPropRenderer::RenderingCallback );
  renderWindow->GetInteractor()->AddObserver( vtkCommand::RenderEvent, renderCallbackCommand );
  renderCallbackCommand->Delete();

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

void mitk::VtkPropRenderer::RenderingCallback( vtkObject *caller, unsigned long , void *, void * )
{
  vtkRenderWindowInteractor *renderWindowInteractor
      = dynamic_cast< vtkRenderWindowInteractor * >( caller );
  if(!renderWindowInteractor) return;
  mitk::BaseRenderer* renderer = mitk::BaseRenderer::GetInstance(renderWindowInteractor->GetRenderWindow());
  if(renderer) renderer->RequestUpdate();
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
    m_PointPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
    vtk2itk(m_PointPicker->GetPickPosition(), worldPoint);
    break;
  }
  case(CellPicking) :
  {
    m_CellPicker->Pick(displayPoint[0], displayPoint[1], 0, m_VtkRenderer);
    vtk2itk(m_CellPicker->GetPickPosition(), worldPoint);
    break;
  }
  }
  //todo: is this picking in 2D renderwindows?
  //    Superclass::PickWorldPoint(displayPoint, worldPoint);
}

mitk::DataNode *
mitk::VtkPropRenderer::PickObject( const Point2D &displayPosition, Point3D &worldPosition ) const
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

    VtkMapper *mapper = dynamic_cast < VtkMapper * >  ( node->GetMapper( m_MapperID ) );
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

    mitk::Mapper * mapper = node->GetMapper( m_MapperID );
    if ( mapper == NULL)
      continue;

    mitk::VtkMapper * vtkmapper = dynamic_cast< VtkMapper * >(mapper);

    if(vtkmapper){
      //if vtk-based, then ...
      if ( vtkmapper->HasVtkProp( prop, const_cast< mitk::VtkPropRenderer * >( this ) ) )
      {
        return node;
      }
    }
  }
  return NULL;
}
//todo: is this 2D renderwindow picking?
//    return Superclass::PickObject( displayPosition, worldPosition );

int mitk::VtkPropRenderer::WriteSimpleText(std::string text, double posX, double posY, double color1, double color2, double color3, float opacity)
{
  this->GetVtkRenderer()->ViewToDisplay();
  if(!text.empty())
  {
    Point2D p;
    p[0] = posX;
    p[1] = posY;
    p = TransformOpenGLPointToViewport(p);

    vtkTextActor* textActor = vtkTextActor::New();

    textActor->SetPosition(p[0], p[1]);
    textActor->SetInput(text.c_str());
    textActor->SetTextScaleModeToNone();
    textActor->GetTextProperty()->SetColor(color1, color2, color3); //TODO: Read color from node property
    textActor->GetTextProperty()->SetOpacity( opacity );
    int text_id = m_TextCollection.size();
    m_TextCollection.insert(TextMapType::value_type(text_id,textActor));
    return text_id;
  }
  else
  {
    return -1;
  }
}

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

int mitk::VtkPropRenderer::GetNumberOfPaths()
{
  if (m_DataStorage.IsNull()) {
    return 0;
  }

  int nPaths = 0;
  DataStorage::SetOfObjects::ConstPointer objects = m_DataStorage->GetAll();
  for (DataStorage::SetOfObjects::const_iterator iter = objects->begin(); iter != objects->end(); ++iter) {
    Mapper* mapper = (*iter)->GetMapper(BaseRenderer::Standard3D);
    if (mapper)
    {
      VtkMapper* vtkmapper = dynamic_cast<VtkMapper*>(mapper);
      if (vtkmapper)
      {
        vtkProp* prop = vtkmapper->GetVtkProp(this);
        if (prop && prop->GetVisibility())
        {
          ++nPaths;
        }
      }
    }
  }

  return nPaths;
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
        VtkMapper* vtkmapper = dynamic_cast<VtkMapper*>( mapper );
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

void mitk::VtkPropRenderer::ReleaseGraphicsResources(vtkWindow* /*renWin*/)
{
  if( m_DataStorage.IsNull() )
    return;

  DataStorage::SetOfObjects::ConstPointer allObjects = m_DataStorage->GetAll();
  for (DataStorage::SetOfObjects::const_iterator iter = allObjects->begin(); iter != allObjects->end(); ++iter)
  {
    DataNode::Pointer node = *iter;
    if ( node.IsNull() )
      continue;

    Mapper * mapper = node->GetMapper(m_MapperID);

    if (mapper)
    {
      VtkMapper* vtkmapper = dynamic_cast<VtkMapper*>( mapper );

      if(vtkmapper)
        vtkmapper->ReleaseGraphicsResources(this);
    }
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
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    this->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->GetRenderWindow()->GetInteractor()->EnableRenderOff();
    m_CameraInitializedForMapperID = Standard3D;
  }
  else if( this->GetMapperID() == Standard2D)
  {
    //activate parallel projection for 2D
    this->GetVtkRenderer()->GetActiveCamera()->SetParallelProjection(true);
    //turn the light out in the scene in order to render correct grey values.
    //TODO Implement a property for light in the 2D render windows (in another method)
    this->GetVtkRenderer()->RemoveAllLights();

    vtkSmartPointer<mitkVtkInteractorStyle> style = vtkSmartPointer<mitkVtkInteractorStyle>::New();
    this->GetRenderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->GetRenderWindow()->GetInteractor()->EnableRenderOff();
    m_CameraInitializedForMapperID = Standard2D;
  }
  return true;
}

void mitk::VtkPropRenderer::AdjustCameraToScene(){
  if(this->GetMapperID() == Standard2D)
  {
    const mitk::DisplayGeometry* displayGeometry = this->GetDisplayGeometry();

    double objectHeightInMM = this->GetCurrentWorldPlaneGeometry()->GetExtentInMM(1);//the height of the current object slice in mm
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
      camera->SetClippingRange(0.1, 1000000); //Reason for huge range: VTK seems to calculate the clipping planes wrong for small values. See VTK bug (id #7823) in VTK bugtracker.
    }

    const PlaneGeometry *planeGeometry = dynamic_cast< const PlaneGeometry * >( this->GetCurrentWorldPlaneGeometry() );
    const AbstractTransformGeometry *abstractTransformGeometry = dynamic_cast< const AbstractTransformGeometry * >( planeGeometry );

    if ( planeGeometry != NULL && !abstractTransformGeometry)
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

mitk::Point2D mitk::VtkPropRenderer::TransformOpenGLPointToViewport( mitk::Point2D point )
{
  double* iViewport = this->GetVtkRenderer()->GetViewport();

  const mitk::DisplayGeometry* displayGeometry = this->GetDisplayGeometry();

  float displayGeometryWidth = displayGeometry->GetSizeInDisplayUnits()[0];
  float displayGeometryHeight = displayGeometry->GetSizeInDisplayUnits()[1];

  float viewportWidth = (iViewport[2]-iViewport[0]) * displayGeometryWidth;
  float viewportHeight = (iViewport[3]-iViewport[1]) * displayGeometryHeight; // seemingly right

  float zoom = (iViewport[3]-iViewport[1]);

  point[0] +=
    0.5 * (viewportWidth/viewportHeight-1.0)*displayGeometryHeight
    - 0.5 * (displayGeometryWidth - displayGeometryHeight)
    ;

  point[0] *= zoom;
  point[1] *= zoom;
  return point;
}
