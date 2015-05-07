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

#include <mitkIGTLMessageToUSImageFilter.h>
#include <igtlImageMessage.h>

void mitk::IGTLMessageToUSImageFilter::GetNextRawImage(
    mitk::Image::Pointer& img)
{
  m_upstream->Update();

  IGTLMessage* msg = m_upstream->GetOutput();
  MITK_INFO << "<IGTLMessage>";
  msg->Print(std::cout, 2);
  MITK_INFO << "</IGTLMessage>";

  igtl::ImageMessage* imgMsg = (igtl::ImageMessage*)(m_upstream->GetOutput());

  if (!imgMsg)
  {
    throw(
        "Cast from igtl::MessageBase to igtl::ImageMessage failed! Please "
        "check the message.");
  }

  MITK_INFO << "<ImageMessage>";
  imgMsg->Print(std::cout);
  MITK_INFO << "</ImageMessage>";

  if (imgMsg->GetNumComponents() != 1)
  {
    // TODO: Handle non-grayscale images
    throw("Can not handle non-grayscale images");
  }

  int host_endian;
  // TODO: Can this be done simpler?
  if (htonl(47) == 47)
  {
    host_endian = igtl::ImageMessage::ENDIAN_BIG;
  }
  else
  {
    host_endian = igtl::ImageMessage::ENDIAN_LITTLE;
  }
  if (host_endian != imgMsg->GetEndian())
  {
    // TODO: Convert endiannes
    throw("Can not handle messages in non-host endiannes");
  }

  if (imgMsg->GetCoordinateSystem() != igtl::ImageMessage::COORDINATE_RAS)
  {
    // TODO: Which coordinate system does MITK use?
    throw("Can not handle messages with LAS coordinate system");
  }

  switch (imgMsg->GetScalarType())
  {
    case igtl::ImageMessage::TYPE_UINT8:
      Initiate<unsigned char>(img, imgMsg);
    case igtl::ImageMessage::TYPE_INT8:
      Initiate<char>(img, imgMsg);
    case igtl::ImageMessage::TYPE_UINT16:
      Initiate<unsigned short>(img, imgMsg);
    case igtl::ImageMessage::TYPE_INT16:
      Initiate<short>(img, imgMsg);
    case igtl::ImageMessage::TYPE_UINT32:
      Initiate<unsigned int>(img, imgMsg);
    case igtl::ImageMessage::TYPE_INT32:
      Initiate<int>(img, imgMsg);
    case igtl::ImageMessage::TYPE_FLOAT32:
      Initiate<float>(img, imgMsg);
    case igtl::ImageMessage::TYPE_FLOAT64:
      Initiate<double>(img, imgMsg);
    default:
      mitkThrow() << "Incompatible PixelType " << imgMsg;
  }
}

template <typename TPixel>
void mitk::IGTLMessageToUSImageFilter::Initiate(mitk::Image::Pointer& img,
                                                igtl::ImageMessage* msg)
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
  size_t num_bytes = sizeof(TPixel);
  for (size_t i = 0; i < 3; i++)
  {
    size[i] = dims[i];
    num_bytes *= size[i];
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
  memcpy((void*)output->GetBufferPointer(), msg->GetScalarPointer(), num_bytes);

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
