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

#include <mitkIGTLMessageToUSImageFilter.h>
#include <igtlImageMessage.h>
#include <itkByteSwapper.h>

void mitk::IGTLMessageToUSImageFilter::GetNextRawImage(
    mitk::Image::Pointer& img)
{
  m_upstream->Update();

  mitk::IGTLMessage* msg = m_upstream->GetOutput();
  if (!msg->IsDataValid()) {
      return;
  }

  igtl::MessageBase::Pointer msgBase = msg->GetMessage();
  igtl::ImageMessage* imgMsg = (igtl::ImageMessage*)(msgBase.GetPointer());

  MITK_INFO << "<ImageMessage>";
  imgMsg->Print(std::cout);
  MITK_INFO << "</ImageMessage>";

  if (imgMsg->GetNumComponents() != 1)
  {
    // TODO: Handle non-grayscale images
    throw("Can not handle non-grayscale images");
  }

  bool big_endian = (imgMsg->GetEndian() == igtl::ImageMessage::ENDIAN_BIG);

  if (imgMsg->GetCoordinateSystem() != igtl::ImageMessage::COORDINATE_RAS)
  {
    // TODO: Which coordinate system does MITK use?
    throw("Can not handle messages with LAS coordinate system");
  }

  switch (imgMsg->GetScalarType())
  {
    case igtl::ImageMessage::TYPE_UINT8:
      Initiate<unsigned char>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_INT8:
      Initiate<char>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_UINT16:
      Initiate<unsigned short>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_INT16:
      Initiate<short>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_UINT32:
      Initiate<unsigned int>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_INT32:
      Initiate<int>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_FLOAT32:
      Initiate<float>(img, imgMsg, big_endian);
    case igtl::ImageMessage::TYPE_FLOAT64:
      Initiate<double>(img, imgMsg, big_endian);
    default:
      mitkThrow() << "Incompatible PixelType " << imgMsg;
  }
}

template <typename TPixel>
void mitk::IGTLMessageToUSImageFilter::Initiate(mitk::Image::Pointer& img,
                                                igtl::ImageMessage* msg, bool big_endian)
{
  typedef itk::Image<TPixel, 3> ImageType;

  typename ImageType::Pointer output = ImageType::New();
  typename ImageType::RegionType region;
  typename ImageType::RegionType::SizeType size;
  typename ImageType::RegionType::IndexType index;
  typename ImageType::SpacingType spacing;
  typename ImageType::PointType origin;
  typename ImageType::DirectionType direction;

  int dims[3];
  msg->GetDimensions(dims);
  size_t num_pixel = 1;
  for (size_t i = 0; i < 3; i++)
  {
    size[i] = dims[i];
    num_pixel *= dims[i];
  }

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

  float spc[3];
  msg->GetSpacing(spc);
  for (size_t i = 0; i < 3; i++)
  {
    spacing[i] = spc[i];
  }

  spacing.Fill(1);

  region.SetSize(size);
  region.SetIndex(index);
  output->SetRegions(region);
  output->SetSpacing(spacing);
  output->Allocate();

  TPixel* in = (TPixel*)msg->GetScalarPointer();
  TPixel* out = (TPixel*)output->GetBufferPointer();
  memcpy(out, in, num_pixel * sizeof(TPixel));
  if (big_endian) {
    // Even though this method is called "FromSystemToBigEndian", it also swaps "FromBigEndianToSystem".
    // This makes sense, but might be confusing at first glance.
    itk::ByteSwapper<TPixel>::SwapRangeFromSystemToBigEndian(out, num_pixel);
  } else {
    itk::ByteSwapper<TPixel>::SwapRangeFromSystemToLittleEndian(out, num_pixel);
  }

  // TODO: Coordinate system

  img->InitializeByItk(output.GetPointer());
  img->SetVolume(output->GetBufferPointer());

  float iorigin[3];
  msg->GetOrigin(iorigin);
  for (size_t i = 0; i < 3; i++)
  {
    origin[i] = iorigin[i];
  }
  output->SetOrigin(origin);

  output->SetDirection(direction);
}

mitk::IGTLMessageToUSImageFilter::IGTLMessageToUSImageFilter()
  : m_upstream(nullptr)
{
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
  m_upstream = UpstreamFilter;
}
