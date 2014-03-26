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

#include <sstream>
#include <iomanip>

#include "mitkGeometry3D.h"

#include "mitkRotationOperation.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkApplyTransformMatrixOperation.h"
#include "mitkPointOperation.h"
#include "mitkInteractionConst.h"
#include <vtkMatrixToLinearTransform.h>
#include <vtkMatrix4x4.h>
#include "mitkMatrixConvert.h"

// Standard constructor for the New() macro. Sets the geometry to 3 dimensions
mitk::Geometry3D::Geometry3D() :  BaseGeometry()
{
}
mitk::Geometry3D::Geometry3D(const Geometry3D& other) : BaseGeometry(other)
{
}

mitk::Geometry3D::~Geometry3D()
{
}

itk::LightObject::Pointer mitk::Geometry3D::InternalClone() const
{
  Self::Pointer newGeometry = new Self(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}

void mitk::Geometry3D::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  os << indent << " IndexToWorldTransform: ";
  if(this->IsIndexToWorldTransformNull())
    os << "NULL" << std::endl;
  else
  {
    // from itk::MatrixOffsetTransformBase
    unsigned int i, j;
    os << std::endl;
    os << indent << "Matrix: " << std::endl;
    for (i = 0; i < 3; i++)
    {
      os << indent.GetNextIndent();
      for (j = 0; j < 3; j++)
      {
        os << this->GetIndexToWorldTransform()->GetMatrix()[i][j] << " ";
      }
      os << std::endl;
    }

    os << indent << "Offset: " << this->GetIndexToWorldTransform()->GetOffset() << std::endl;
    os << indent << "Center: " << this->GetIndexToWorldTransform()->GetCenter() << std::endl;
    os << indent << "Translation: " << this->GetIndexToWorldTransform()->GetTranslation() << std::endl;

    os << indent << "Inverse: " << std::endl;
    for (i = 0; i < 3; i++)
    {
      os << indent.GetNextIndent();
      for (j = 0; j < 3; j++)
      {
        os << this->GetIndexToWorldTransform()->GetInverseMatrix()[i][j] << " ";
      }
      os << std::endl;
    }

    // from itk::ScalableAffineTransform
    os << indent << "Scale : ";
    for (i = 0; i < 3; i++)
    {
      os << this->GetIndexToWorldTransform()->GetScale()[i] << " ";
    }
    os << std::endl;
  }

  os << indent << " BoundingBox: ";
  if(this->IsBoundingBoxNull())
    os << "NULL" << std::endl;
  else
  {
    os << indent << "( ";
    for (unsigned int i=0; i<3; i++)
    {
      os << this->GetBoundingBox()->GetBounds()[2*i] << "," << this->GetBoundingBox()->GetBounds()[2*i+1] << " ";
    }
    os << " )" << std::endl;
  }

  os << indent << " Origin: " << this->GetOrigin() << std::endl;
  os << indent << " ImageGeometry: " << this->GetImageGeometry() << std::endl;
  os << indent << " Spacing: " << this->GetSpacing() << std::endl;
  os << indent << " TimeBounds: " << this->GetTimeBounds() << std::endl;
}
