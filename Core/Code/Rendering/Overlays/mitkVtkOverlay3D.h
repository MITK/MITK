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

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkVtkOverlay.h"
#include "mitkBaseRenderer.h"
#include <vtkSmartPointer.h>


namespace mitk {

class MITK_CORE_EXPORT VtkOverlay3D : public VtkOverlay {
public:

  void SetPosition3D(Point3D position3D);

  void SetPosition3D(Point3D position3D, mitk::BaseRenderer* renderer);

  Point3D GetPosition3D(mitk::BaseRenderer* renderer);

  Point3D GetPosition3D();

  Overlay::Bounds GetBoundsOnDisplay(BaseRenderer*);
  void SetBoundsOnDisplay(BaseRenderer*, Overlay::Bounds);

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


