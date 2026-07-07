/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCameraController_h
#define mitkCameraController_h

#include <mitkBaseController.h>
#include <MitkCoreExports.h>

namespace mitk
{
  class KeyEvent;

  /**
   * \brief Controls the camera used by the associated BaseRenderer.
   *
   * Subclass of BaseController. Provides methods for setting standard
   * anatomical views, fitting the view to the world bounding box, zooming,
   * panning, and adjusting the camera to the current plane geometry.
   *
   * \ingroup NavigationControl
   * \sa BaseController BaseRenderer
   */
  class MITKCORE_EXPORT CameraController : public BaseController
  {
  public:
    /** \brief Enumeration of standard anatomical viewing directions. */
    enum StandardView
    {
      ANTERIOR,  ///< View from the front (ventral).
      POSTERIOR,  ///< View from the back (dorsal).
      SINISTER,  ///< View from the left.
      DEXTER,  ///< View from the right.
      CRANIAL,  ///< View from above (superior).
      CAUDAL  ///< View from below (inferior).
    };

    mitkClassMacro(CameraController, BaseController);
    // mitkNewMacro1Param(Self, const char*);
    itkNewMacro(Self);

    /** \brief Set the renderer whose camera this controller manages. */
    void SetRenderer(const BaseRenderer *renderer) { m_Renderer = renderer; };
    itkGetConstMacro(Renderer, const BaseRenderer *);

    /** \brief Set the camera to an anterior (front) view. */
    virtual void SetViewToAnterior();
    /** \brief Set the camera to a posterior (back) view. */
    virtual void SetViewToPosterior();
    /** \brief Set the camera to a sinister (left) view. */
    virtual void SetViewToSinister();
    /** \brief Set the camera to a dexter (right) view. */
    virtual void SetViewToDexter();
    /** \brief Set the camera to a cranial (top) view. */
    virtual void SetViewToCranial();
    /** \brief Set the camera to a caudal (bottom) view. */
    virtual void SetViewToCaudal();

    /**
     * \brief Set the camera to one of the standard anatomical views.
     * \param view The StandardView direction to apply.
     */
    virtual void SetStandardView(StandardView view);

    /**
     * \brief Adjust the camera so that the world bounding box is fully visible.
     */
    void Fit();

    /**
     * \brief Set the desired zoom level to an absolute value.
     * \param scale The scale factor in millimeters per display unit.
     */
    void SetScaleFactorInMMPerDisplayUnit(ScalarType scale);

    /**
     * \brief Move the camera so that the given point on the plane is in the view center.
     * \param planePoint The target point in plane coordinates (mm).
     */
    void MoveCameraToPoint(const Point2D &planePoint);

    /**
     * \brief Move the camera by the given vector (in mm).
     * \param moveVectorInMM The translation vector in millimeters.
     */
    void MoveBy(const Vector2D &moveVectorInMM);

    /**
     * \brief Zoom the camera by the given factor around the specified point.
     * \param factor The zoom factor (>1 zooms in, <1 zooms out, <=0 is ignored).
     * \param zoomPointInMM The point around which to zoom, in plane coordinates (mm).
     */
    void Zoom(ScalarType factor, const Point2D &zoomPointInMM);

    /**
     * \brief Get the current camera position projected onto the plane.
     * \return The camera position in plane coordinates (mm).
     */
    Point2D GetCameraPositionOnPlane();

    /**
     * \brief Adjust the camera orientation to match the current plane without panning or zooming.
     *
     * This rotates the camera so it is aligned with the current world plane geometry.
     */
    void AdjustCameraToPlane();

  protected:
    /** \brief Default constructor. */
    CameraController();

    /** \brief Destructor. */
    ~CameraController() override;

    /** \brief The renderer whose camera is controlled. */
    const BaseRenderer *m_Renderer;

    /**
     * \brief Compute the maximum parallel scale that fits the entire plane into the viewport.
     * \return The maximum parallel scale value in mm.
     */
    ScalarType ComputeMaxParallelScale();

  private:
    /**
     * \brief Adjust the camera orientation and move it to the specified plane point.
     *
     * Moves the camera of a 2D render window without changing zoom level,
     * only rotating and translating to match the current plane geometry.
     *
     * \param PlanePoint The point on the plane where the camera is positioned.
     */
    void AdjustCameraToPlane(const Point2D &PlanePoint);

    /**
     * \brief Clamp the given plane point to stay within the visible image bounds.
     * \param planePoint The point to constrain (modified in-place).
     */
    void AdjustConstrainedCameraPosition(Point2D &planePoint);
  };

} // namespace mitk

#endif
