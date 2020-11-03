/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include <vtkPropAssembly.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <vtkCellArray.h>

#include <mitkProperties.h>
#include <mitkExceptionMacro.h>
#include <mitkException.h>

#include "mitkMAPRegistrationWrapper.h"
#include "mitkRegistrationWrapperMapper3D.h"
#include "mitkRegVisColorStyleProperty.h"
#include "mitkRegVisHelper.h"
#include "mitkRegVisPropertyTags.h"
#include "mitkRegVisHelper.h"

mitk::MITKRegistrationWrapperMapper3D::MITKRegistrationWrapperMapper3D()
{

}


mitk::MITKRegistrationWrapperMapper3D::~MITKRegistrationWrapperMapper3D()
{

}

bool mitk::MITKRegistrationWrapperMapper3D::RendererGeometryIsOutdated(mitk::BaseRenderer*, const itk::TimeStamp&) const
{
  return false;
}

bool mitk::MITKRegistrationWrapperMapper3D::GetGeometryDescription(mitk::BaseRenderer*, mitk::BaseGeometry::ConstPointer& gridDesc, unsigned int& gridFrequ) const
{
  mitk::Geometry3D::Pointer newDesc;
  mitk::GetGridGeometryFromNode(this->GetDataNode(),newDesc,gridFrequ);
  gridDesc = newDesc;
  return true;
};

