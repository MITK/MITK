/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVtkLogoRepresentation_h
#define mitkVtkLogoRepresentation_h

#include <vtkLogoRepresentation.h>

/**
 * \brief Extended VTK logo representation with configurable corner placement.
 *
 * Extends vtkLogoRepresentation to allow positioning the logo in a specific
 * corner of the render window (bottom-left, bottom-right, top-right, top-left,
 * or center).
 *
 * \sa LogoAnnotation
 */
class mitkVtkLogoRepresentation : public vtkLogoRepresentation
{
public:
  /** \brief Instantiate this class. */
  static mitkVtkLogoRepresentation *New();

  vtkTypeMacro(mitkVtkLogoRepresentation, vtkLogoRepresentation);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  /** \brief Build the representation geometry. */
  void BuildRepresentation() override;

  /** \brief Set the corner position for the logo placement.
   *
   * 0 = Bottom left, 1 = Bottom right, 2 = Top right, 3 = Top left, 4 = Center.
   */
  void SetCornerPosition(int corner) { cornerPosition = corner; }
protected:
  mitkVtkLogoRepresentation();
  ~mitkVtkLogoRepresentation() override;

  // Helper methods
  void AdjustImageSize(double o[2], double borderSize[2], double imageSize[2]) override;

  int cornerPosition;

private:
  mitkVtkLogoRepresentation(const mitkVtkLogoRepresentation &); // Not implemented
  void operator=(const mitkVtkLogoRepresentation &);            // Not implemented
};

#endif
