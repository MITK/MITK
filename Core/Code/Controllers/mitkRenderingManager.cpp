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

#include "mitkRenderingManager.h"
#include "mitkRenderingManagerFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkGlobalInteraction.h"
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"
#include "mitkProportionalTimeGeometry.h"

#include <vtkRenderWindow.h>

#include <itkCommand.h>
#include "mitkNumericTypes.h"
#include <itkAffineGeometryFrame.h>
#include <itkScalableAffineTransform.h>
#include <mitkVtkPropRenderer.h>

#include <algorithm>

namespace mitk
{
RenderingManager::Pointer RenderingManager::s_Instance = 0;
RenderingManagerFactory *RenderingManager::s_RenderingManagerFactory = 0;

RenderingManager
::RenderingManager()
: m_UpdatePending( false ),
  m_MaxLOD( 1 ),
  m_LODIncreaseBlocked( false ),
  m_LODAbortMechanismEnabled( false ),
  m_ClippingPlaneEnabled( false ),
  m_TimeNavigationController( SliceNavigationController::New("dummy") ),
  m_DataStorage( NULL ),
  m_ConstrainedPaddingZooming ( true )
{
  m_ShadingEnabled.assign( 3, false );
  m_ShadingValues.assign( 4, 0.0 );

  m_GlobalInteraction = mitk::GlobalInteraction::GetInstance();

  InitializePropertyList();
}

RenderingManager
::~RenderingManager()
{
  // Decrease reference counts of all registered vtkRenderWindows for
  // proper destruction
  RenderWindowVector::iterator it;
  for ( it = m_AllRenderWindows.begin(); it != m_AllRenderWindows.end(); ++it )
  {
    (*it)->UnRegister( NULL );

    RenderWindowCallbacksList::iterator callbacks_it = this->m_RenderWindowCallbacksList.find(*it);

    if (callbacks_it != this->m_RenderWindowCallbacksList.end())
    {
      (*it)->RemoveObserver(callbacks_it->second.commands[0u]);
      (*it)->RemoveObserver(callbacks_it->second.commands[1u]);
      (*it)->RemoveObserver(callbacks_it->second.commands[2u]);
    }
  }
}

void
RenderingManager
::SetFactory( RenderingManagerFactory *factory )
{
  s_RenderingManagerFactory = factory;
}

const RenderingManagerFactory *
RenderingManager
::GetFactory()
{
  return s_RenderingManagerFactory;
}

bool
RenderingManager
::HasFactory()
{
  if ( RenderingManager::s_RenderingManagerFactory )
  {
    return true;
  }
  else
  {
    return false;
  }
}

RenderingManager::Pointer
RenderingManager
::New()
{
  const RenderingManagerFactory* factory = GetFactory();
  if(factory == NULL)
    return NULL;
  return factory->CreateRenderingManager();
}

RenderingManager *
RenderingManager
::GetInstance()
{
  if ( !RenderingManager::s_Instance )
  {
    if ( s_RenderingManagerFactory )
    {
      s_Instance = s_RenderingManagerFactory->CreateRenderingManager();
    }
  }

  return s_Instance;
}

bool
RenderingManager
::IsInstantiated()
{
  if ( RenderingManager::s_Instance )
    return true;
  else
    return false;
}

void
RenderingManager
::AddRenderWindow( vtkRenderWindow *renderWindow )
{
  if ( renderWindow
    && (m_RenderWindowList.find( renderWindow ) == m_RenderWindowList.end()) )
  {
    m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;
    m_AllRenderWindows.push_back( renderWindow );

    if ( m_DataStorage.IsNotNull() )
      mitk::BaseRenderer::GetInstance( renderWindow )->SetDataStorage( m_DataStorage.GetPointer() );

    // Register vtkRenderWindow instance
    renderWindow->Register( NULL );

    // Add callbacks for rendering abort mechanism
    //BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
    vtkCallbackCommand *startCallbackCommand = vtkCallbackCommand::New();
    startCallbackCommand->SetCallback(
      RenderingManager::RenderingStartCallback );
    renderWindow->AddObserver( vtkCommand::StartEvent, startCallbackCommand );

    vtkCallbackCommand *progressCallbackCommand = vtkCallbackCommand::New();
    progressCallbackCommand->SetCallback(
      RenderingManager::RenderingProgressCallback );
    renderWindow->AddObserver( vtkCommand::AbortCheckEvent, progressCallbackCommand );

    vtkCallbackCommand *endCallbackCommand = vtkCallbackCommand::New();
    endCallbackCommand->SetCallback(
      RenderingManager::RenderingEndCallback );
    renderWindow->AddObserver( vtkCommand::EndEvent, endCallbackCommand );

    RenderWindowCallbacks callbacks;

    callbacks.commands[0u] = startCallbackCommand;
    callbacks.commands[1u] = progressCallbackCommand;
    callbacks.commands[2u] = endCallbackCommand;
    this->m_RenderWindowCallbacksList[renderWindow] = callbacks;

    //Delete vtk variables correctly
    startCallbackCommand->Delete();
    progressCallbackCommand->Delete();
    endCallbackCommand->Delete();
  }
}

void
RenderingManager
::RemoveRenderWindow( vtkRenderWindow *renderWindow )
{
  if (m_RenderWindowList.erase( renderWindow ))
  {
    RenderWindowCallbacksList::iterator callbacks_it = this->m_RenderWindowCallbacksList.find(renderWindow);
    if(callbacks_it != this->m_RenderWindowCallbacksList.end())
    {
      renderWindow->RemoveObserver(callbacks_it->second.commands[0u]);
      renderWindow->RemoveObserver(callbacks_it->second.commands[1u]);
      renderWindow->RemoveObserver(callbacks_it->second.commands[2u]);
      this->m_RenderWindowCallbacksList.erase(callbacks_it);
    }

    RenderWindowVector::iterator rw_it = std::find( m_AllRenderWindows.begin(), m_AllRenderWindows.end(), renderWindow );

    if(rw_it != m_AllRenderWindows.end())
    {
      // Decrease reference count for proper destruction
      (*rw_it)->UnRegister(NULL);
      m_AllRenderWindows.erase( rw_it );
    }
  }
}

const RenderingManager::RenderWindowVector&
RenderingManager
::GetAllRegisteredRenderWindows()
{
  return m_AllRenderWindows;
}

void
RenderingManager
::RequestUpdate( vtkRenderWindow *renderWindow )
{
  // If the renderWindow is not valid, we do not want to inadvertantly create
  // an entry in the m_RenderWindowList map. It is possible if the user is
  // regularly calling AddRenderer and RemoveRenderer for a rendering update
  // to come into this method with a renderWindow pointer that is valid in the
  // sense that the window does exist within the application, but that
  // renderWindow has been temporarily removed from this RenderingManager for
  // performance reasons.
  if (m_RenderWindowList.find( renderWindow ) == m_RenderWindowList.end())
  {
    return;
  }

  m_RenderWindowList[renderWindow] = RENDERING_REQUESTED;

  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->GenerateRenderingRequestEvent();
  }
}

void
RenderingManager
::ForceImmediateUpdate( vtkRenderWindow *renderWindow )
{
  // If the renderWindow is not valid, we do not want to inadvertantly create
  // an entry in the m_RenderWindowList map. It is possible if the user is
  // regularly calling AddRenderer and RemoveRenderer for a rendering update
  // to come into this method with a renderWindow pointer that is valid in the
  // sense that the window does exist within the application, but that
  // renderWindow has been temporarily removed from this RenderingManager for
  // performance reasons.
  if (m_RenderWindowList.find( renderWindow ) == m_RenderWindowList.end())
  {
    return;
  }

  // Erase potentially pending requests for this window
  m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;

  m_UpdatePending = false;

  // Immediately repaint this window (implementation platform specific)
  // If the size is 0 it crahses
  int *size = renderWindow->GetSize();
  if ( 0 != size[0] && 0 != size[1] )
  {
    //prepare the camera etc. before rendering
    //Note: this is a very important step which should be called before the VTK render!
    //If you modify the camera anywhere else or after the render call, the scene cannot be seen.
    mitk::VtkPropRenderer *vPR =
        dynamic_cast<mitk::VtkPropRenderer*>(mitk::BaseRenderer::GetInstance( renderWindow ));
    if(vPR)
       vPR->PrepareRender();
    // Execute rendering
    renderWindow->Render();
  }
}

void
RenderingManager
::RequestUpdateAll( RequestType type )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int id = BaseRenderer::GetInstance(it->first)->GetMapperID();
    if ( (type == REQUEST_UPDATE_ALL)
      || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
       this->RequestUpdate( it->first );
    }
  }
}

void
RenderingManager
::ForceImmediateUpdateAll( RequestType type )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int id = BaseRenderer::GetInstance(it->first)->GetMapperID();
    if ( (type == REQUEST_UPDATE_ALL)
      || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      // Immediately repaint this window (implementation platform specific)
      // If the size is 0, it crashes
      this->ForceImmediateUpdate(it->first);
    }
  }
}

void RenderingManager::InitializeViewsByBoundingObjects( const DataStorage *ds)
{
  if (!ds)
    return;

  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNot::Pointer pred
    = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
    , mitk::BoolProperty::New(false)));

  mitk::DataStorage::SetOfObjects::ConstPointer rs = ds->GetSubset(pred);
  // calculate bounding geometry of these nodes
  mitk::TimeGeometry::Pointer bounds = ds->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  this->InitializeViews(bounds);
}

//TODO_GOETZ
// Remove old function, so only this one is working.
bool
RenderingManager
::InitializeViews( const BaseGeometry * dataGeometry, RequestType type, bool preserveRoughOrientationInWorldSpace )
{
  ProportionalTimeGeometry::Pointer propTimeGeometry = ProportionalTimeGeometry::New();
  propTimeGeometry->Initialize(dynamic_cast<BaseGeometry *>(dataGeometry->Clone().GetPointer()), 1);
  return InitializeViews(propTimeGeometry,type, preserveRoughOrientationInWorldSpace);
}

bool
RenderingManager
::InitializeViews( const TimeGeometry * dataGeometry, RequestType type, bool /*preserveRoughOrientationInWorldSpace*/ )
{
  MITK_DEBUG << "initializing views";

  bool boundingBoxInitialized = false;

  TimeGeometry::ConstPointer timeGeometry = dataGeometry;
  TimeGeometry::Pointer modifiedGeometry = NULL;
  if (dataGeometry!=NULL)
  {
    modifiedGeometry = dataGeometry->Clone();
  }

  // //TODO_GOETZ previously this code section has been disabled by
  // a later asignment to geometry (e.g. timeGeometry)
  // This has been fixed during Geometry-1-Plattform Project
  // Propably this code is not working anymore, test!!
  /*
  if (dataGeometry && preserveRoughOrientationInWorldSpace)
  {
    // clone the input geometry
    assert(modifiedGeometry.IsNotNull());

    // construct an affine transform from it
    Geometry3D::TransformType::Pointer transform = Geometry3D::TransformType::New();
    assert( modifiedGeometry->GetGeometryForTimeStep(0)->GetIndexToWorldTransform() );
    transform->SetMatrix( modifiedGeometry->GetGeometryForTimeStep(0)->GetIndexToWorldTransform()->GetMatrix() );
    transform->SetOffset( modifiedGeometry->GetGeometryForTimeStep(0)->GetIndexToWorldTransform()->GetOffset() );

    // get transform matrix
    Geometry3D::TransformType::MatrixType::InternalMatrixType& oldMatrix =
      const_cast< Geometry3D::TransformType::MatrixType::InternalMatrixType& > ( transform->GetMatrix().GetVnlMatrix() );
    Geometry3D::TransformType::MatrixType::InternalMatrixType newMatrix(oldMatrix);

    // get offset and bound
    Vector3D offset = modifiedGeometry->GetIndexToWorldTransform()->GetOffset();
    Geometry3D::BoundsArrayType oldBounds = modifiedGeometry->GetBounds();
    Geometry3D::BoundsArrayType newBounds = modifiedGeometry->GetBounds();

    // get rid of rotation other than pi/2 degree
    for ( unsigned int i = 0; i < 3; ++i )
    {
      // i-th column of the direction matrix
      Vector3D currentVector;
      currentVector[0] = oldMatrix(0,i);
      currentVector[1] = oldMatrix(1,i);
      currentVector[2] = oldMatrix(2,i);

      // matchingRow will store the row that holds the biggest
      // value in the column
      unsigned int matchingRow = 0;

      // maximum value in the column
      ScalarType max = std::numeric_limits<ScalarType>::min();

      // sign of the maximum value (-1 or 1)
      int sign = 1;

      // iterate through the column vector
      for (unsigned int dim = 0; dim < 3; ++dim)
      {
        if ( fabs(currentVector[dim]) > max )
        {
          matchingRow = dim;
          max = fabs(currentVector[dim]);
          if(currentVector[dim]<0)
            sign = -1;
          else
            sign = 1;
        }
      }

      // in case we found a negative maximum,
      // we negate the column and adjust the offset
      // (in order to run through the dimension in the opposite direction)
      if(sign == -1)
      {
        currentVector *= sign;
        offset += modifiedGeometry->GetAxisVector(i);
      }

      // matchingRow is now used as column index to place currentVector
      // correctly in the new matrix
      vnl_vector<ScalarType> newMatrixColumn(3);
      newMatrixColumn[0] = currentVector[0];
      newMatrixColumn[1] = currentVector[1];
      newMatrixColumn[2] = currentVector[2];
      newMatrix.set_column( matchingRow, newMatrixColumn );

      // if a column is moved, we also have to adjust the bounding
      // box accordingly, this is done here
      newBounds[2*matchingRow  ] = oldBounds[2*i  ];
      newBounds[2*matchingRow+1] = oldBounds[2*i+1];
    }

    // set the newly calculated bounds array
    modifiedGeometry->SetBounds(newBounds);

    // set new offset and direction matrix
    Geometry3D::TransformType::MatrixType newMatrixITK( newMatrix );
    transform->SetMatrix( newMatrixITK );
    transform->SetOffset( offset );
    modifiedGeometry->SetIndexToWorldTransform( transform );
    geometry = modifiedGeometry;
  }*/

  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (timeGeometry.IsNotNull() ) && (const_cast< mitk::BoundingBox * >(
    timeGeometry->GetBoundingBoxInWorld())->GetDiagonalLength2() > mitk::eps) )
  {
    boundingBoxInitialized = true;
  }

  if (timeGeometry.IsNotNull() )
  {// make sure bounding box has an extent bigger than zero in any direction
    // clone the input geometry
    //Old Geometry3D::Pointer modifiedGeometry = dynamic_cast<Geometry3D*>( dataGeometry->Clone().GetPointer() );
    assert(modifiedGeometry.IsNotNull());
    for (TimeStepType step = 0; step < modifiedGeometry->CountTimeSteps(); ++step)
    {
      BaseGeometry::BoundsArrayType newBounds = modifiedGeometry->GetGeometryForTimeStep(step)->GetBounds();
      for( unsigned int dimension = 0; ( 2 * dimension ) < newBounds.Size() ; dimension++ )
      {
        //check for equality but for an epsilon
        if( Equal( newBounds[ 2 * dimension ], newBounds[ 2 * dimension + 1 ] ) )
        {
          newBounds[ 2 * dimension + 1 ] += 1;
        }
      }
      modifiedGeometry->GetGeometryForTimeStep(step)->SetBounds(newBounds);
    }
  }

  timeGeometry = modifiedGeometry;
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( it->first );

    baseRenderer->GetDisplayGeometry()->SetConstrainZoomingAndPanning(m_ConstrainedPaddingZooming);

    int id = baseRenderer->GetMapperID();
    if ( ((type == REQUEST_UPDATE_ALL)
      || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)))
      )
    {
      this->InternalViewInitialization( baseRenderer, timeGeometry,
        boundingBoxInitialized, id );
    }
  }

  if ( boundingBoxInitialized )
  {
    m_TimeNavigationController->SetInputWorldTimeGeometry( timeGeometry );
  }
  m_TimeNavigationController->Update();

  this->RequestUpdateAll( type );

  vtkObject::SetGlobalWarningDisplay( warningLevel );

  // Inform listeners that views have been initialized
  this->InvokeEvent( mitk::RenderingManagerViewsInitializedEvent() );

  return boundingBoxInitialized;
}

bool
RenderingManager
::InitializeViews( RequestType type )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( it->first );
    int id = baseRenderer->GetMapperID();
    if ( (type == REQUEST_UPDATE_ALL)
      || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)) )
    {
      mitk::SliceNavigationController *nc =

      baseRenderer->GetSliceNavigationController();

      // Re-initialize view direction
      nc->SetViewDirectionToDefault();

      // Update the SNC
      nc->Update();
    }
  }

  this->RequestUpdateAll( type );

  return true;
}

bool RenderingManager::InitializeView( vtkRenderWindow * renderWindow, const BaseGeometry * geometry, bool initializeGlobalTimeSNC )
{
  ProportionalTimeGeometry::Pointer propTimeGeometry = ProportionalTimeGeometry::New();
  propTimeGeometry->Initialize(dynamic_cast<BaseGeometry *>(geometry->Clone().GetPointer()), 1);
  return InitializeView(renderWindow, propTimeGeometry, initializeGlobalTimeSNC );
}

bool RenderingManager::InitializeView( vtkRenderWindow * renderWindow, const TimeGeometry * geometry, bool initializeGlobalTimeSNC )
{
  bool boundingBoxInitialized = false;

  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (geometry != NULL ) && (const_cast< mitk::BoundingBox * >(
   geometry->GetBoundingBoxInWorld())->GetDiagonalLength2() > mitk::eps) )
  {
   boundingBoxInitialized = true;
  }

  mitk::BaseRenderer *baseRenderer =
   mitk::BaseRenderer::GetInstance( renderWindow );

  int id = baseRenderer->GetMapperID();

  this->InternalViewInitialization( baseRenderer, geometry,
   boundingBoxInitialized, id );

  if ( boundingBoxInitialized && initializeGlobalTimeSNC )
  {
    m_TimeNavigationController->SetInputWorldTimeGeometry( geometry );
  }
  m_TimeNavigationController->Update();

  this->RequestUpdate( renderWindow );

  vtkObject::SetGlobalWarningDisplay( warningLevel );

  return boundingBoxInitialized;
}

bool RenderingManager::InitializeView( vtkRenderWindow * renderWindow )
{
  mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( renderWindow );

  mitk::SliceNavigationController *nc =
    baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  nc->SetViewDirectionToDefault();

  // Update the SNC
  nc->Update();

  this->RequestUpdate( renderWindow );

  return true;
}

void RenderingManager::InternalViewInitialization(mitk::BaseRenderer *baseRenderer, const mitk::TimeGeometry *geometry, bool boundingBoxInitialized, int mapperID )
{
  mitk::SliceNavigationController *nc = baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  nc->SetViewDirectionToDefault();

  if ( boundingBoxInitialized )
  {
    // Set geometry for NC
    nc->SetInputWorldTimeGeometry( geometry );
    nc->Update();

    if ( mapperID == 1 )
    {
      // For 2D SNCs, steppers are set so that the cross is centered
      // in the image
      nc->GetSlice()->SetPos( nc->GetSlice()->GetSteps() / 2 );
    }

    // Fit the render window DisplayGeometry
    baseRenderer->GetDisplayGeometry()->Fit();
    baseRenderer->GetCameraController()->SetViewToAnterior();
  }
  else
  {
    nc->Update();
  }
}

const SliceNavigationController* RenderingManager::GetTimeNavigationController() const
{
  return m_TimeNavigationController.GetPointer();
}

SliceNavigationController* RenderingManager::GetTimeNavigationController()
{
  return m_TimeNavigationController.GetPointer();
}

void RenderingManager::ExecutePendingRequests()
{
  m_UpdatePending = false;

  // Satisfy all pending update requests
  RenderWindowList::iterator it;
  int i = 0;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it, ++i )
  {
    if ( it->second == RENDERING_REQUESTED )
    {
      this->ForceImmediateUpdate( it->first );
    }
  }
}

void RenderingManager::RenderingStartCallback( vtkObject *caller, unsigned long , void *, void * )
{
  vtkRenderWindow *renderWindow  = dynamic_cast< vtkRenderWindow * >( caller );
  mitk::RenderingManager* renman = mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderingManager();
  RenderWindowList &renderWindowList = renman->m_RenderWindowList;

  if ( renderWindow )
  {
    renderWindowList[renderWindow] = RENDERING_INPROGRESS;
  }

  renman->m_UpdatePending = false;
}

void
RenderingManager
::RenderingProgressCallback( vtkObject *caller, unsigned long , void *, void * )
{
  vtkRenderWindow *renderWindow  = dynamic_cast< vtkRenderWindow * >( caller );
  mitk::RenderingManager* renman = mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderingManager();

  if ( renman->m_LODAbortMechanismEnabled )
  {
    vtkRenderWindow *renderWindow = dynamic_cast< vtkRenderWindow * >( caller );
    if ( renderWindow )
    {
      BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
      if ( renderer && (renderer->GetNumberOfVisibleLODEnabledMappers() > 0) )
      {
        renman->DoMonitorRendering();
      }
    }
  }
}

void
RenderingManager
::RenderingEndCallback( vtkObject *caller, unsigned long , void *, void * )
{
  vtkRenderWindow *renderWindow  = dynamic_cast< vtkRenderWindow * >( caller );

  mitk::RenderingManager* renman = mitk::BaseRenderer::GetInstance(renderWindow)->GetRenderingManager();

  RenderWindowList &renderWindowList = renman->m_RenderWindowList;

  RendererIntMap &nextLODMap = renman->m_NextLODMap;

  if ( renderWindow )
  {
    BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
    if ( renderer )
    {
      renderWindowList[renderer->GetRenderWindow()] = RENDERING_INACTIVE;

      // Level-of-Detail handling
      if ( renderer->GetNumberOfVisibleLODEnabledMappers() > 0 )
      {
        if(nextLODMap[renderer]==0)
          renman->StartOrResetTimer();
        else
          nextLODMap[renderer] = 0;
      }
    }
  }
}

bool
RenderingManager
::IsRendering() const
{
  RenderWindowList::const_iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second == RENDERING_INPROGRESS )
    {
      return true;
    }
  }
  return false;
}

void
RenderingManager
::AbortRendering()
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second == RENDERING_INPROGRESS )
    {
      it->first->SetAbortRender( true );
      m_RenderingAbortedMap[BaseRenderer::GetInstance(it->first)] = true;
    }
  }
}

int
RenderingManager
::GetNextLOD( BaseRenderer *renderer )
{
  if ( renderer != NULL )
  {
    return m_NextLODMap[renderer];
  }
  else
  {
    return 0;
  }
}

void
RenderingManager
::ExecutePendingHighResRenderingRequest()
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    BaseRenderer *renderer = BaseRenderer::GetInstance( it->first );

    if(renderer->GetNumberOfVisibleLODEnabledMappers()>0)
    {
      if(m_NextLODMap[renderer]==0)
      {
        m_NextLODMap[renderer]=1;
        RequestUpdate( it->first );
      }
    }
  }
}

void
RenderingManager
::SetMaximumLOD( unsigned int max )
{
  m_MaxLOD = max;
}

//enable/disable shading
void
RenderingManager
::SetShading(bool state, unsigned int lod)
{
  if(lod>m_MaxLOD)
  {
    itkWarningMacro(<<"LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
    return;
  }
  m_ShadingEnabled[lod] = state;
}

bool
RenderingManager
::GetShading(unsigned int lod)
{
  if(lod>m_MaxLOD)
  {
    itkWarningMacro(<<"LOD out of range requested: " << lod << " maxLOD: " << m_MaxLOD);
    return false;
  }
  return m_ShadingEnabled[lod];
}

//enable/disable the clipping plane
void
RenderingManager
::SetClippingPlaneStatus(bool status)
{
  m_ClippingPlaneEnabled = status;
}

bool
RenderingManager
::GetClippingPlaneStatus()
{
  return m_ClippingPlaneEnabled;
}

void
RenderingManager
::SetShadingValues(float ambient, float diffuse, float specular, float specpower)
{
  m_ShadingValues[0] = ambient;
  m_ShadingValues[1] = diffuse;
  m_ShadingValues[2] = specular;
  m_ShadingValues[3] = specpower;
}

RenderingManager::FloatVector &
RenderingManager
::GetShadingValues()
{
  return m_ShadingValues;
}

void RenderingManager::InitializePropertyList()
{
  if (m_PropertyList.IsNull())
  {
    m_PropertyList = PropertyList::New();
  }

  this->SetProperty("coupled-zoom", BoolProperty::New(false));
  this->SetProperty("coupled-plane-rotation", BoolProperty::New(false));
  this->SetProperty("MIP-slice-rendering", BoolProperty::New(false));
}

PropertyList::Pointer RenderingManager::GetPropertyList() const
{
  return m_PropertyList;
}

BaseProperty* RenderingManager::GetProperty(const char *propertyKey) const
{
  return m_PropertyList->GetProperty(propertyKey);
}

void RenderingManager::SetProperty(const char *propertyKey, BaseProperty* propertyValue)
{
  m_PropertyList->SetProperty(propertyKey, propertyValue);
}

void RenderingManager::SetDataStorage( DataStorage* storage )
{
  if ( storage != NULL )
  {
    m_DataStorage = storage;

    RenderingManager::RenderWindowVector::iterator iter;
    for ( iter = m_AllRenderWindows.begin(); iter<m_AllRenderWindows.end(); iter++ )
    {
      mitk::BaseRenderer::GetInstance( (*iter) )->SetDataStorage( m_DataStorage.GetPointer() );
    }
  }
}

mitk::DataStorage* RenderingManager::GetDataStorage()
{
  return m_DataStorage;
}

void RenderingManager::SetGlobalInteraction( mitk::GlobalInteraction* globalInteraction )
{
  if ( globalInteraction != NULL )
  {
    m_GlobalInteraction = globalInteraction;
  }
}

mitk::GlobalInteraction* RenderingManager::GetGlobalInteraction()
{
  return m_GlobalInteraction;
}

// Create and register generic RenderingManagerFactory.
TestingRenderingManagerFactory renderingManagerFactory;
} // namespace
