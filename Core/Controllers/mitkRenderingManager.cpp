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

/*! \class RenderingManager

   A short explanation of the s_RenderWindowList[rendererWindow] map. There are some magic numbers
   involved in the signalling of an update request. These numbers are stored per render window inside
   the static STL map s_RenderWindowList. The numbers are:

   - 0 for "no update requested"
   - 1 for "overlay update requested"
   - 2 for "normal update requested"
   - 3 for "normal update, plus calling the UpdateIncludingVtkActors() method of all involved OpenGLRenderers"

   These numbers should perhaps be replaced by some constants...

*/

#include "mitkRenderingManager.h"
#include "mitkRenderingManagerFactory.h"
#include "mitkRenderWindow.h"
#include "mitkVtkRenderWindow.h"
#include "mitkOpenGLRenderer.h"

#include <itkCommand.h>
#include <algorithm>

mitk::RenderingManager::RenderWindowList mitk::RenderingManager::s_RenderWindowList;
mitk::RenderingManager::Pointer mitk::RenderingManager::s_Instance = 0;
mitk::RenderingManagerFactory *mitk::RenderingManager::s_RenderingManagerFactory = 0;


void
mitk::RenderingManager
::SetFactory( RenderingManagerFactory *factory )
{
  s_RenderingManagerFactory = factory;
}

const mitk::RenderingManagerFactory * 
mitk::RenderingManager
::GetFactory()
{
  return s_RenderingManagerFactory;
}

bool
mitk::RenderingManager
::HasFactory()
{
  if ( RenderingManager::s_RenderingManagerFactory )
    return true;
  else
    return false;
}


mitk::RenderingManager::Pointer mitk::RenderingManager::New()
{
  const mitk::RenderingManagerFactory* factory = GetFactory();
  if(factory == NULL)
    return NULL;
  return factory->CreateRenderingManager();
}

mitk::RenderingManager *
mitk::RenderingManager
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
mitk::RenderingManager
::IsInstantiated()
{
  if ( RenderingManager::s_Instance )
    return true;
  else
    return false;
}

mitk::RenderingManager::RenderingManager()
: m_UpdatePending( false ), m_Interval( 10 )
{
  m_ShadingEnabled.resize(3);
  m_ShadingValues.resize(4);
  // The default (minimum) interval is 10 msec, theoretically enabling a
  // maximum frame rate of 100 Hz.
}

mitk::RenderingManager::~RenderingManager()
{
}

void
mitk::RenderingManager
::AddRenderWindow( RenderWindow *renderWindow )
{
  m_RenderWindowList[renderWindow] = 0;
  
  m_AllRenderWindows.push_back( renderWindow );

  typedef itk::MemberCommand< RenderingManager > MemberCommandType;

  mitk::BaseRenderer* renderer = renderWindow->GetRenderer();

  if(renderer){
   MemberCommandType::Pointer startCallbackCommand = MemberCommandType::New();
   startCallbackCommand->SetCallbackFunction(
    this, &RenderingManager::RenderingStartCallback
  );
  renderer->AddObserver( itk::StartEvent(), startCallbackCommand );

  MemberCommandType::Pointer progressCallbackCommand = MemberCommandType::New();
  progressCallbackCommand->SetCallbackFunction(
    this, &RenderingManager::RenderingProgressCallback
  );
  renderer->AddObserver( itk::ProgressEvent(), progressCallbackCommand );

  MemberCommandType::Pointer endCallbackCommand = MemberCommandType::New();
  endCallbackCommand->SetCallbackFunction( 
    this, &RenderingManager::RenderingEndCallback
  );
  renderer->AddObserver( itk::EndEvent(), endCallbackCommand );

  m_IsRendering[renderWindow] = false;
 }



}

void
mitk::RenderingManager
::RemoveRenderWindow( RenderWindow *renderWindow )
{
  m_RenderWindowList.erase( renderWindow );

  RenderWindowVector::iterator thisRenderWindowsPosition = std::find( m_AllRenderWindows.begin(), m_AllRenderWindows.end(), renderWindow );
  if ( thisRenderWindowsPosition != m_AllRenderWindows.end() )
  {
    m_AllRenderWindows.erase( thisRenderWindowsPosition );
  }
}

const mitk::RenderingManager::RenderWindowVector&
mitk::RenderingManager
::GetAllRegisteredRenderWindows()
{
  return m_AllRenderWindows;
}

mitk::RenderWindow* 
mitk::RenderingManager::
GetRenderWindowByName(const std::string& name)
{
  for (RenderWindowVector::iterator iter = m_AllRenderWindows.begin();
       iter != m_AllRenderWindows.end();
       ++iter)
  {
    if ( name == (*iter)->GetName() )
    {
      return *iter;
    }
  }

  return NULL;
}

void
mitk::RenderingManager
::RequestUpdate( RenderWindow *renderWindow )
{
  if (m_IsRendering[renderWindow])
  {
    this->AbortRendering( renderWindow );
  }

  if ( m_RenderWindowList[renderWindow] < 2 )
  {
    m_RenderWindowList[renderWindow] = 2;
  }

  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}

void
mitk::RenderingManager
::CheckUpdatePending()
{
  // Check if there are pending requests for any other windows
  m_UpdatePending = false;
  RenderWindowList::iterator it;
  m_Numberof3DRW = 0;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    //std::cout<<"it->second: "<<it->second<<std::endl;
    if ( it->second > 0 )
    {
      m_UpdatePending = true;
    }
    if(it->first->GetRenderer()->GetMapperID() == 2)
    {
      m_Numberof3DRW++;
    }
  }
  //std::cout<<std::endl;
}

void
mitk::RenderingManager
::ForceImmediateUpdate( RenderWindow *renderWindow )
{
  bool onlyOverlay =        (( m_RenderWindowList[renderWindow] == 1 )) ? true:false;
  bool includingVtkActors = (( m_RenderWindowList[renderWindow] == 3 )) ? true:false;

  if ( includingVtkActors ) // TODO temporary fix until bug 167 (new vtk-based rendering mechanism) is done 
  {
    // if the render window is rendered via an mitk::OpenGLRenderer
    // call UpdateIncludingVtkActors. 
    OpenGLRenderer* openGLRenderer = dynamic_cast<OpenGLRenderer*>( renderWindow->GetRenderer() );
    if ( openGLRenderer ) openGLRenderer->UpdateIncludingVtkActors();

  }

  // Erase potentially pending requests for this window
  m_RenderWindowList[renderWindow] = 0;

  this->CheckUpdatePending();

  // Stop the timer if no more requests are pending
  if ( !m_UpdatePending )
  {
    this->StopTimer();
    
    /** Level-Of-Detail **/
    if(m_Numberof3DRW > 1)
    {
      if(m_CurrentLOD < m_MaxLOD)
      {
        int nextLOD = m_CurrentLOD+1;
        SetCurrentLOD(nextLOD);
        this->RequestUpdateAll3D();
      }
    }
  }

  // Immediately repaint this window (implementation platform specific)
  renderWindow->Repaint(onlyOverlay);
}

void
mitk::RenderingManager
::RequestUpdateAll( bool includeVtkActors ) // TODO temporary fix until bug 167 (new vtk-based rendering mechanism) is done 
{
  
  int magicUpdateFlag = includeVtkActors?3:2;
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second < magicUpdateFlag )
    {
      it->second = magicUpdateFlag;
    }

    this->RequestUpdate( it->first);

    if ( m_RenderWindowList[it->first] < magicUpdateFlag )
    {
      m_RenderWindowList[it->first] = magicUpdateFlag;
    }
  }

  // Restart the timer if there are no requests already
  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}

void mitk::RenderingManager::RequestUpdateAll3D(bool includeVtkActors)
{
  int magicUpdateFlag = includeVtkActors?3:2;
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if(it->first->GetRenderer()->GetMapperID() == 2) //if RenderWindow uses a 3D Mapper
    {
      if ( it->second < magicUpdateFlag )
      {
        it->second = magicUpdateFlag;
      }

      this->RequestUpdate( it->first);

      if ( m_RenderWindowList[it->first] < magicUpdateFlag )
      {
        m_RenderWindowList[it->first] = magicUpdateFlag;
      }
    }
  }
}

void
mitk::RenderingManager
::ForceImmediateUpdateAll()
{ 
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    bool onlyOverlay = ( it->second == 1 ) ?true:false;

    // Erase potentially pending requests
    it->second = 0;

    // Immediately repaint this window (implementation platform specific)
    it->first->Repaint(onlyOverlay);
  }

  if ( m_UpdatePending )
  {
    this->StopTimer();
    m_UpdatePending = false;
  }

  this->CheckUpdatePending();
}

void 
mitk::RenderingManager
::ForceImmediateUpdateIncludingVtkActors()
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    bool onlyOverlay = ( it->second == 1 ) ? true:false;

    // Erase potentially pending requests
    it->second = 0;
    
    // if the render window is rendered via an mitk::OpenGLRenderer
    // call UpdateIncludingVtkActors. 
    OpenGLRenderer* openGLRenderer = dynamic_cast<OpenGLRenderer*>( it->first->GetRenderer() );
    if ( openGLRenderer )
      openGLRenderer->UpdateIncludingVtkActors();
    it->first->Repaint(onlyOverlay);
  }

  if ( m_UpdatePending )
  {
    this->StopTimer();
    m_UpdatePending = false;
  }

  this->CheckUpdatePending();
}

void
mitk::RenderingManager
::SetMinimumInterval( int msec )
{
  if ( msec < 0 )
  {
    msec = 0;
  }
  if ( msec > 5000 )
  {
    msec = 5000;
  }

  m_Interval = msec;
}

int
mitk::RenderingManager
::GetMinimumInterval() const
{
  return m_Interval;
}

void
mitk::RenderingManager
::UpdateCallback()
{
 m_UpdatePending = false;

  // Satisfy all pending update requests
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second )
    {
      this->ForceImmediateUpdate( it->first );
    }
  }
}

void
mitk::RenderingManager
::RequestOverlayUpdate( RenderWindow *renderWindow )
{

  // don't distroy rendering request
  if ( m_RenderWindowList[renderWindow] < 1 )
  {
    m_RenderWindowList[renderWindow] = 1;
  }
  
  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}

void
mitk::RenderingManager
::RequestOverlayUpdateAll( )
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if( it->second < 1 )
    {
      it->second = 1;
    }
  }

  // Restart the timer if there are no requests already
  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}


namespace mitk
{
class GenericRenderingManager : public RenderingManager
{
public:
  mitkClassMacro(GenericRenderingManager,RenderingManager);
  itkNewMacro(Self);

protected:
  virtual void RestartTimer()
  {
  };

  virtual void StopTimer()
  {
  };
};

class GenericRenderingManagerFactory : public mitk::RenderingManagerFactory
{
public:
  GenericRenderingManagerFactory()
  {
    if ( !mitk::RenderingManager::HasFactory() )
    {
      mitk::RenderingManager::SetFactory( this );
    }
  };

  virtual ~GenericRenderingManagerFactory() {};

  virtual mitk::RenderingManager::Pointer CreateRenderingManager() const
  {
    GenericRenderingManager::Pointer specificSmartPtr = GenericRenderingManager::New();
    RenderingManager::Pointer smartPtr = specificSmartPtr.GetPointer();
    return smartPtr;
  };
};
}

void
mitk::RenderingManager
::RenderingStartCallback( itk::Object* object, const itk::EventObject& /*event*/ )
{ 
  mitk::BaseRenderer* renderer = dynamic_cast< mitk::BaseRenderer* >( object );
  if (renderer)
  { 
    //std::cout<<"S "<<std::endl;
    m_IsRendering[renderer->GetRenderWindow()] = true;
  }
}

void
mitk::RenderingManager
::RenderingProgressCallback( itk::Object* /*object*/, const itk::EventObject& /*event*/ )
{
  this->DoMonitorRendering();

}

void
mitk::RenderingManager
::RenderingEndCallback( itk::Object* object, const itk::EventObject& /*event*/ )
{ 
  mitk::BaseRenderer* renderer = dynamic_cast< mitk::BaseRenderer* >( object );
  if (renderer)
  {
    //std::cout<<"E "<<std::endl;
    m_IsRendering[renderer->GetRenderWindow()] = false;
    this->DoFinishAbortRendering();
    
    /** Level-Of-Detail **/
    if(m_Numberof3DRW == 1)
    {
      if(m_CurrentLOD < m_MaxLOD)
      {
        int nextLOD = m_CurrentLOD+1;
        SetCurrentLOD(nextLOD);
        this->RequestUpdateAll3D();
      }
    }
  }
}

bool
mitk::RenderingManager
::IsRendering() const
{
  bool rendering = false;

  RenderWindowList::const_iterator it;
  for ( it = m_IsRendering.begin(); it != m_IsRendering.end(); ++it )
  {
    if ( it->second )
    {
      rendering = true;
    }
  }

  return rendering;
}

void
mitk::RenderingManager
::AbortRendering( mitk::RenderWindow* renderWindow )
{ 
  //std::cout<<" abort "<<std::endl;

  if ( renderWindow && m_IsRendering[renderWindow] )
  { 
    renderWindow->GetVtkRenderWindow()->SetAbortRender( true );
    m_UpdatePending = true;
  }
  else
  {
    RenderWindowList::iterator it;
    for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
    {
      if ( m_IsRendering[it->first] )
      {
        it->first->GetVtkRenderWindow()->SetAbortRender( true );       
      }
    }
  }
}

int mitk::RenderingManager::GetCurrentLOD() //returns the LOD requested for the next rendering
{ 
  return m_CurrentLOD;
}

void mitk::RenderingManager::SetCurrentLOD(int lod) //sets the LOD requested for the next rendering
{ 
  //std::cout<<"SetCurrentLOD("<<lod<<")"<<std::endl;
  m_CurrentLOD=lod;
}

void mitk::RenderingManager::SetNumberOfLOD(int number)
{
  m_MaxLOD = number-1;
}

//enable/disable shading
void mitk::RenderingManager::SetShading(bool state, int lod)
{ 
  m_ShadingEnabled[lod] = state;

}

bool mitk::RenderingManager::GetShading(int lod)
{ 
  return m_ShadingEnabled[lod];
}

//enable/disable the clipping plane
void mitk::RenderingManager::SetClippingPlaneStatus(bool status)
{
  m_ClippingPlaneStatus = status;
}

bool mitk::RenderingManager::GetClippingPlaneStatus()
{
  return m_ClippingPlaneStatus;
}

void mitk::RenderingManager::SetShadingValues(float ambient, float diffuse, float specular, float specpower)
{
  m_ShadingValues[0] = ambient;
  m_ShadingValues[1] = diffuse;
  m_ShadingValues[2] = specular;
  m_ShadingValues[3] = specpower;
}

std::vector<float> mitk::RenderingManager::GetShadingValues()
{
  return m_ShadingValues;
}


// Create and register generic RenderingManagerFactory.
mitk::GenericRenderingManagerFactory renderingManagerFactory;
