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
#include "itkByteSwapper.h"
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
  // MITK_INFO << "ImageToIGTLMessageFilter.GenerateData()";
  for (unsigned int i = 0; i < this->GetNumberOfIndexedOutputs(); ++i)
  {
    mitk::IGTLMessage* output = this->GetOutput(i);
    assert(output);

    const mitk::Image* img = this->GetInput(i);

    int dims = img->GetDimension();
    int chn = img->GetNumberOfChannels();

    MITK_INFO << "Sending image. Dimensions: " << dims << " Channels: " << chn << "\n";

    if (dims < 1)
    {
      MITK_ERROR << "Can not handle dimensionless images";
    }
    if (dims > 3)
    {
      MITK_ERROR << "Can not handle more than three dimensions";
      continue;
    }

    if (chn != 1)
    {
      MITK_ERROR << "Can not handle anything but one channel. Image contained " << chn;
      continue;
    }

    igtl::ImageMessage::Pointer imgMsg = igtl::ImageMessage::New();

    // TODO: Which kind of coordinate system does MITK really use?
    imgMsg->SetCoordinateSystem(igtl::ImageMessage::COORDINATE_RAS);

    // We could do this based on the host endiannes, but that's weird.
    // We instead use little endian, as most modern systems are little endian,
    // so there will probably not be an endian swap involved.
    imgMsg->SetEndian(igtl::ImageMessage::ENDIAN_LITTLE);

    // Set number of components.
    mitk::PixelType type = img->GetPixelType();
    imgMsg->SetNumComponents(type.GetNumberOfComponents());

    // Set scalar type.
    switch (type.GetComponentType())
    {
    case itk::ImageIOBase::CHAR:
      imgMsg->SetScalarTypeToInt8();
      break;
    case itk::ImageIOBase::UCHAR:
      imgMsg->SetScalarTypeToUint8();
      break;
    case itk::ImageIOBase::SHORT:
      imgMsg->SetScalarTypeToInt16();
      break;
    case itk::ImageIOBase::USHORT:
      imgMsg->SetScalarTypeToUint16();
      break;
    case itk::ImageIOBase::INT:
      imgMsg->SetScalarTypeToInt32();
      break;
    case itk::ImageIOBase::UINT:
      imgMsg->SetScalarTypeToUint32();
      break;
    case itk::ImageIOBase::LONG:
      // OIGTL doesn't formally support 64bit int scalars, but if they are
      // ever added,
      // they will have the identifier 8 assigned.
      imgMsg->SetScalarType(8);
      break;
    case itk::ImageIOBase::ULONG:
      // OIGTL doesn't formally support 64bit uint scalars, but if they are
      // ever added,
      // they will have the identifier 9 assigned.
      imgMsg->SetScalarType(9);
      break;
    case itk::ImageIOBase::FLOAT:
      // The igtl library has no method for this. Correct type is 10.
      imgMsg->SetScalarType(10);
      break;
    case itk::ImageIOBase::DOUBLE:
      // The igtl library has no method for this. Correct type is 11.
      imgMsg->SetScalarType(11);
      break;
    default:
      MITK_ERROR << "Can not handle pixel component type "
        << type.GetComponentType();
      return;
    }

    // Set transformation matrix.
    vtkMatrix4x4* matrix = img->GetGeometry()->GetVtkMatrix();

    float matF[4][4];
    for (size_t i = 0; i < 4; ++i)
    {
      for (size_t j = 0; j < 4; ++j)
      {
        matF[i][j] = matrix->GetElement(i, j);
      }
    }
    imgMsg->SetMatrix(matF);

    float spacing[3];
    auto spacingImg = img->GetGeometry()->GetSpacing();

    for (int i = 0; i < 3; ++i)
      spacing[i] = spacingImg[i];

    imgMsg->SetSpacing(spacing);

    // Set dimensions.
    int sizes[3];
    for (size_t j = 0; j < 3; ++j)
    {
      sizes[j] = img->GetDimension(j);
    }
    imgMsg->SetDimensions(sizes);

    // Allocate and copy data.
    imgMsg->AllocatePack();
    imgMsg->AllocateScalars();

    size_t num_pixel = sizes[0] * sizes[1] * sizes[2];
    void* out = imgMsg->GetScalarPointer();
    {
      // Scoped, so that readAccess will be released ASAP.
      void* in = img->GetVolumeData()->GetData();

      memcpy(out, in, num_pixel * type.GetSize());
    }

    // We want to byte swap to little endian. We would like to just
    // swap by number of bytes for each component, but itk::ByteSwapper
    // is templated over element type, not over element size. So we need to
    // switch on the size and use types of the same size.
    size_t num_scalars = num_pixel * type.GetNumberOfComponents();
    switch (type.GetComponentType())
    {
    case itk::ImageIOBase::CHAR:
    case itk::ImageIOBase::UCHAR:
      // No endian conversion necessary, because a char is exactly one byte!
      break;
    case itk::ImageIOBase::SHORT:
    case itk::ImageIOBase::USHORT:
      itk::ByteSwapper<short>::SwapRangeFromSystemToLittleEndian((short*)out,
        num_scalars);
      break;
    case itk::ImageIOBase::INT:
    case itk::ImageIOBase::UINT:
      itk::ByteSwapper<int>::SwapRangeFromSystemToLittleEndian((int*)out,
        num_scalars);
      break;
    case itk::ImageIOBase::LONG:
    case itk::ImageIOBase::ULONG:
      itk::ByteSwapper<long>::SwapRangeFromSystemToLittleEndian((long*)out,
        num_scalars);
      break;
    case itk::ImageIOBase::FLOAT:
      itk::ByteSwapper<float>::SwapRangeFromSystemToLittleEndian((float*)out,
        num_scalars);
      break;
    case itk::ImageIOBase::DOUBLE:
      itk::ByteSwapper<double>::SwapRangeFromSystemToLittleEndian(
        (double*)out, num_scalars);
      break;
    }

    imgMsg->Pack();

    output->SetMessage(imgMsg.GetPointer());
  }
}

void mitk::ImageToIGTLMessageFilter::SetInput(const mitk::Image* img)
{
  this->ProcessObject::SetNthInput(0, const_cast<mitk::Image*>(img));
  this->CreateOutputsForAllInputs();
}

void mitk::ImageToIGTLMessageFilter::SetInput(unsigned int idx,
  const Image* img)
{
  this->ProcessObject::SetNthInput(idx, const_cast<mitk::Image*>(img));
  this->CreateOutputsForAllInputs();
}

const mitk::Image* mitk::ImageToIGTLMessageFilter::GetInput(void)
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::Image*>(this->ProcessObject::GetInput(0));
}

const mitk::Image* mitk::ImageToIGTLMessageFilter::GetInput(unsigned int idx)
{
  if (this->GetNumberOfInputs() < idx + 1)
  {
    return NULL;
  }
  return static_cast<const mitk::Image*>(this->ProcessObject::GetInput(idx));
}

void mitk::ImageToIGTLMessageFilter::ConnectTo(mitk::ImageSource* upstream)
{
  MITK_INFO << "Image source for this (" << this << ") mitkImageToIGTLMessageFilter is " << upstream;
  for (DataObjectPointerArraySizeType i = 0; i < upstream->GetNumberOfOutputs();
    i++)
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
}
