#pragma once

#include <MitkCoreExports.h>

#include <unordered_map>

#include <mitkPoint.h>
#include <mitkVector.h>

namespace mitk {
  class BaseRenderer;
  class CameraController;
  class SliceNavigationController;
  class LevelWindowProperty;
}

class MITKCORE_EXPORT WindowSyncManager {
public:
  static WindowSyncManager& getInstance() {
    static WindowSyncManager instance;
    return instance;
  }

  bool syncSlice;
  bool syncSliceDelta;
  bool syncCamera;
  bool syncLevelWindow;

  WindowSyncManager(WindowSyncManager const&) = delete;
  void operator=(WindowSyncManager const&)    = delete;

  void removeWindow(mitk::BaseRenderer* renderer);

  void setSlicer(mitk::BaseRenderer* renderer, mitk::SliceNavigationController* controller);
  void setLevelProperty(mitk::BaseRenderer* renderer, mitk::LevelWindowProperty* levelProp);
  void removeLevelProperty(mitk::BaseRenderer* renderer);
  void setCamera(mitk::BaseRenderer* renderer, mitk::CameraController* camera);

  // Special api for very specialy designed mitk architecture
  void updateSlicersDeltaNext();
  void updateSlicersDeltaPrevious();
  void updateSlicersDeltaComponentNext();
  void updateSlicersDeltaComponentPrevious();

  void updateSlicers(mitk::Point3D& point);
  void updateCameraMove(mitk::Vector2D& moveVector);
  void updateCameraZoom(float& factor, mitk::Point2D& start);
  void updateLevelWindow(mitk::LevelWindowProperty* prop);

  bool rendererInCameraSync(mitk::BaseRenderer* renderer);
  bool rendererInSlicersSync(mitk::BaseRenderer* renderer);

protected:
  std::unordered_map<mitk::BaseRenderer*, mitk::SliceNavigationController*> m_Slicers;
  std::unordered_map<mitk::BaseRenderer*, mitk::CameraController*> m_Cameras;
  std::unordered_map<mitk::BaseRenderer*, mitk::LevelWindowProperty*> m_Levels;

private:
  WindowSyncManager();
  ~WindowSyncManager(){};
};

