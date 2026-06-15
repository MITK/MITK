/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAnatomicalPlanes_h
#define mitkAnatomicalPlanes_h

namespace mitk
{
  /**
   * \brief Enumeration of standard anatomical image planes.
   *
   * Used to specify the orientation of 2D image slices in medical imaging.
   */
  enum class AnatomicalPlane
  {
    Axial,    ///< Transverse plane (top-down view, perpendicular to body's long axis).
    Sagittal, ///< Plane dividing the body into left and right portions.
    Coronal,  ///< Frontal plane dividing the body into anterior and posterior portions.
    Original  ///< The image's native acquisition plane.
  };
}

#endif
