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
#include "mitkRenderWindow.h"
#include "mitkOpenGLRenderer.h"

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
  s_RenderWindowList[renderWindow] = 0;
}

void
mitk::RenderingManager
::RemoveRenderWindow( RenderWindow *renderWindow )
{
  m_RenderWindowList.erase( renderWindow );
  if(s_Instance == this)
  {
    s_RenderWindowList.erase( renderWindow );
  }
}

void
mitk::RenderingManager
::RequestUpdate( RenderWindow *renderWindow )
{
  s_RenderWindowList[renderWindow] = 2;
  
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
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    if ( it->second > 0 )
    {
      m_UpdatePending = true;
    }
  }

}

void
mitk::RenderingManager
::ForceImmediateUpdate( RenderWindow *renderWindow )
{
  bool onlyOverlay = (( m_RenderWindowList[renderWindow] == 1 ) && ( s_RenderWindowList[renderWindow] == 1 )) ?true:false;

  // Immediately repaint this window (implementation platform specific)
  renderWindow->Repaint(onlyOverlay);

  // Erase potentially pending requests for this window
  m_RenderWindowList[renderWindow] = 0;
  s_RenderWindowList[renderWindow] = 0;

  CheckUpdatePending();
  if(s_Instance != this)
    GetInstance()->CheckUpdatePending();

  // Stop the timer if no more requests are pending
  if ( !m_UpdatePending )
  {
    this->StopTimer();
  }
}

void
mitk::RenderingManager
::RequestUpdateAll()
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    it->second = 2;
    s_RenderWindowList[it->first] = 2;
  }

  // Restart the timer if there are no requests already
  if ( !m_UpdatePending )
  {
    m_UpdatePending = true;
    this->RestartTimer();
  }
}

void
mitk::RenderingManager
::ForceImmediateUpdateAll()
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int& globalRequest = s_RenderWindowList[it->first];
    bool onlyOverlay = (( it->second == 1 ) && ( globalRequest == 1 )) ?true:false;

    // Erase potentially pending requests
    it->second = 0;
    globalRequest = 0;

    // Immediately repaint this window (implementation platform specific)
    it->first->Repaint(onlyOverlay);
  }

  if ( m_UpdatePending )
  {
    this->StopTimer();
    m_UpdatePending = false;
  }

  if(s_Instance != this)
    GetInstance()->CheckUpdatePending();
}

void 
mitk::RenderingManager
::ForceImmediateUpdateIncludingVtkActors()
{
  RenderWindowList::iterator it;
  for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
  {
    int& globalRequest = s_RenderWindowList[it->first];
    bool onlyOverlay = (( it->second == 1 ) && ( globalRequest == 1 )) ?true:false;

    // Erase potentially pending requests
    it->second = 0;
    globalRequest = 0;
    
    // if the render window is rendered via an mitk::OpenGLRenderer
    // call UpdateIncludingVtkActors. 
    mitk::OpenGLRenderer* openGLRenderer = dynamic_cast<mitk::OpenGLRenderer*>( it->first->GetRenderer() );
    if ( openGLRenderer )
      openGLRenderer->UpdateIncludingVtkActors();
    it->first->Repaint(onlyOverlay);
  }

  if ( m_UpdatePending )
  {
    this->StopTimer();
    m_UpdatePending = false;
  }

  if(s_Instance != this)
    GetInstance()->CheckUpdatePending();
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
  if(s_Instance == this)
  {
    RenderWindowList::iterator it;
    for ( it = s_RenderWindowList.begin(); it != s_RenderWindowList.end(); ++it )
    {
      if ( it->second )
      {
        this->ForceImmediateUpdate( it->first );
        it->second = 0;
      }
    }
  }
  else
  {
    RenderWindowList::iterator it;
    for ( it = m_RenderWindowList.begin(); it != m_RenderWindowList.end(); ++it )
    {
      if ( it->second )
      {
        this->ForceImmediateUpdate( it->first );
        it->second = 0;
      }
    }
  }
}

void
mitk::RenderingManager
::RequestOverlayUpdate( RenderWindow *renderWindow )
{

  // don't distroy rendering request
  if( s_RenderWindowList[renderWindow] < 1 && m_RenderWindowList[renderWindow] < 1 )
  {
    s_RenderWindowList[renderWindow] = 1;
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
      s_RenderWindowList[it->first] = 1;
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

// Create and register generic RenderingManagerFactory.
mitk::GenericRenderingManagerFactory renderingManagerFactory;
