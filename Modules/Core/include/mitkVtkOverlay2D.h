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

#ifndef VTKOVERLAY2D_H
#define VTKOVERLAY2D_H

#include <MitkCoreExports.h>
#include "mitkVtkOverlay.h"
#include <vtkSmartPointer.h>

class vtkActor2D;
class vtkProperty2D;

namespace mitk {

/**
 * @brief The VtkOverlay2D class is the basis for all VTK based Overlays which create
 * a vtkActor2D element that will be drawn on the renderer.
 */
class MITKCORE_EXPORT VtkOverlay2D : public VtkOverlay {
public:

  mitkClassMacro(VtkOverlay2D, VtkOverlay);
  virtual Overlay::Bounds GetBoundsOnDisplay(BaseRenderer *renderer) const override;
  virtual void SetBoundsOnDisplay(BaseRenderer *renderer, const Bounds& bounds) override;

  void SetPosition2D(const Point2D& position2D, mitk::BaseRenderer* renderer = NULL);

  Point2D GetPosition2D(mitk::BaseRenderer* renderer = NULL) const;

  void SetOffsetVector(const Point2D& OffsetVector, BaseRenderer* renderer = NULL);

  Point2D GetOffsetVector(mitk::BaseRenderer* renderer = NULL) const;

protected:

  virtual vtkProp* GetVtkProp(BaseRenderer *renderer) const override;
  virtual void UpdateVtkOverlay(BaseRenderer *renderer) override;
  virtual void UpdateVtkOverlay2D(BaseRenderer *renderer) = 0;
  virtual vtkActor2D* GetVtkActor2D(BaseRenderer *renderer) const = 0;

  /** \brief explicit constructor which disallows implicit conversions */
  explicit VtkOverlay2D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~VtkOverlay2D();

private:

  /** \brief copy constructor */
  VtkOverlay2D( const VtkOverlay2D &);

  /** \brief assignment operator */
  VtkOverlay2D &operator=(const VtkOverlay2D &);

};

} // namespace mitk
#endif // VTKOVERLAY2D_H


