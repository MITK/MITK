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
#include "mitkOverlay.h"
#include <vtkSmartPointer.h>

class vtkProp;

namespace mitk {

/**
 * @brief The VtkOverlay class is the base for all Overlays which are using the VTK framework to render
 *the elements.
 */
class MITK_CORE_EXPORT VtkOverlay : public Overlay {
public:

  mitkClassMacro(VtkOverlay, Overlay);
  void Update(BaseRenderer *renderer);
  void AddToBaseRenderer(BaseRenderer *renderer);
  void RemoveFromBaseRenderer(BaseRenderer *renderer);

protected:

  /**
   * @brief This method is implemented by the specific VTKOverlays in order to create the element as a vtkProp
   * @param renderer
   * @return The element that was created by the subclasses as a vtkProp.
   */
  virtual vtkProp* GetVtkProp(BaseRenderer *renderer) const = 0;
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


