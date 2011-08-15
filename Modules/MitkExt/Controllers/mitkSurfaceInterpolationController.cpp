#include "mitkSurfaceInterpolationController.h"


mitk::SurfaceInterpolationController::SurfaceInterpolationController()
{
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  //for (unsigned int i = 0; i < m_PositionList.size(); i++)
  //{
  //  delete m_PositionList.at(i);
  //}
}

mitk::SurfaceInterpolationController* mitk::SurfaceInterpolationController::GetInstance()
{
  static mitk::SurfaceInterpolationController* m_Instance;

  if ( m_Instance == 0)
  {
    m_Instance = new SurfaceInterpolationController();
  }

  return m_Instance;
}
