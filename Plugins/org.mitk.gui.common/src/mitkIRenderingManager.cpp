/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkIRenderingManager.h"

namespace mitk {
struct RenderingManagerInterface : public IRenderingManager
{
  RenderingManagerInterface(RenderingManager::Pointer manager)
    : m_RM(manager)
  {}

  QList<vtkRenderWindow*> GetAllRegisteredVtkRenderWindows() const override
  {
    RenderingManager::RenderWindowVector vec(m_RM->GetAllRegisteredRenderWindows());
    QList<vtkRenderWindow*> result;
    for (auto i = vec.begin();
         i != vec.end(); ++i)
    {
      result.append(*i);
    }
    return result;
  }

  void RequestUpdate( vtkRenderWindow *renderWindow ) override
  {
    m_RM->RequestUpdate(renderWindow);
  }

  void ForceImmediateUpdate( vtkRenderWindow *renderWindow ) override
  {
    m_RM->ForceImmediateUpdate(renderWindow);
  }

  void RequestUpdateAll( RenderingManager::RequestType type ) override
  {
    m_RM->RequestUpdateAll(type);
  }

  void ForceImmediateUpdateAll( RenderingManager::RequestType type ) override
  {
    m_RM->ForceImmediateUpdateAll(type);
  }

  bool InitializeViews( const BaseGeometry *geometry,
                        RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL,
                        bool preserveRoughOrientationInWorldSpace = false ) override
  {
    return m_RM->InitializeViews( geometry, type, preserveRoughOrientationInWorldSpace);
  }

  bool InitializeViews( const TimeGeometry *geometry,
                        RenderingManager::RequestType type = RenderingManager::REQUEST_UPDATE_ALL,
                        bool preserveRoughOrientationInWorldSpace = false ) override
  {
    return m_RM->InitializeViews( geometry, type, preserveRoughOrientationInWorldSpace);
  }

  bool InitializeViews( RenderingManager::RequestType type ) override
  {
    return m_RM->InitializeViews(type);
  }

  bool InitializeView( vtkRenderWindow *renderWindow, const BaseGeometry *geometry,
                               bool initializeGlobalTimeSNC = false) override
  {
    return m_RM->InitializeView(renderWindow, geometry, initializeGlobalTimeSNC);
  }

  bool InitializeView( vtkRenderWindow *renderWindow ) override
  {
    return m_RM->InitializeView(renderWindow);
  }

  const SliceNavigationController *GetTimeNavigationController() const override
  {
    return m_RM->GetTimeNavigationController();
  }

  SliceNavigationController *GetTimeNavigationController() override
  {
    return m_RM->GetTimeNavigationController();
  }

  bool IsRendering() const override
  {
    return m_RM->IsRendering();
  }

  void AbortRendering() override
  {
    m_RM->AbortRendering();
  }

  void SetLODIncreaseBlocked(bool blocked) override
  {
    m_RM->SetLODIncreaseBlocked(blocked);
  }

  bool GetLODIncreaseBlocked() const override
  {
    return m_RM->GetLODIncreaseBlocked();
  }

  void SetLODAbortMechanismEnabled(bool abort) override
  {
    m_RM->SetLODAbortMechanismEnabled(abort);
  }

  bool GetLODAbortMechanismEnabled() const override
  {
    return m_RM->GetLODAbortMechanismEnabled();
  }

  int GetNextLOD( BaseRenderer* renderer ) const override
  {
    return m_RM->GetNextLOD(renderer);
  }

  void SetMaximumLOD( unsigned int max ) override
  {
    m_RM->SetMaximumLOD(max);
  }

  void SetShading( bool state, unsigned int lod ) override
  {
    m_RM->SetShading(state, lod);
  }

  bool GetShading( unsigned int lod ) override
  {
    return m_RM->GetShading(lod);
  }

  void SetClippingPlaneStatus( bool status ) override
  {
    m_RM->SetClippingPlaneStatus(status);
  }

  bool GetClippingPlaneStatus() override
  {
    return m_RM->GetClippingPlaneStatus();
  }

  void SetShadingValues( float ambient, float diffuse,
                         float specular, float specpower ) override
  {
    m_RM->SetShadingValues(ambient, diffuse, specular, specpower);
  }

  QList<float> GetShadingValues() const override
  {
    RenderingManager::FloatVector vec(m_RM->GetShadingValues());
    QList<float> result;
    for (auto i = vec.begin();
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
