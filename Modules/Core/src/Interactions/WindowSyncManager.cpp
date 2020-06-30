#include "WindowSyncManager.h"

#include <mitkBaseRenderer.h>
#include <mitkCameraController.h>
#include <mitkLevelWindowProperty.h>
#include <mitkSliceNavigationController.h>

WindowSyncManager::WindowSyncManager()
{
  syncSlice       = true;
  syncSliceDelta  = false;
  syncCamera      = false;
  syncLevelWindow = true;
}

void WindowSyncManager::setSlicer(mitk::BaseRenderer* renderer, mitk::SliceNavigationController* controller)
{
  m_Slicers[renderer] = controller;
}

void WindowSyncManager::setLevelProperty(mitk::BaseRenderer* renderer, mitk::LevelWindowProperty* levelProp)
{
  m_Levels[renderer] = levelProp;
}

void WindowSyncManager::setCamera(mitk::BaseRenderer* renderer, mitk::CameraController* camera)
{
  m_Cameras[renderer] = camera;
}

void WindowSyncManager::removeWindow(mitk::BaseRenderer* renderer)
{
  m_Slicers.erase(renderer);
  m_Levels.erase(renderer);
  m_Cameras.erase(renderer);
}

void WindowSyncManager::updateSlicers(mitk::Point3D& p)
{
  for (const auto& slicer : m_Slicers) {
    slicer.second->SelectSliceByPoint(p);
    slicer.first->RequestUpdate();
  }
}

void WindowSyncManager::updateCameraMove(mitk::Vector2D& moveVector)
{
  for (const auto& camera : m_Cameras) {
    camera.second->MoveBy(moveVector);
    camera.first->RequestUpdate();
  }
}

void WindowSyncManager::updateCameraZoom(float& factor, mitk::Point2D& start)
{
  for (const auto& camera : m_Cameras) {
    camera.second->Zoom(factor, start);
    camera.first->RequestUpdate();
  }
}

void WindowSyncManager::updateSlicersDeltaNext()
{
  for (const auto& slicer : m_Slicers) {
    slicer.second->GetSlice()->Next();
    slicer.first->RequestUpdate();
  }
}

void WindowSyncManager::updateSlicersDeltaPrevious()
{
  for (const auto& slicer : m_Slicers) {
    slicer.second->GetSlice()->Previous();
    slicer.first->RequestUpdate();
  }
}

void WindowSyncManager::updateSlicersDeltaComponentNext()
{
  for (const auto& slicer : m_Slicers) {
    slicer.second->GetComponent()->Next();
    slicer.first->RequestUpdate();
  }
}

void WindowSyncManager::updateSlicersDeltaComponentPrevious()
{
  for (const auto& slicer : m_Slicers) {
    slicer.second->GetComponent()->Previous();
    slicer.first->RequestUpdate();
  }
}

void WindowSyncManager::updateLevelWindow(mitk::LevelWindowProperty* prop)
{
  for (const auto& levelWindow : m_Levels) {
    levelWindow.second->SetValue(prop->GetValue());
    levelWindow.first->RequestUpdate();
  }
}

bool WindowSyncManager::rendererInCameraSync(mitk::BaseRenderer* renderer)
{
  return m_Cameras.find(renderer) != m_Cameras.end();
}

bool WindowSyncManager::rendererInSlicersSync(mitk::BaseRenderer* renderer)
{
  return m_Slicers.find(renderer) != m_Slicers.end();
}

void WindowSyncManager::removeLevelProperty(mitk::BaseRenderer* renderer)
{
  m_Levels.erase(renderer);
}
