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

#include "mitkImageToIGTLMessageFilter.h"
#include "igtlImageMessage.h"

mitk::ImageToIGTLMessageFilter::ImageToIGTLMessageFilter()
{
  mitk::IGTLMessage::Pointer output = mitk::IGTLMessage::New();
  this->SetNumberOfRequiredOutputs(1);
  this->SetNthOutput(0, output.GetPointer());
  this->SetNumberOfRequiredInputs(1);
}

void mitk::ImageToIGTLMessageFilter::GenerateData()
{
  MITK_INFO << "GenerateData()";



  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs() ; ++i)
  {
    mitk::IGTLMessage* output = this->GetOutput(i);
    assert(output);

    igtl::ImageMessage::Pointer imgMsg = igtl::ImageMessage::New();

    // TODO: Actually use the data of this->GetInput(i) (itk::Image*)

    imgMsg->SetDimensions(256, 256, 1);
    imgMsg->SetCoordinateSystem(igtl::ImageMessage::COORDINATE_RAS);
    imgMsg->SetEndian(igtl::ImageMessage::ENDIAN_LITTLE);
    igtl::Matrix4x4 atm;
    memset(atm, '\0', sizeof(atm));
    atm[0][0] = 1;
    atm[1][1] = -1;
    atm[2][2] = 1;
    atm[3][3] = 1;
    imgMsg->SetMatrix(atm);
    imgMsg->SetNumComponents(1);
    imgMsg->SetScalarTypeToUint8();

    imgMsg->AllocatePack();
    imgMsg->AllocateScalars();

    unsigned char* buf = (unsigned char*) imgMsg->GetScalarPointer();
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 256; ++j) {
            buf[256*i + j] = (i + j) % 256;
        }
    }

    imgMsg->Pack();

    output->SetMessage(imgMsg.GetPointer());
  }

  return;

  for (size_t i = 0; i < this->GetNumberOfIndexedOutputs(); ++i)
  {
    mitk::IGTLMessage* output = this->GetOutput(i);
    assert(output);
    const mitk::Image* input = this->GetInput(i);
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

//    mitk::ImageMetadata::Pointer md = input->GetMetadata();
//    imgMsg->SetDeviceName(md->GetProbeName().c_str());

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

    // TODO: Endian

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

void mitk::ImageToIGTLMessageFilter::SetInput(const mitk::Image* img)
{
  this->ProcessObject::SetNthInput(0, const_cast<Image*>(img));
  this->CreateOutputsForAllInputs();
}

void mitk::ImageToIGTLMessageFilter::SetInput(unsigned int idx,
                                                      const Image* img)
{
  this->ProcessObject::SetNthInput(idx, const_cast<Image*>(img));
  this->CreateOutputsForAllInputs();
}

const mitk::Image* mitk::ImageToIGTLMessageFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfInputs() < idx + 1)
  {
    return NULL;
  }
  return static_cast<const Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ImageToIGTLMessageFilter::ConnectTo(mitk::ImageSource* upstream)
{
    for (DataObjectPointerArraySizeType i = 0; i < upstream->GetNumberOfOutputs(); i++)
    {
      this->SetInput(i, upstream->GetOutput(i));
    }
}

void mitk::ImageToIGTLMessageFilter::CreateOutputsForAllInputs()
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


  for (unsigned int idx = 0; idx < this->GetNumberOfIndexedOutputs(); ++idx)
  {
    if (this->GetOutput(idx) == NULL)
    {
      DataObjectPointer newOutput = this->MakeOutput(idx);
      this->SetNthOutput(idx, newOutput);
    }
    this->Modified();
  }

}
