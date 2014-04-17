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

#include "mitkIRenderingManager.h"

namespace mitk {
struct RenderingManagerInterface : public IRenderingManager
{
  RenderingManagerInterface(RenderingManager::Pointer manager)
    : m_RM(manager)
  {}

  QList<vtkRenderWindow*> GetAllRegisteredVtkRenderWindows() const
  {
    RenderingManager::RenderWindowVector vec(m_RM->GetAllRegisteredRenderWindows());
    QList<vtkRenderWindow*> result;
    for (RenderingManager::RenderWindowVector::iterator i = vec.begin();
         i != vec.end(); ++i)
    {
      result.append(*i);
    }
    return result;
  }

  void RequestUpdate( vtkRenderWindow *renderWindow )
  {
    m_RM->RequestUpdate(renderWindow);
  }

  void ForceImmediateUpdate( vtkRenderWindow *renderWindow )
  {
    m_RM->ForceImmediateUpdate(renderWindow);
  }

  void RequestUpdateAll( RenderingManager::RequestType type )
  {
    m_RM->RequestUpdateAll(type);
  }

  void ForceImmediateUpdateAll( RenderingManager::RequestType type )
  {
    m_RM->ForceImmediateUpdateAll(type);
  }

  bool InitializeViews( const BaseGeometry *geometry,
                        RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL,
                        bool preserveRoughOrientationInWorldSpace = false )
  {
    return m_RM->InitializeViews( geometry, type, preserveRoughOrientationInWorldSpace);
  }

  bool InitializeViews( const TimeGeometry *geometry,
                        RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL,
                        bool preserveRoughOrientationInWorldSpace = false )
  {
    return m_RM->InitializeViews( geometry, type, preserveRoughOrientationInWorldSpace);
  }

  bool InitializeViews( RenderingManager::RequestType type )
  {
    return m_RM->InitializeViews(type);
  }

  bool InitializeView( vtkRenderWindow *renderWindow, const BaseGeometry *geometry,
                               bool initializeGlobalTimeSNC = false)
  {
    return m_RM->InitializeView(renderWindow, geometry, initializeGlobalTimeSNC);
  }

  bool InitializeView( vtkRenderWindow *renderWindow )
  {
    return m_RM->InitializeView(renderWindow);
  }

  const SliceNavigationController *GetTimeNavigationController() const
  {
    return m_RM->GetTimeNavigationController();
  }

  SliceNavigationController *GetTimeNavigationController()
  {
    return m_RM->GetTimeNavigationController();
  }

  bool IsRendering() const
  {
    return m_RM->IsRendering();
  }

  void AbortRendering()
  {
    m_RM->AbortRendering();
  }

  void SetLODIncreaseBlocked(bool blocked)
  {
    m_RM->SetLODIncreaseBlocked(blocked);
  }

  bool GetLODIncreaseBlocked() const
  {
    return m_RM->GetLODIncreaseBlocked();
  }

  void SetLODAbortMechanismEnabled(bool abort)
  {
    m_RM->SetLODAbortMechanismEnabled(abort);
  }

  bool GetLODAbortMechanismEnabled() const
  {
    return m_RM->GetLODAbortMechanismEnabled();
  }

  int GetNextLOD( BaseRenderer* renderer ) const
  {
    return m_RM->GetNextLOD(renderer);
  }

  void SetMaximumLOD( unsigned int max )
  {
    m_RM->SetMaximumLOD(max);
  }

  void SetShading( bool state, unsigned int lod )
  {
    m_RM->SetShading(state, lod);
  }

  bool GetShading( unsigned int lod )
  {
    return m_RM->GetShading(lod);
  }

  void SetClippingPlaneStatus( bool status )
  {
    m_RM->SetClippingPlaneStatus(status);
  }

  bool GetClippingPlaneStatus()
  {
    return m_RM->GetClippingPlaneStatus();
  }

  void SetShadingValues( float ambient, float diffuse,
                         float specular, float specpower )
  {
    m_RM->SetShadingValues(ambient, diffuse, specular, specpower);
  }

  QList<float> GetShadingValues() const
  {
    RenderingManager::FloatVector vec(m_RM->GetShadingValues());
    QList<float> result;
    for (RenderingManager::FloatVector::iterator i = vec.begin();
         i != vec.end(); ++i)
    {
      result.push_back(*i);
    }
    return result;
  }

  const RenderingManager::Pointer m_RM;
};

IRenderingManager* MakeRenderingManagerInterface(RenderingManager::Pointer manager)
{
  return new RenderingManagerInterface(manager);
}
}