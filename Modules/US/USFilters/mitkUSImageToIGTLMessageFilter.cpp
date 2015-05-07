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

#include <arpa/inet.h>

#include "mitkUSImageToIGTLMessageFilter.h"
#include "igtlImageMessage.h"

mitk::USImageToIGTLMessageFilter::USImageToIGTLMessageFilter()
{
  mitk::IGTLMessage::Pointer output = mitk::IGTLMessage::New();
  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
  this->SetNumberOfRequiredInputs(1);

  m_CurrentTimeStep = 0;
}

mitk::USImageToIGTLMessageFilter::~USImageToIGTLMessageFilter() {}

void mitk::USImageToIGTLMessageFilter::GenerateData()
{
  for (size_t i = 0; i < this->GetNumberOfIndexedOutputs(); ++i)
  {
    mitk::IGTLMessage* output = this->GetOutput(i);
    assert(output);
    const mitk::USImage* input = this->GetInput(i);
    assert(input);

    int dim = input->GetDimension();

    if (dim > 3)
    {
      mitkThrow() << "Too many dimensions";
    }

    igtl::ImageMessage::Pointer imgMsg = igtl::ImageMessage::New();
    // TODO: Find out, if MITK uses RAS or LPS or something else entirely and we
    // need to convert
    imgMsg->SetCoordinateSystem(igtl::ImageMessage::COORDINATE_RAS);

    mitk::USImageMetadata::Pointer md = input->GetMetadata();
    imgMsg->SetDeviceName(md->GetProbeName().c_str());

    const mitk::PixelType pt = input->GetPixelType(0);
    switch (pt.GetComponentType())
    {
      case itk::ImageIOBase::UCHAR:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_UINT8);
      case itk::ImageIOBase::CHAR:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_INT8);
      case itk::ImageIOBase::USHORT:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_UINT16);
      case itk::ImageIOBase::SHORT:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_INT16);
      case itk::ImageIOBase::UINT:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_UINT32);
      case itk::ImageIOBase::INT:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_INT32);
      case itk::ImageIOBase::FLOAT:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_FLOAT32);
      case itk::ImageIOBase::DOUBLE:
        imgMsg->SetScalarType(igtl::ImageMessage::TYPE_FLOAT64);
      default:
        mitkThrow() << "Incompatible PixelType " << pt.GetPixelTypeAsString();
    }

    imgMsg->SetNumComponents(pt.GetNumberOfComponents());

    // TODO: Can this be done simpler?
    if (htonl(47) == 47)
    {
      imgMsg->SetEndian(igtl::ImageMessage::ENDIAN_BIG);
    }
    else
    {
      imgMsg->SetEndian(igtl::ImageMessage::ENDIAN_LITTLE);
    }

    switch (dim)
    {
      case 2:
        imgMsg->SetDimensions(input->GetDimension(0), input->GetDimension(1),
                              1);
      case 3:
        imgMsg->SetDimensions((int*)input->GetDimensions());
      default:
        mitkThrow() << "Too few dimensions";
    }

    // TODO: Origin, Geometry, spacing…
    mitk::SlicedGeometry3D* geometry = input->GetSlicedGeometry();

    Vector3D x = geometry->GetAxisVector(0);
    Vector3D y = geometry->GetAxisVector(1);
    Vector3D z = geometry->GetAxisVector(2);

    float t[3] = {(float)x[0], (float)x[1], (float)x[2]};
    float s[3] = {(float)y[0], (float)y[1], (float)y[2]};
    float n[3] = {(float)z[0], (float)z[1], (float)z[2]};

    imgMsg->SetNormals(t, s, n);

    mitk::Vector3D spacing = geometry->GetSpacing();
    imgMsg->SetSpacing(spacing[0], spacing[1], spacing[2]);

    Point3D origin = geometry->GetOrigin();
    float forigin[3] = {(float)origin[0], (float)origin[1], (float)origin[2]};
    imgMsg->SetOrigin(forigin);
  }
}

void mitk::USImageToIGTLMessageFilter::SetInput(const USImage* im)
{
  this->ProcessObject::SetNthInput(0, const_cast<USImage*>(im));
  this->CreateOutputsForAllInputs();
}

void mitk::USImageToIGTLMessageFilter::SetInput(unsigned int idx, const USImage* im)
{
  this->ProcessObject::SetNthInput(idx, const_cast<USImage*>(im));
  this->CreateOutputsForAllInputs();
}

const mitk::USImage* mitk::USImageToIGTLMessageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
  {
    return NULL;
  }
  return static_cast<const USImage*>(this->ProcessObject::GetInput(0));
}

const mitk::USImage* mitk::USImageToIGTLMessageFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfInputs() < idx + 1)
  {
    return NULL;
  }
  return static_cast<const USImage*>(this->ProcessObject::GetInput(idx));
}

void mitk::USImageToIGTLMessageFilter::CreateOutputsForAllInputs()
{
  // create one message output for all image inputs
  this->SetNumberOfIndexedOutputs(this->GetNumberOfIndexedInputs());

  for (size_t idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == NULL)
    {
      this->SetNthOutput(idx, this->MakeOutput(idx));
    }
    this->Modified();
  }
}

void mitk::USImageToIGTLMessageFilter::ConnectTo(mitk::USImageSource* upstream)
{
  this->SetInput(static_cast<USImage*>(upstream->GetNextImage().GetPointer()));
}
