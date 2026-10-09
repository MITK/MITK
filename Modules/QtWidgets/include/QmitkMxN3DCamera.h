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

class vtkRenderer;

namespace mitk
{
  class BaseGeometry;
  class PlaneGeometry;
}

/**
* \brief Interactor style of a 3D cell of the MxN editor.
*
*   VTK's trackball navigation (rotate, spin, pan, dolly), plus the detection
*   of camera rotations: when an interaction ends with a direction of
*   projection or a view-up different from the one it started with (beyond
*   'QmitkMxN3DCamera::Tolerance'), the rotation callback runs. Pan and dolly
*   change neither, so they never report a rotation.
*/
class MITKQTWIDGETS_EXPORT QmitkMxN3DInteractorStyle : public vtkInteractorStyleTrackballCamera
{
public:
  static QmitkMxN3DInteractorStyle* New();
  vtkTypeMacro(QmitkMxN3DInteractorStyle, vtkInteractorStyleTrackballCamera);

  using RotationCallback = std::function<void()>;

  /** \brief The renderer whose active camera the detection compares. */
  void SetObservedRenderer(vtkRenderer* renderer);

  /** \brief Called once per interaction that rotated the camera. */
  void SetRotationCallback(RotationCallback callback);

protected:
  QmitkMxN3DInteractorStyle();
  ~QmitkMxN3DInteractorStyle() override;

private:
  QmitkMxN3DInteractorStyle(const QmitkMxN3DInteractorStyle&) = delete;
  void operator=(const QmitkMxN3DInteractorStyle&) = delete;

  static void OnInteractionEvent(vtkObject* caller, unsigned long eventId, void* clientData, void* callData);

  vtkWeakPointer<vtkRenderer> m_ObservedRenderer;
  RotationCallback m_RotationCallback;
  bool m_InteractionStarted = false;
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
  * \brief The standard plane (axial, sagittal or coronal) a slice
  *        navigation controller would build for 'geometry' whose normal is
  *        closest to 'directionOfProjection' (either sign).
  */
  MITKQTWIDGETS_EXPORT mitk::AnatomicalPlane NearestStandardPlane(const mitk::BaseGeometry& geometry,
                                                                  const mitk::Vector3D& directionOfProjection);
}

#endif
