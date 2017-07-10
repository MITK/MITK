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

#include "mitkVtkSmartVolumeMapper3D.h"

void mitk::VtkSmartVolumeMapper3D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  mitk::Image *input = const_cast<mitk::Image *>(static_cast<const mitk::Image *>(this->GetDataNode()->GetData()));
  vtkImageData *inputData = input->GetVtkImageData(this->GetTimestep());
}

vtkProp* mitk::VtkSmartVolumeMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return nullptr;
}

void mitk::VtkSmartVolumeMapper3D::ApplyProperties(vtkActor *actor, mitk::BaseRenderer *renderer)
{

}

void mitk::VtkSmartVolumeMapper3D::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{

}

void mitk::VtkSmartVolumeMapper3D::MitkRenderVolumetricGeometry(mitk::BaseRenderer *renderer)
{

}

mitk::VtkSmartVolumeMapper3D::VtkSmartVolumeMapper3D()
{

}

mitk::VtkSmartVolumeMapper3D::~VtkSmartVolumeMapper3D()
{

}