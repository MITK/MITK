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


#ifndef CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722
#define CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722


#include <MitkCoreExports.h>
#include "mitkBaseController.h"

namespace mitk {

class KeyEvent;

//##Documentation
//## @brief controls the camera used by the associated BaseRenderer
//##
//## Subclass of BaseController. Controls the camera used by the associated
//## BaseRenderer.
//## @ingroup NavigationControl
class MITKCORE_EXPORT CameraController : public BaseController
{
public:
    enum StandardView { ANTERIOR,POSTERIOR,SINISTER,DEXTER,CRANIAL,CAUDAL };

    mitkClassMacro(CameraController, BaseController);
    //mitkNewMacro1Param(Self, const char*);
    itkNewMacro(Self);


    void SetRenderer(const BaseRenderer* renderer)
    {
      m_Renderer = renderer;
    };
    itkGetConstMacro(Renderer,const BaseRenderer*);


   virtual void SetViewToAnterior();
   virtual void SetViewToPosterior();
   virtual void SetViewToSinister();
   virtual void SetViewToDexter();
   virtual void SetViewToCranial();
   virtual void SetViewToCaudal();
   virtual void SetStandardView(StandardView view);

   /**
    * @brief Fit Adjust the camera, so that the world bounding box is fully visible.
    */
   void Fit();

   /**
    * @brief Set the desired zoom-level to the absolute value given.
    */
   void SetScaleFactorInMMPerDisplayUnit( ScalarType scale );

   /**
    * @brief MoveCameraToPoint Move camera so that the point on the plane is in the view center.
    * @param planePoint
    */
   void MoveCameraToPoint(const Point2D &planePoint, bool useConstraints = true);

   // Move by moveVector in display coordinates
   void MoveBy(const Vector2D &moveVector);

   void Zoom(ScalarType factor, const Point2D &zoomPointInMM);

   Point2D GetCameraPositionOnPlane();

   /**
    * @brief AdjustCameraToPlane Moves the camera of a 2D Renderwindow without panning or
    * zooming, eg. only rotate the camera.
    */
   void AdjustCameraToPlane();

protected:

    /**
    * @brief Default Constructor
    **/
    CameraController();

    /**
    * @brief Default Destructor
    **/
    virtual ~CameraController();
    const BaseRenderer* m_Renderer;

    ScalarType ComputeMaxParallelScale();
    ScalarType ComputeMaxAllowedParallelScale();

private:


    /**
     * @brief AdjustCameraToPlane Moves the camera of a 2D Renderwindow without panning or
     * zooming, eg. only rotate the camera.
     * @param PlanePoint point where the camera is moved to during the adjustment.
     */
    void AdjustCameraToPlane(const Point2D& PlanePoint, bool useConstraints = true);

    void AdjustConstrainedCameraPosition(Point2D& planePoint);

    const float ALLOWED_TO_MAX_ZOOM_COF = 3.;

};

} // namespace mitk



#endif /* CAMERACONTROLLER_H_HEADER_INCLUDED_C1C53722 */
