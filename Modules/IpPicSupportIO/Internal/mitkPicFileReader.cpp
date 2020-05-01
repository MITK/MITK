/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPicFileReader.h"
#include "mitkPicHelper.h"

#include "mitkCustomMimeType.h"
#include "mitkImageWriteAccessor.h"

#include <mitkIpPic.h>


static mitk::PixelType CastToPixelType(mitkIpPicType_t pictype, size_t bpe)
{
  const bool isSignedIntegralType = (pictype == mitkIpPicInt);
  const bool isUnsignedIntegralType = (pictype == mitkIpPicUInt);

  if (isSignedIntegralType)
  {
    switch (bpe)
    {
    case sizeof(char) :
      return mitk::MakeScalarPixelType<char>();
    case sizeof(short) :
      return mitk::MakeScalarPixelType<short>();
    default:
      return mitk::MakeScalarPixelType<int>();
    }
  }
  else if (isUnsignedIntegralType)
  {
    switch (bpe)
    {
    case sizeof(unsigned char) :
      return mitk::MakeScalarPixelType<unsigned char>();
    case sizeof(unsigned short) :
      return mitk::MakeScalarPixelType<unsigned short>();
    default:
      return mitk::MakeScalarPixelType<unsigned int>();
    }
  }
  else // is floating point type
  {
    switch (bpe)
    {
    case sizeof(float) :
      return mitk::MakeScalarPixelType<float>();
    default:
      return mitk::MakeScalarPixelType<double>();
    }
  }
}

static mitk::ImageDescriptor::Pointer CastToImageDescriptor(mitkIpPicDescriptor *desc)
{
  mitk::ImageDescriptor::Pointer imDescriptor = mitk::ImageDescriptor::New();

  imDescriptor->Initialize(desc->n, desc->dim);

  mitk::PixelType ptype = ::CastToPixelType(desc->type, (desc->bpe / 8));
  imDescriptor->AddNewChannel(ptype, "imported by pic");

  return imDescriptor;
}

static mitkIpPicType_t CastToIpPicType(int intype)
{
  const bool isSignedIntegralType = (intype == itk::ImageIOBase::INT || intype == itk::ImageIOBase::SHORT ||
    intype == itk::ImageIOBase::CHAR || intype == itk::ImageIOBase::LONG);

  const bool isUnsignedIntegralType = (intype == itk::ImageIOBase::UINT || intype == itk::ImageIOBase::USHORT ||
    intype == itk::ImageIOBase::UCHAR || intype == itk::ImageIOBase::ULONG);

  const bool isFloatingPointType = (intype == itk::ImageIOBase::FLOAT || intype == itk::ImageIOBase::DOUBLE);

  if (isSignedIntegralType)
    return mitkIpPicInt;
  if (isUnsignedIntegralType)
    return mitkIpPicUInt;
  if (isFloatingPointType)
    return mitkIpPicFloat;
  return mitkIpPicUnknown;
}

static mitkIpPicDescriptor * CastToIpPicDescriptor(mitk::Image::Pointer refImg,
  mitk::ImageWriteAccessor *imageAccess,
  mitkIpPicDescriptor *picDesc)
{
  const mitk::ImageDescriptor::Pointer imDesc = refImg->GetImageDescriptor();

  // initialize dimension information
  for (unsigned int i = 0; i < 8; i++)
  {
    picDesc->n[i] = 1;
  }

  // set dimension information
  picDesc->dim = refImg->GetDimension();
  memcpy(picDesc->n, imDesc->GetDimensions(), picDesc->dim * sizeof(unsigned int));

  picDesc->type = ::CastToIpPicType(refImg->GetPixelType().GetComponentType());
  picDesc->bpe = refImg->GetPixelType().GetBpe();
  if (imageAccess != nullptr)
  {
    picDesc->data = imageAccess->GetData();
  }

  return picDesc;
}


mitk::PicFileReader::PicFileReader() : AbstractFileReader()
{
  CustomMimeType mimeType(this->GetMimeTypePrefix() + "mbipic");
  mimeType.AddExtension("pic");
  mimeType.AddExtension("pic.gz");
  mimeType.AddExtension("PIC");
  mimeType.AddExtension("PIC.gz");
  mimeType.SetCategory("Images");
  mimeType.SetComment("DKFZ Legacy PIC Format");

  this->SetMimeType(mimeType);
  this->SetDescription("DKFZ PIC");

  this->RegisterService();
}

std::vector<mitk::BaseData::Pointer> mitk::PicFileReader::DoRead()
{
  mitk::Image::Pointer image = this->CreateImage();
  this->FillImage(image);
  std::vector<BaseData::Pointer> result;
  result.push_back(image.GetPointer());
  return result;
}

mitk::Image::Pointer mitk::PicFileReader::CreateImage()
{
  Image::Pointer output = Image::New();

  std::string fileName = this->GetLocalFileName();

  mitkIpPicDescriptor *header = mitkIpPicGetHeader(fileName.c_str(), nullptr);

  if (!header)
  {
    mitkThrow() << "File could not be read.";
  }

  header = mitkIpPicGetTags(fileName.c_str(), header);

  int channels = 1;

  mitkIpPicTSV_t *tsv;
  if ((tsv = mitkIpPicQueryTag(header, "SOURCE HEADER")) != nullptr)
  {
    if (tsv->n[0] > 1e+06)
    {
      mitkIpPicTSV_t *tsvSH;
      tsvSH = mitkIpPicDelTag(header, "SOURCE HEADER");
      mitkIpPicFreeTag(tsvSH);
    }
  }
  if ((tsv = mitkIpPicQueryTag(header, "ICON80x80")) != nullptr)
  {
    mitkIpPicTSV_t *tsvSH;
    tsvSH = mitkIpPicDelTag(header, "ICON80x80");
    mitkIpPicFreeTag(tsvSH);
  }
  if ((tsv = mitkIpPicQueryTag(header, "VELOCITY")) != nullptr)
  {
    ++channels;
    mitkIpPicDelTag(header, "VELOCITY");
  }

  if (header == nullptr || header->bpe == 0)
  {
    mitkThrow() << " Could not read file " << fileName;
  }

  // if pic image only 2D, the n[2] value is not initialized
  unsigned int slices = 1;
  if (header->dim == 2)
  {
    header->n[2] = slices;
  }

  // First initialize the geometry of the output image by the pic-header
  SlicedGeometry3D::Pointer slicedGeometry = mitk::SlicedGeometry3D::New();
  PicHelper::InitializeEvenlySpaced(header, header->n[2], slicedGeometry);

  // if pic image only 3D, the n[3] value is not initialized
  unsigned int timesteps = 1;
  if (header->dim > 3)
  {
    timesteps = header->n[3];
  }

  slicedGeometry->ImageGeometryOn();
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, timesteps);

  // Cast the pic descriptor to ImageDescriptor and initialize the output

  output->Initialize(CastToImageDescriptor(header));
  output->SetTimeGeometry(timeGeometry);
  mitkIpPicFree(header);

  return output;
}

void mitk::PicFileReader::ConvertHandedness(mitkIpPicDescriptor *pic)
{
  // left to right handed conversion
  if (pic->dim >= 3)
  {
    mitkIpPicDescriptor *slice = mitkIpPicCopyHeader(pic, nullptr);
    slice->dim = 2;
    size_t size = _mitkIpPicSize(slice);
    slice->data = malloc(size);

    size_t v, volumes = (pic->dim > 3 ? pic->n[3] : 1);
    size_t volume_size = size * pic->n[2];

    for (v = 0; v < volumes; ++v)
    {
      auto *p_first = (unsigned char *)pic->data;
      auto *p_last = (unsigned char *)pic->data;
      p_first += v * volume_size;
      p_last += size * (pic->n[2] - 1) + v * volume_size;

      size_t i, smid = pic->n[2] / 2;
      for (i = 0; i < smid; ++i, p_last -= size, p_first += size)
      {
        memcpy(slice->data, p_last, size);
        memcpy(p_last, p_first, size);
        memcpy(p_first, slice->data, size);
      }
    }
    mitkIpPicFree(slice);
  }
}

mitk::PicFileReader *mitk::PicFileReader::Clone() const
{
  return new PicFileReader(*this);
}

void mitk::PicFileReader::FillImage(Image::Pointer output)
{
  mitkIpPicDescriptor *outputPic = mitkIpPicNew();
  outputPic = CastToIpPicDescriptor(output, nullptr, outputPic);
  mitkIpPicDescriptor *pic = mitkIpPicGet(this->GetLocalFileName().c_str(), outputPic);
  // comes upside-down (in MITK coordinates) from PIC file
  ConvertHandedness(pic);

  mitkIpPicTSV_t *tsv;
  if ((tsv = mitkIpPicQueryTag(pic, "SOURCE HEADER")) != nullptr)
  {
    if (tsv->n[0] > 1e+06)
    {
      mitkIpPicTSV_t *tsvSH;
      tsvSH = mitkIpPicDelTag(pic, "SOURCE HEADER");
      mitkIpPicFreeTag(tsvSH);
    }
  }
  if ((tsv = mitkIpPicQueryTag(pic, "ICON80x80")) != nullptr)
  {
    mitkIpPicTSV_t *tsvSH;
    tsvSH = mitkIpPicDelTag(pic, "ICON80x80");
    mitkIpPicFreeTag(tsvSH);
  }
  if ((tsv = mitkIpPicQueryTag(pic, "VELOCITY")) != nullptr)
  {
    mitkIpPicDescriptor *header = mitkIpPicCopyHeader(pic, nullptr);
    header->data = tsv->value;
    ConvertHandedness(header);
    output->SetChannel(header->data, 1);
    header->data = nullptr;
    mitkIpPicFree(header);
    mitkIpPicDelTag(pic, "VELOCITY");
  }

  // Copy the memory to avoid mismatches of malloc() and delete[].
  // mitkIpPicGet will always allocate a new memory block with malloc(),
  // but MITK Images delete the data via delete[].
  output->SetImportChannel(pic->data, 0, Image::CopyMemory);
  pic->data = nullptr;
  mitkIpPicFree(pic);
}
