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

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkVtkOverlay.h"
#include "mitkBaseRenderer.h"
#include <vtkSmartPointer.h>
#include <vtkActor2D.h>
#include <vtkProperty2D.h>


namespace mitk {

class MITK_CORE_EXPORT VtkOverlay2D : public VtkOverlay {
public:

  mitkClassMacro(VtkOverlay2D, VtkOverlay);
  virtual Overlay::Bounds GetBoundsOnDisplay(BaseRenderer *renderer);
  virtual void SetBoundsOnDisplay(BaseRenderer *renderer, Overlay::Bounds bounds);

protected:

  vtkSmartPointer<vtkProp> GetVtkProp(BaseRenderer *renderer);
  virtual void UpdateVtkOverlay(BaseRenderer *renderer);
  virtual void UpdateVtkOverlay2D(BaseRenderer *renderer) = 0;
  virtual vtkSmartPointer<vtkActor2D> GetVtkActor2D(BaseRenderer *renderer) = 0;

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


