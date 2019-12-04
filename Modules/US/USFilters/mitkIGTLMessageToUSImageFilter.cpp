/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIGTLMessageToUSImageFilter.h>
#include <igtlImageMessage.h>
#include <itkByteSwapper.h>

#include <vtkSmartPointer.h>

void mitk::IGTLMessageToUSImageFilter::GetNextRawImage(
  std::vector<mitk::Image::Pointer>& imgVector)
{
  if (imgVector.size() != 1)
    imgVector.resize(1);

  mitk::Image::Pointer& img = imgVector[0];
  m_upstream->Update();

  mitk::IGTLMessage* msg = m_upstream->GetOutput();

  if (msg != nullptr && (!msg->IsDataValid() || std::strcmp(msg->GetIGTLMessageType(), "IMAGE") != 0))
  {
    img = m_previousImage;
    return;
  }

  igtl::MessageBase::Pointer msgBase = msg->GetMessage();
  igtl::ImageMessage* imgMsg = (igtl::ImageMessage*)(msgBase.GetPointer());

  bool big_endian = (imgMsg->GetEndian() == igtl::ImageMessage::ENDIAN_BIG);

  if (imgMsg->GetCoordinateSystem() != igtl::ImageMessage::COORDINATE_RAS)
  {
    // TODO: Which coordinate system does MITK use?
    mitkThrow() << "Can not handle messages with LPS coordinate system";
  }

  switch (imgMsg->GetScalarType())
  {
  case igtl::ImageMessage::TYPE_UINT8:
    Initiate<unsigned char>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_INT8:
    Initiate<char>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_UINT16:
    Initiate<unsigned short>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_INT16:
    Initiate<short>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_UINT32:
    Initiate<unsigned int>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_INT32:
    Initiate<int>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_FLOAT32:
    Initiate<float>(img, imgMsg, big_endian);
    break;
  case igtl::ImageMessage::TYPE_FLOAT64:
    Initiate<double>(img, imgMsg, big_endian);
    break;
  default:
    mitkThrow() << "Incompatible PixelType " << imgMsg;
  }
}

template <typename TPixel>
void mitk::IGTLMessageToUSImageFilter::Initiate(mitk::Image::Pointer& img,
  igtl::ImageMessage* msg,
  bool big_endian)
{
  typedef itk::Image<TPixel, 3> ImageType;

  typename ImageType::Pointer output = ImageType::New();
  typename ImageType::RegionType region;
  typename ImageType::RegionType::SizeType size;
  typename ImageType::RegionType::IndexType index;
  typename ImageType::SpacingType spacing;
  typename ImageType::PointType origin;

  // Copy dimensions
  int dims[3];
  msg->GetDimensions(dims);
  size_t num_pixel = 1;
  for (size_t i = 0; i < 3; i++)
  {
    size[i] = dims[i];
    num_pixel *= dims[i];
  }

  // Handle subvolume information. We want the subvolume to be the whole image
  // for now.
  int sdims[3], offs[3];
  msg->GetSubVolume(sdims, offs);
  for (size_t i = 0; i < 3; i++)
  {
    if (offs[i] != 0 || sdims[i] != dims[i])
    {
      // TODO: Handle messages with smaller subvolume than whole image
      throw("Can not handle message with smaller subvolume than whole image");
    }
  }

  index.Fill(0);

  float matF[4][4];
  msg->GetMatrix(matF);
  vtkSmartPointer<vtkMatrix4x4> vtkMatrix = vtkSmartPointer<vtkMatrix4x4>::New();

  for (size_t i = 0; i < 4; ++i)
    for (size_t j = 0; j < 4; ++j)
      vtkMatrix->SetElement(i, j, matF[i][j]);

  float spacingMsg[3];

  msg->GetSpacing(spacingMsg);

  for (int i = 0; i < 3; ++i)
    spacing[i] = spacingMsg[i];

  region.SetSize(size);
  region.SetIndex(index);
  output->SetRegions(region);
  output->SetSpacing(spacing);
  output->Allocate();

  TPixel* in = (TPixel*)msg->GetScalarPointer();
  TPixel* out = (TPixel*)output->GetBufferPointer();
  memcpy(out, in, num_pixel * sizeof(TPixel));
  if (big_endian)
  {
    // Even though this method is called "FromSystemToBigEndian", it also swaps
    // "FromBigEndianToSystem".
    // This makes sense, but might be confusing at first glance.
    itk::ByteSwapper<TPixel>::SwapRangeFromSystemToBigEndian(out, num_pixel);
  }
  else
  {
    itk::ByteSwapper<TPixel>::SwapRangeFromSystemToLittleEndian(out, num_pixel);
  }

  img = mitk::Image::New();
  img->InitializeByItk(output.GetPointer());
  img->SetVolume(output->GetBufferPointer());
  //img->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(vtkMatrix);
  m_previousImage = img;

  float iorigin[3];
  msg->GetOrigin(iorigin);
  for (size_t i = 0; i < 3; i++)
  {
    origin[i] = iorigin[i];
  }
  output->SetOrigin(origin);
}

mitk::IGTLMessageToUSImageFilter::IGTLMessageToUSImageFilter()
  : m_upstream(nullptr)
{
  MITK_DEBUG << "Instantiated this (" << this << ") mitkIGTMessageToUSImageFilter\n";
}

void mitk::IGTLMessageToUSImageFilter::SetNumberOfExpectedOutputs(
  unsigned int numOutputs)
{
  if (numOutputs > 1)
  {
    throw("Can only have 1 output for IGTLMessageToUSImageFilter.");
  }
}

void mitk::IGTLMessageToUSImageFilter::ConnectTo(
  mitk::IGTLMessageSource* UpstreamFilter)
{
  MITK_DEBUG << "Connected this (" << this << ") mitkIGTLMessageToUSImageFilter to MessageSource (" << UpstreamFilter << ")\n";
  m_upstream = UpstreamFilter;
}
