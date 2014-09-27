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
  vtkTypeMacro(mitkVtkLogoRepresentation,vtkLogoRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Satisfy the superclasses' API.
  virtual void BuildRepresentation();

  void SetCornerPosition(int corner){cornerPosition = corner;}

protected:
  mitkVtkLogoRepresentation();
  ~mitkVtkLogoRepresentation();

  // Helper methods
  void AdjustImageSize(double o[2], double borderSize[2], double imageSize[2]);

  int cornerPosition;

private:
  mitkVtkLogoRepresentation(const mitkVtkLogoRepresentation&);  //Not implemented
  void operator=(const mitkVtkLogoRepresentation&);  //Not implemented
};

#endif
