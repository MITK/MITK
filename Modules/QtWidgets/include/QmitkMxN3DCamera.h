/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMxN3DCamera_h
#define QmitkMxN3DCamera_h

#include <MitkQtWidgetsExports.h>

#include <mitkAnatomicalPlanes.h>
#include <mitkVector.h>

#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkWeakPointer.h>

#include <functional>

class vtkCamera;
class vtkRenderer;

namespace mitk
{
  class BaseGeometry;
  class PlaneGeometry;
}

/** \brief The camera quantities that 3D cells of the MxN editor synchronize. */
struct QmitkMxN3DCameraState
{
  mitk::Point3D focalPoint;
  double visibleHalfHeight = 0.0;
  mitk::Vector3D directionOfProjection;
  mitk::Vector3D viewUp;
};

/**
* \brief Interactor style of a 3D cell of the MxN editor.
*
*   VTK's trackball navigation (rotate, spin, pan, dolly), plus the detection
*   of camera rotations: when an interaction ends with a direction of
*   projection or a view-up different from the one it started with (beyond
*   'QmitkMxN3DCamera::Tolerance'), the rotation callback runs. Pan and dolly
*   change neither, so they never report a rotation. While the rotation lock
*   reports true, rotate and spin do nothing; pan and dolly stay. Every
*   camera change of an interaction is reported in steps, which is what the
*   camera synchronization of 3D cells relays.
*/
class MITKQTWIDGETS_EXPORT QmitkMxN3DInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static QmitkMxN3DInteractorStyle* New();
  vtkTypeMacro(QmitkMxN3DInteractorStyle, vtkInteractorStyleTrackballCamera);

  using RotationCallback = std::function<void()>;
  using RotationLock = std::function<bool()>;

  using CameraState = QmitkMxN3DCameraState;

  /** \brief Receives one step of an interaction, from the camera the
   *         previous step ended with to the current one. */
  using CameraStepCallback = std::function<void(const CameraState& from, const CameraState& to)>;

  /** \brief The renderer whose active camera the detection compares. */
  void SetObservedRenderer(vtkRenderer* renderer);

  /** \brief Called once per interaction that rotated the camera. */
  void SetRotationCallback(RotationCallback callback);

  /** \brief Asked at every rotate and spin step whether the camera may turn. */
  void SetRotationLock(RotationLock lock);

  /**
  * \brief Called for every step of an interaction that changed the camera.
  *
  *   Steps follow each other without gaps: the first starts at the camera
  *   the interaction started with, the last ends at the camera it ended
  *   with, so the steps of one interaction add up to its whole change. An
  *   interaction without intermediate steps (a mouse wheel turn) reports
  *   one step when it ends.
  */
  void SetCameraStepCallback(CameraStepCallback callback);

  void Rotate() override;
  void Spin() override;

protected:
  QmitkMxN3DInteractorStyle();
  ~QmitkMxN3DInteractorStyle() override;

private:
  QmitkMxN3DInteractorStyle(const QmitkMxN3DInteractorStyle&) = delete;
  void operator=(const QmitkMxN3DInteractorStyle&) = delete;

  static void OnInteractionEvent(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);

  void ReportCameraStep();

  vtkWeakPointer<vtkRenderer> m_ObservedRenderer;
  RotationCallback m_RotationCallback;
  RotationLock m_RotationLock;
  CameraStepCallback m_CameraStepCallback;
  bool m_InteractionStarted = false;
  CameraState m_LastReportedState;
  double m_StartDirectionOfProjection[3] = { 0.0, 0.0, 0.0 };
  double m_StartViewUp[3] = { 0.0, 0.0, 0.0 };
};

/**
* \brief Camera placement of 3D cells of the MxN editor.
*/
namespace QmitkMxN3DCamera
{
  /** \brief Tolerance for comparing unit vectors (direction of projection, view-up). */
  constexpr double Tolerance = 1e-6;

  /**
  * \brief Look at 'plane' the way a 2D cell showing it does: the camera
  *        looks along minus the plane normal, with the plane's second axis
  *        as view-up.
  *
  *   The camera rotates about its focal point; focal point and distance are
  *   kept and the clipping range is reset. Does nothing when the camera
  *   already looks that way.
  *
  * \return Whether the camera changed.
  */
  MITKQTWIDGETS_EXPORT bool ApplyViewFrame(vtkRenderer* renderer, const mitk::PlaneGeometry& plane);

  /**
  * \brief Turn the camera to look along 'directionOfProjection' with
  *        'viewUp', about its focal point; focal point and distance are kept
  *        and the clipping range is reset. Does nothing when the camera
  *        already looks that way.
  *
  * \return Whether the camera changed.
  */
  MITKQTWIDGETS_EXPORT bool ApplyDirection(vtkRenderer* renderer,
                                           const mitk::Vector3D& directionOfProjection,
                                           const mitk::Vector3D& viewUp);

  /** \brief The camera quantities 3D cells synchronize. */
  MITKQTWIDGETS_EXPORT QmitkMxN3DCameraState CaptureState(vtkCamera* camera);

  /**
  * \brief Half the height of the world a 3D camera shows at its focal point.
  *
  *   The zoom measure 3D cells share with 2D cells, whose equivalent is the
  *   camera's parallel scale: for a perspective camera the distance times
  *   the tangent of half the view angle, for a parallel projection the
  *   parallel scale.
  */
  MITKQTWIDGETS_EXPORT double VisibleHalfHeight(vtkCamera* camera);

  /**
  * \brief Zoom a 3D camera in by 'factor' (> 1 magnifies): the visible
  *        half-height is divided by it. A perspective camera dollies toward
  *        its focal point. The clipping range is reset.
  */
  MITKQTWIDGETS_EXPORT void Zoom(vtkRenderer* renderer, double factor);

  /** \brief Move camera and focal point by 'worldVector'; the clipping range is reset. */
  MITKQTWIDGETS_EXPORT void Translate(vtkRenderer* renderer, const mitk::Vector3D& worldVector);

  /**
  * \brief 'planeVector', given in the coordinates of 'plane' (millimeters
  *        along its first and second axis, like 2D camera positions), as a
  *        world vector.
  */
  MITKQTWIDGETS_EXPORT mitk::Vector3D PlaneToWorld(const mitk::PlaneGeometry& plane, const mitk::Vector2D& planeVector);

  /** \brief The part of 'worldVector' that lies in 'plane', in plane coordinates (see PlaneToWorld). */
  MITKQTWIDGETS_EXPORT mitk::Vector2D WorldToPlane(const mitk::PlaneGeometry& plane, const mitk::Vector3D& worldVector);

  /**
  * \brief The standard plane (axial, sagittal or coronal) a slice
  *        navigation controller would build for 'geometry' whose normal is
  *        closest to 'directionOfProjection' (either sign).
  */
  MITKQTWIDGETS_EXPORT mitk::AnatomicalPlane NearestStandardPlane(const mitk::BaseGeometry& geometry,
                                                                  const mitk::Vector3D& directionOfProjection);
}

#endif
