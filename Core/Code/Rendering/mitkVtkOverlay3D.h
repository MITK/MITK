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

#ifndef VTKOVERLAY3D_H
#define VTKOVERLAY3D_H

#include <MitkCoreExports.h>
#include "mitkVtkOverlay.h"
#include <vtkSmartPointer.h>


namespace mitk {

/**
 * @brief The VtkOverlay3D class is the basis for all VTK based Overlays which create
 * any 3D element as a vtkProp that will be drawn on the renderer.
 */
class MITKCORE_EXPORT VtkOverlay3D : public VtkOverlay {
public:

  void SetPosition3D(const Point3D& position3D, mitk::BaseRenderer* renderer = NULL);

  Point3D GetPosition3D(mitk::BaseRenderer* renderer = NULL) const;

  void SetOffsetVector(const Point3D& OffsetVector, mitk::BaseRenderer* renderer = NULL);

  Point3D GetOffsetVector(mitk::BaseRenderer* renderer = NULL) const;

  mitkClassMacro(VtkOverlay3D, VtkOverlay);

protected:

  virtual void UpdateVtkOverlay(BaseRenderer *renderer) = 0;

  /** \brief explicit constructor which disallows implicit conversions */
  explicit VtkOverlay3D();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~VtkOverlay3D();

private:

  /** \brief copy constructor */
  VtkOverlay3D( const VtkOverlay3D &);

  /** \brief assignment operator */
  VtkOverlay3D &operator=(const VtkOverlay3D &);

};

} // namespace mitk
#endif // OVERLAY_H


