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

#include "mitkRenderingManager.h"
#include "mitkRenderingManagerFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkDataStorage.h"

#include "mitkDataTreeStorage.h"
#include "mitkDataStorage.h"

#include <vtkRenderWindow.h>

#include <itkCommand.h>
#include "mitkVector.h"
#include <itkAffineGeometryFrame.h>
#include <itkScalableAffineTransform.h>

#include <algorithm>

namespace mitk
{

RenderingManager::Pointer RenderingManager::s_Instance = 0;
RenderingManagerFactory *RenderingManager::s_RenderingManagerFactory = 0;


RenderingManager
::RenderingManager()
: m_UpdatePending( false ),
  m_MaxLOD( 2 ),
  m_LODIncreaseBlocked( false ),
  m_LODAbortMechanismEnabled( false ),
  m_ClippingPlaneEnabled( false ),
  m_TimeNavigationController( NULL )
{
  m_ShadingEnabled.assign( 3, false );
  m_ShadingValues.assign( 4, 0.0 );
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

    (*it)->RemoveObserver(callbacks_it->second.commands[0u]);
    (*it)->RemoveObserver(callbacks_it->second.commands[1u]);
    (*it)->RemoveObserver(callbacks_it->second.commands[2u]);
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

    // Register vtkRenderWindow instance
    renderWindow->Register( NULL );

    typedef itk::MemberCommand< RenderingManager > MemberCommandType;

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
  }
}


void
RenderingManager
::RemoveRenderWindow( vtkRenderWindow *renderWindow )
{
  if (m_RenderWindowList.erase( renderWindow ))
  {
    RenderWindowCallbacksList::iterator callbacks_it = this->m_RenderWindowCallbacksList.find(renderWindow);

    renderWindow->RemoveObserver(callbacks_it->second.commands[0u]);
    renderWindow->RemoveObserver(callbacks_it->second.commands[1u]);
    renderWindow->RemoveObserver(callbacks_it->second.commands[2u]);
    this->m_RenderWindowCallbacksList.erase(callbacks_it);

    RenderWindowVector::iterator rw_it = std::find( m_AllRenderWindows.begin(), m_AllRenderWindows.end(), renderWindow );

    // Decrease reference count for proper destruction
    (*rw_it)->UnRegister(NULL);
    m_AllRenderWindows.erase( rw_it );
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
  // Erase potentially pending requests for this window
  m_RenderWindowList[renderWindow] = RENDERING_INACTIVE;

  m_UpdatePending = false;

  // Immediately repaint this window (implementation platform specific)
  // If the size is 0 it crahses
  int *size = renderWindow->GetSize();
  if ( 0 != size[0] && 0 != size[1] )
  {
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
      //it->second = RENDERING_INPROGRESS;

      // Immediately repaint this window (implementation platform specific)
      // If the size is 0, it crashes
      int *size = it->first->GetSize();
      if ( 0 != size[0] && 0 != size[1] )
      {
        // Execute rendering
        it->first->Render();
      }

      it->second = RENDERING_INACTIVE;
    }
  }

  m_UpdatePending = false;
}


bool
RenderingManager
::InitializeViews( DataTreeIteratorBase * dataIt, RequestType type, bool preserveRoughOrientationInWorldSpace )
{
  mitk::Geometry3D::Pointer geometry;
  if ( dataIt != NULL )
  {
    geometry = mitk::DataTree::ComputeVisibleBoundingGeometry3D(
      dataIt, NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      // let's see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry =
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(
          geometry, (unsigned int) duration );
        timegeometry->SetTimeBounds( timebounds );

        timebounds[1] = timebounds[0] + 1.0;
        geometry->SetTimeBounds( timebounds );

        geometry = timegeometry;
      }
    }
  }

  // Use geometry for initialization
  return this->InitializeViews( geometry.GetPointer(), type, preserveRoughOrientationInWorldSpace );
}


bool
RenderingManager
::InitializeViews( const mitk::DataStorage * storage, RequestType type, bool preserveRoughOrientationInWorldSpace )
{
  mitk::DataTreePreOrderIterator it((dynamic_cast<const mitk::DataTreeStorage*>(storage))->m_DataTree);
  return this->InitializeViews(&it, type, preserveRoughOrientationInWorldSpace);


// following lines of code are never reached
// and may be used to generate native datastorage code later

    mitk::Geometry3D::Pointer geometry;
  if ( storage != NULL )
  {
//    geometry = mitk::DataStorage::ComputeVisibleBoundingGeometry3D(
//      *storage, "visible", NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      // let's see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry =
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(
          geometry, (unsigned int) duration );
        timegeometry->SetTimeBounds( timebounds );

        timebounds[1] = timebounds[0] + 1.0;
        geometry->SetTimeBounds( timebounds );

        geometry = timegeometry;
      }
    }
  }

  // Use geometry for initialization
  return this->InitializeViews( geometry.GetPointer(), type );
}


bool
RenderingManager
::InitializeViews( const Geometry3D * dataGeometry, RequestType type, bool preserveRoughOrientationInWorldSpace )
{
  bool boundingBoxInitialized = false;

  Geometry3D::ConstPointer geometry = dataGeometry;

  if (dataGeometry && preserveRoughOrientationInWorldSpace)
  {

    // clone the input geometry
    Geometry3D::Pointer modifiedGeometry = dynamic_cast<Geometry3D*>( dataGeometry->Clone().GetPointer() );
    assert(modifiedGeometry.IsNotNull());

    // construct an affine transform from it
    AffineGeometryFrame3D::TransformType::Pointer transform = AffineGeometryFrame3D::TransformType::New();
    assert( modifiedGeometry->GetIndexToWorldTransform() );
    transform->SetMatrix( modifiedGeometry->GetIndexToWorldTransform()->GetMatrix() );
    transform->SetOffset( modifiedGeometry->GetIndexToWorldTransform()->GetOffset() );

    // get transform matrix
    AffineGeometryFrame3D::TransformType::MatrixType::InternalMatrixType& oldMatrix =
      const_cast< AffineGeometryFrame3D::TransformType::MatrixType::InternalMatrixType& > ( transform->GetMatrix().GetVnlMatrix() );
    AffineGeometryFrame3D::TransformType::MatrixType::InternalMatrixType newMatrix(oldMatrix);

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
      float max = std::numeric_limits<float>::min();

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
    AffineGeometryFrame3D::TransformType::MatrixType newMatrixITK( newMatrix );
    transform->SetMatrix( newMatrixITK );
    transform->SetOffset( offset );
    modifiedGeometry->SetIndexToWorldTransform( transform );
    geometry = modifiedGeometry;

  }


  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (geometry.IsNotNull() ) && (const_cast< mitk::BoundingBox * >(
    geometry->GetBoundingBox())->GetDiagonalLength2() > mitk::eps) )
  {
    boundingBoxInitialized = true;
  }

  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer =
      mitk::BaseRenderer::GetInstance( it->first );
    int id = baseRenderer->GetMapperID();
    if ( ((type == REQUEST_UPDATE_ALL)
      || ((type == REQUEST_UPDATE_2DWINDOWS) && (id == 1))
      || ((type == REQUEST_UPDATE_3DWINDOWS) && (id == 2)))
      )
    {
      this->InternalViewInitialization( baseRenderer, geometry,
        boundingBoxInitialized, id );
    }
  }

  if ( m_TimeNavigationController != NULL )
  {
    if ( boundingBoxInitialized )
    {
      m_TimeNavigationController->SetInputWorldGeometry( geometry );
    }
    m_TimeNavigationController->Update();
  }

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

bool
RenderingManager
::InitializeView( vtkRenderWindow * renderWindow,
  DataTreeIteratorBase * dataIt,
  bool initializeGlobalTimeSNC )
{
  mitk::Geometry3D::Pointer geometry;
  if ( dataIt != NULL )
  {
    geometry = mitk::DataTree::ComputeVisibleBoundingGeometry3D(
      dataIt, NULL, "includeInBoundingBox" );

    if ( geometry.IsNotNull() )
    {
      // let's see if we have data with a limited live-span ...
      mitk::TimeBounds timebounds = geometry->GetTimeBounds();
      if ( timebounds[1] < mitk::ScalarTypeNumericTraits::max() )
      {
        mitk::ScalarType duration = timebounds[1]-timebounds[0];

        mitk::TimeSlicedGeometry::Pointer timegeometry =
          mitk::TimeSlicedGeometry::New();
        timegeometry->InitializeEvenlyTimed(
          geometry, (unsigned int) duration );
        timegeometry->SetTimeBounds( timebounds );

        timebounds[1] = timebounds[0] + 1.0;
        geometry->SetTimeBounds( timebounds );

        geometry = timegeometry;
      }
    }
  }

  // Use geometry for initialization
  return this->InitializeView( renderWindow,
    geometry.GetPointer(), initializeGlobalTimeSNC );
}

bool
RenderingManager
::InitializeView( vtkRenderWindow * renderWindow,
  const Geometry3D * geometry,
  bool initializeGlobalTimeSNC )
{
  bool boundingBoxInitialized = false;

  int warningLevel = vtkObject::GetGlobalWarningDisplay();
  vtkObject::GlobalWarningDisplayOff();

  if ( (geometry != NULL ) && (const_cast< mitk::BoundingBox * >(
   geometry->GetBoundingBox())->GetDiagonalLength2() > mitk::eps) )
  {
   boundingBoxInitialized = true;
  }

  mitk::BaseRenderer *baseRenderer =
   mitk::BaseRenderer::GetInstance( renderWindow );

  int id = baseRenderer->GetMapperID();

  this->InternalViewInitialization( baseRenderer, geometry,
   boundingBoxInitialized, id );

  if ( m_TimeNavigationController != NULL )
  {
   if ( boundingBoxInitialized && initializeGlobalTimeSNC )
   {
     m_TimeNavigationController->SetInputWorldGeometry( geometry );
   }
   m_TimeNavigationController->Update();
  }

  this->RequestUpdate( renderWindow );

  vtkObject::SetGlobalWarningDisplay( warningLevel );

  return boundingBoxInitialized;
}


bool
RenderingManager
::InitializeView( vtkRenderWindow * renderWindow )
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

void
RenderingManager
::InternalViewInitialization(
  mitk::BaseRenderer *baseRenderer, const mitk::Geometry3D *geometry,
  bool boundingBoxInitialized, int mapperID )
{
  mitk::SliceNavigationController *nc =
    baseRenderer->GetSliceNavigationController();

  // Re-initialize view direction
  nc->SetViewDirectionToDefault();

  if ( boundingBoxInitialized )
  {
    // Set geometry for NC
    nc->SetInputWorldGeometry( geometry );
    nc->Update();

    if ( mapperID == 1 )
    {
      // For 2D SNCs, steppers are set so that the cross is centered
      // in the image
      nc->GetSlice()->SetPos( nc->GetSlice()->GetSteps() / 2 );
    }

    // Fit the render window DisplayGeometry
    baseRenderer->GetDisplayGeometry()->Fit();

    vtkRenderer *renderer = baseRenderer->GetVtkRenderer();

    /*
      the LocalizerWidget's camera must not be resetted, because it's interaction is disabled and the cameraposition cannot be changed manually
    */
    if ( (std::string) renderer->GetRenderWindow()->GetWindowName() != "LocalizerRenderWindow")
    {
      if ( renderer != NULL ) renderer->ResetCamera();
    }
  }
  else
  {
    nc->Update();
  }
}


void
RenderingManager
::SetTimeNavigationController( SliceNavigationController *nc )
{
  m_TimeNavigationController = nc;
}


const SliceNavigationController *
RenderingManager
::GetTimeNavigationController() const
{
  return m_TimeNavigationController;
}


SliceNavigationController *
RenderingManager
::GetTimeNavigationController()
{
  return m_TimeNavigationController;
}


void
RenderingManager
::UpdateCallback()
{
  m_UpdatePending = false;

  // Satisfy all pending update requests
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second == RENDERING_REQUESTED )
    {
      this->ForceImmediateUpdate( it->first );
    }
  }
}


void
RenderingManager
::RenderingStartCallback( vtkObject *caller, unsigned long , void *, void * )
{
  // Static method: access member objects via static instance
  RenderWindowList &renderWindowList = GetInstance()->m_RenderWindowList;

  vtkRenderWindow *renderWindow = dynamic_cast< vtkRenderWindow * >( caller );
  if ( renderWindow )
  {
    renderWindowList[renderWindow] = RENDERING_INPROGRESS;
  }

  GetInstance()->m_UpdatePending = false;
}


void
RenderingManager
::RenderingProgressCallback( vtkObject *caller, unsigned long , void *, void * )
{
  if ( GetInstance()->m_LODAbortMechanismEnabled )
  {
    vtkRenderWindow *renderWindow = dynamic_cast< vtkRenderWindow * >( caller );
    if ( renderWindow )
    {
      BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
      if ( renderer && (renderer->GetNumberOfVisibleLODEnabledMappers() > 0) )
      {
        GetInstance()->DoMonitorRendering();
      }
    }
  }
}

void
RenderingManager
::RenderingEndCallback( vtkObject *caller, unsigned long , void *, void * )
{
  // Static method: access member objects via static instance
  RenderWindowList &renderWindowList = GetInstance()->m_RenderWindowList;
  RendererBoolMap &renderingAbortedMap = GetInstance()->m_RenderingAbortedMap;
  RendererIntMap &nextLODMap = GetInstance()->m_NextLODMap;
  unsigned int &maxLOD = GetInstance()->m_MaxLOD;
  bool &lodIncreaseBlocked = GetInstance()->m_LODIncreaseBlocked;

  vtkRenderWindow *renderWindow = dynamic_cast< vtkRenderWindow * >( caller );
  if ( renderWindow )
  {
    BaseRenderer *renderer = BaseRenderer::GetInstance( renderWindow );
    if ( renderer )
    {
      renderWindowList[renderer->GetRenderWindow()] = RENDERING_INACTIVE;

      // Level-of-Detail handling
      if ( renderer->GetNumberOfVisibleLODEnabledMappers() > 0 )
      {
        bool newRenderingRequest = false;

        // Check if rendering has been aborted
        if ( renderingAbortedMap[renderer] )
        {
          // YES: reset LOD counter and request next update
          renderingAbortedMap[renderer] = false;
          nextLODMap[renderer] = 0;
          newRenderingRequest = true;
        }
        else
        {
          // NO: Make sure that LOD-increase is currently not blocked
          // (by mouse-movement)
          if ( !lodIncreaseBlocked )
          {
            // Check if the maximum LOD level has already been reached
            if ( nextLODMap[renderer] < maxLOD )
            {
              // NO: increase the level for this renderer...
              nextLODMap[renderer]++;

              // ... and request new update for this window
              newRenderingRequest = true;
            }
            else
            {
              // YES: Reset to level 0 for next rendering request (by user)
              nextLODMap[renderer] = 0;
            }
          }
        }

        // Issue events queued during rendering (abort mechanism)
        GetInstance()->DoFinishAbortRendering();

        // Post new rendering request only at the end to give DoFinishAbortRendering
        // the chance to process other events first!
        if ( newRenderingRequest )
        {
          GetInstance()->RequestUpdate( renderer->GetRenderWindow() );
        }
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
::SetNextLOD( unsigned int lod, BaseRenderer *renderer )
{
  unsigned int newLOD = lod < m_MaxLOD ? lod : m_MaxLOD;

  if ( renderer != NULL )
  {
    m_NextLODMap[renderer] = newLOD;
  }
  else
  {
    // Set next LOD for all renderers
    RenderWindowList::iterator it;
    for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
    {
      m_NextLODMap[BaseRenderer::GetInstance( it->first )] = newLOD;
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

void RenderingManager::SetDepthPeelingEnabled( bool enabled )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer = mitk::BaseRenderer::GetInstance( it->first );
    baseRenderer->SetDepthPeelingEnabled(enabled);
  }
}

void RenderingManager::SetMaxNumberOfPeels( int maxNumber )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    mitk::BaseRenderer *baseRenderer = mitk::BaseRenderer::GetInstance( it->first );
    baseRenderer->SetMaxNumberOfPeels(maxNumber);
  }
}
// Create and register generic RenderingManagerFactory.
GenericRenderingManagerFactory renderingManagerFactory;


} // namespace
