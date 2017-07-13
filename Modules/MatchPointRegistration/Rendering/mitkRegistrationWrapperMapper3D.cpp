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

