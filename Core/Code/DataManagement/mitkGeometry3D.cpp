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
mitk::Geometry3D::Geometry3D() :  m_ImageGeometry(false)
{
}
mitk::Geometry3D::Geometry3D(const Geometry3D& other) : BaseGeometry(other), m_ImageGeometry(other.m_ImageGeometry), m_ParametricBoundingBox(other.m_ParametricBoundingBox)
{
  if (other.m_ParametricBoundingBox.IsNotNull())
  {
    m_ParametricBoundingBox = other.m_ParametricBoundingBox->DeepCopy();
  }
}

mitk::Geometry3D::~Geometry3D()
{
}

void mitk::Geometry3D::SetParametricBounds(const BoundingBox::BoundsArrayType& bounds)
{
  m_ParametricBoundingBox = BoundingBoxType::New();

  BoundingBoxType::PointsContainer::Pointer pointscontainer =
    BoundingBoxType::PointsContainer::New();
  BoundingBoxType::PointType p;
  BoundingBoxType::PointIdentifier pointid;

  for(pointid=0; pointid<2;++pointid)
  {
    unsigned int i;
    for(i=0; i<GetNDimensions(); ++i)
    {
      p[i] = bounds[2*i+pointid];
    }
    pointscontainer->InsertElement(pointid, p);
  }

  m_ParametricBoundingBox->SetPoints(pointscontainer);
  m_ParametricBoundingBox->ComputeBoundingBox();
  this->Modified();
}

itk::LightObject::Pointer mitk::Geometry3D::InternalClone() const
{
  Self::Pointer newGeometry = new Self(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}

void mitk::Geometry3D::InternPostInitializeGeometry(Geometry3D * newGeometry) const
{
  newGeometry->m_ImageGeometry = m_ImageGeometry;
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
  os << indent << " ImageGeometry: " << m_ImageGeometry << std::endl;
  os << indent << " Spacing: " << this->GetSpacing() << std::endl;
  os << indent << " TimeBounds: " << this->GetTimeBounds() << std::endl;
}

mitk::Point3D mitk::Geometry3D::GetCornerPoint(int id) const
{
  assert(id >= 0);
  assert(this->IsBoundingBoxNull()==false);

  BoundingBox::BoundsArrayType bounds = this->GetBoundingBox()->GetBounds();

  Point3D cornerpoint;
  switch(id)
  {
  case 0: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[4]); break;
  case 1: FillVector3D(cornerpoint, bounds[0],bounds[2],bounds[5]); break;
  case 2: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[4]); break;
  case 3: FillVector3D(cornerpoint, bounds[0],bounds[3],bounds[5]); break;
  case 4: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[4]); break;
  case 5: FillVector3D(cornerpoint, bounds[1],bounds[2],bounds[5]); break;
  case 6: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[4]); break;
  case 7: FillVector3D(cornerpoint, bounds[1],bounds[3],bounds[5]); break;
  default:
    {
      itkExceptionMacro(<<"A cube only has 8 corners. These are labeled 0-7.");
    }
  }
  if(m_ImageGeometry)
  {
    // Here i have to adjust the 0.5 offset manually, because the cornerpoint is the corner of the
    // bounding box. The bounding box itself is no image, so it is corner-based
    FillVector3D(cornerpoint, cornerpoint[0]-0.5, cornerpoint[1]-0.5, cornerpoint[2]-0.5);
  }
  return this->GetIndexToWorldTransform()->TransformPoint(cornerpoint);
}

mitk::Point3D mitk::Geometry3D::GetCornerPoint(bool xFront, bool yFront, bool zFront) const
{
  assert(this->IsBoundingBoxNull()==false);
  BoundingBox::BoundsArrayType bounds = this->GetBoundingBox()->GetBounds();

  Point3D cornerpoint;
  cornerpoint[0] = (xFront ? bounds[0] : bounds[1]);
  cornerpoint[1] = (yFront ? bounds[2] : bounds[3]);
  cornerpoint[2] = (zFront ? bounds[4] : bounds[5]);
  if(m_ImageGeometry)
  {
    // Here i have to adjust the 0.5 offset manually, because the cornerpoint is the corner of the
    // bounding box. The bounding box itself is no image, so it is corner-based
    FillVector3D(cornerpoint, cornerpoint[0]-0.5, cornerpoint[1]-0.5, cornerpoint[2]-0.5);
  }

  return this->GetIndexToWorldTransform()->TransformPoint(cornerpoint);
}

void
  mitk::Geometry3D::ChangeImageGeometryConsideringOriginOffset( const bool isAnImageGeometry )
{
  // If Geometry is switched to ImageGeometry, you have to put an offset to the origin, because
  // imageGeometries origins are pixel-center-based
  // ... and remove the offset, if you switch an imageGeometry back to a normal geometry
  // For more information please see the Geometry documentation page

  if(m_ImageGeometry == isAnImageGeometry)
    return;

  const BoundingBox::BoundsArrayType& boundsarray =
    this->GetBoundingBox()->GetBounds();

  Point3D  originIndex;
  FillVector3D(originIndex,  boundsarray[0], boundsarray[2], boundsarray[4]);

  if(isAnImageGeometry == true)
    FillVector3D( originIndex,
    originIndex[0] + 0.5,
    originIndex[1] + 0.5,
    originIndex[2] + 0.5 );
  else
    FillVector3D( originIndex,
    originIndex[0] - 0.5,
    originIndex[1] - 0.5,
    originIndex[2] - 0.5 );

  Point3D originWorld;

  originWorld = GetIndexToWorldTransform()
    ->TransformPoint( originIndex );
  // instead could as well call  IndexToWorld(originIndex,originWorld);

  SetOrigin(originWorld);

  this->SetImageGeometry(isAnImageGeometry);
}

bool mitk::Equal(const mitk::Geometry3D *leftHandSide, const mitk::Geometry3D *rightHandSide, ScalarType eps, bool verbose)
{
  bool result = true;

  if( rightHandSide == NULL )
  {
    if(verbose)
      MITK_INFO << "[( Geometry3D )] rightHandSide NULL.";
    return false;
  }
  if( leftHandSide == NULL)
  {
    if(verbose)
      MITK_INFO << "[( Geometry3D )] leftHandSide NULL.";
    return false;
  }

  //Compare spacings
  if( !mitk::Equal( leftHandSide->GetSpacing(), rightHandSide->GetSpacing(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Spacing differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetSpacing() << " : leftHandSide is " << leftHandSide->GetSpacing() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Origins
  if( !mitk::Equal( leftHandSide->GetOrigin(), rightHandSide->GetOrigin(), eps ) )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] Origin differs.";
      MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetOrigin() << " : leftHandSide is " << leftHandSide->GetOrigin() << " and tolerance is " << eps;
    }
    result = false;
  }

  //Compare Axis and Extents
  for( unsigned int i=0; i<3; ++i)
  {
    if( !mitk::Equal( leftHandSide->GetAxisVector(i), rightHandSide->GetAxisVector(i), eps))
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D )] AxisVector #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetAxisVector(i) << " : leftHandSide is " << leftHandSide->GetAxisVector(i) << " and tolerance is " << eps;
      }
      result =  false;
    }

    if( !mitk::Equal( leftHandSide->GetExtent(i), rightHandSide->GetExtent(i), eps) )
    {
      if(verbose)
      {
        MITK_INFO << "[( Geometry3D )] Extent #" << i << " differ";
        MITK_INFO << "rightHandSide is " << setprecision(12) << rightHandSide->GetExtent(i) << " : leftHandSide is " << leftHandSide->GetExtent(i) << " and tolerance is " << eps;
      }
      result = false;
    }
  }

  //Compare ImageGeometry Flag
  if( rightHandSide->GetImageGeometry() != leftHandSide->GetImageGeometry() )
  {
    if(verbose)
    {
      MITK_INFO << "[( Geometry3D )] GetImageGeometry is different.";
      MITK_INFO << "rightHandSide is " << rightHandSide->GetImageGeometry() << " : leftHandSide is " << leftHandSide->GetImageGeometry();
    }
    result = false;
  }

  //Compare BoundingBoxes
  if( !mitk::Equal( leftHandSide->GetBoundingBox(), rightHandSide->GetBoundingBox(), eps, verbose) )
  {
    result = false;
  }

  //Compare IndexToWorldTransform Matrix
  if( !mitk::Equal( leftHandSide->GetIndexToWorldTransform(), rightHandSide->GetIndexToWorldTransform(), eps, verbose) )
  {
    result = false;
  }
  return result;
}

void mitk::Geometry3D::InternPostInitialize()
{
  m_ImageGeometry = false;
}
