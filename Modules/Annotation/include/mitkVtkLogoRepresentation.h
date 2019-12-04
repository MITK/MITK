/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __mitkVtkLogoRepresentation_h
#define __mitkVtkLogoRepresentation_h

#include "vtkLogoRepresentation.h"

class mitkVtkLogoRepresentation : public vtkLogoRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static mitkVtkLogoRepresentation *New();

  // Description:
  // Standard VTK class methods.
  vtkTypeMacro(mitkVtkLogoRepresentation, vtkLogoRepresentation);
  void PrintSelf(ostream &os, vtkIndent indent) override;

  // Description:
  // Satisfy the superclasses' API.
  void BuildRepresentation() override;

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
