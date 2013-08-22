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

#ifndef VTKOVERLAY_H
#define VTKOVERLAY_H

#include <MitkExports.h>
#include <itkObject.h>
#include <mitkCommon.h>
#include "mitkOverlay.h"
#include "mitkBaseRenderer.h"
#include <vtkSmartPointer.h>
#include <vtkProp.h>


namespace mitk {

class MITK_CORE_EXPORT VtkOverlay : public Overlay {
public:

  mitkClassMacro(VtkOverlay, Overlay);
  void UpdateOverlay(BaseRenderer *renderer);
  void AddOverlay(BaseRenderer *renderer);
  void RemoveOverlay(BaseRenderer *renderer);

protected:

  virtual vtkProp* GetVtkProp(BaseRenderer *renderer) = 0;
  virtual void UpdateVtkOverlay(BaseRenderer *renderer) = 0;

  /** \brief explicit constructor which disallows implicit conversions */
  explicit VtkOverlay();

  /** \brief virtual destructor in order to derive from this class */
  virtual ~VtkOverlay();

private:

  /** \brief copy constructor */
  VtkOverlay( const VtkOverlay &);

  /** \brief assignment operator */
  VtkOverlay &operator=(const VtkOverlay &);

};

} // namespace mitk
#endif // OVERLAY_H


