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


#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>

#include <mitkProperties.h>
#include <mitkExceptionMacro.h>
#include <mitkException.h>

#include "mitkMAPRegistrationWrapper.h"
#include "mitkRegistrationWrapperMapper2D.h"
#include "mitkRegVisHelper.h"

mitk::MITKRegistrationWrapperMapper2D::MITKRegistrationWrapperMapper2D()
{

}


mitk::MITKRegistrationWrapperMapper2D::~MITKRegistrationWrapperMapper2D()
{

}

bool mitk::MITKRegistrationWrapperMapper2D::RendererGeometryIsOutdated(mitk::BaseRenderer *renderer, const itk::TimeStamp& time) const
{
  return (time < renderer->GetCurrentWorldPlaneGeometryUpdateTime()) //was the geometry modified?
    || (time < renderer->GetCurrentWorldPlaneGeometry()->GetMTime());
}

bool mitk::MITKRegistrationWrapperMapper2D::GetGeometryDescription( mitk::BaseRenderer *renderer, mitk::BaseGeometry::ConstPointer& gridDesc, unsigned int& gridFrequ ) const
{
  mitk::PlaneGeometry::ConstPointer worldGeometry = renderer->GetCurrentWorldPlaneGeometry();

  if( ( worldGeometry.IsNull() ) || ( !worldGeometry->IsValid() ) || ( !worldGeometry->HasReferenceGeometry() ))
  {
    return false;
  }

  mitk::PlaneGeometry::Pointer newDesc = worldGeometry->Clone();

  mitk::Geometry3D::Pointer dummyDesc;
  mitk::GetGridGeometryFromNode(this->GetDataNode(), dummyDesc, gridFrequ);

  mitk::Vector3D spacing = worldGeometry->GetSpacing();
  spacing[0] = dummyDesc->GetSpacing()[0];
  spacing[1] = dummyDesc->GetSpacing()[1];

  mitk::Geometry3D::BoundsArrayType bounds = newDesc->GetBounds();
  bounds[1] *= (newDesc->GetSpacing()[0]/spacing[0]);
  bounds[3] *= (newDesc->GetSpacing()[1]/spacing[1]);
  newDesc->SetBounds(bounds);
  newDesc->SetSpacing(spacing);


  gridDesc = newDesc;

  return true;
};

