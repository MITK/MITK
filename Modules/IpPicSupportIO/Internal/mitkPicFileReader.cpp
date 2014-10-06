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


#include "mitkPicFileReader.h"
#include "mitkPicHelper.h"
#include "mitkLegacyAdaptors.h"

#include "mitkImageWriteAccessor.h"
#include "mitkCustomMimeType.h"

extern "C"
{
  mitkIpPicDescriptor * MITKipPicGet( char *infile_name, mitkIpPicDescriptor *pic );
  mitkIpPicDescriptor * MITKipPicGetTags( char *infile_name, mitkIpPicDescriptor *pic );
}

mitk::PicFileReader::PicFileReader()
  : AbstractFileReader()
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

std::vector<mitk::BaseData::Pointer> mitk::PicFileReader::Read()
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

  mitkIpPicDescriptor* header=mitkIpPicGetHeader(const_cast<char *>(fileName.c_str()), NULL);

  if ( !header )
  {
    mitkThrow() << "File could not be read.";
  }

  header=MITKipPicGetTags(const_cast<char *>(fileName.c_str()), header);

  int channels = 1;

  mitkIpPicTSV_t *tsv;
  if ( (tsv = mitkIpPicQueryTag( header, "SOURCE HEADER" )) != NULL)
  {
    if(tsv->n[0]>1e+06)
    {
      mitkIpPicTSV_t *tsvSH;
      tsvSH = mitkIpPicDelTag( header, "SOURCE HEADER" );
      mitkIpPicFreeTag(tsvSH);
    }
  }
  if ( (tsv = mitkIpPicQueryTag( header, "ICON80x80" )) != NULL)
  {
    mitkIpPicTSV_t *tsvSH;
    tsvSH = mitkIpPicDelTag( header, "ICON80x80" );
    mitkIpPicFreeTag(tsvSH);
  }
  if ( (tsv = mitkIpPicQueryTag( header, "VELOCITY" )) != NULL)
  {
    ++channels;
    mitkIpPicDelTag( header, "VELOCITY" );
  }

  if( header == NULL || header->bpe == 0)
  {
    mitkThrow() << " Could not read file " << fileName;
  }

  // if pic image only 2D, the n[2] value is not initialized
  unsigned int slices = 1;
  if( header->dim == 2 )
  {
    header->n[2] = slices;
  }

  // First initialize the geometry of the output image by the pic-header
  SlicedGeometry3D::Pointer slicedGeometry = mitk::SlicedGeometry3D::New();
  PicHelper::InitializeEvenlySpaced(header, header->n[2], slicedGeometry);

  // if pic image only 3D, the n[3] value is not initialized
  unsigned int timesteps = 1;
  if( header->dim > 3 )
  {
    timesteps = header->n[3];
  }

  slicedGeometry->ImageGeometryOn();
  ProportionalTimeGeometry::Pointer timeGeometry = ProportionalTimeGeometry::New();
  timeGeometry->Initialize(slicedGeometry, timesteps);

  // Cast the pic descriptor to ImageDescriptor and initialize the output

  output->Initialize( CastToImageDescriptor(header));
  output->SetTimeGeometry( timeGeometry );
  mitkIpPicFree ( header );

  return output;
}

void mitk::PicFileReader::ConvertHandedness(mitkIpPicDescriptor* pic)
{
  //left to right handed conversion
  if(pic->dim>=3)
  {
    mitkIpPicDescriptor* slice=mitkIpPicCopyHeader(pic, NULL);
    slice->dim=2;
    size_t size=_mitkIpPicSize(slice);
    slice->data=malloc(size);

    size_t v, volumes = (pic->dim>3? pic->n[3] : 1);
    size_t volume_size = size*pic->n[2];

    for(v=0; v<volumes; ++v)
    {
      unsigned char *p_first=(unsigned char *)pic->data;
      unsigned char *p_last=(unsigned char *)pic->data;
      p_first+=v*volume_size;
      p_last+=size*(pic->n[2]-1)+v*volume_size;

      size_t i, smid=pic->n[2]/2;
      for(i=0; i<smid;++i,p_last-=size, p_first+=size)
      {
        memcpy(slice->data, p_last, size);
        memcpy(p_last, p_first, size);
        memcpy(p_first, slice->data, size);
      }
    }
    mitkIpPicFree(slice);
  }
}

mitk::PicFileReader* mitk::PicFileReader::Clone() const
{
  return new PicFileReader(*this);
}

void mitk::PicFileReader::FillImage(Image::Pointer output)
{
  mitkIpPicDescriptor* outputPic = mitkIpPicNew();
  mitk::ImageWriteAccessor imageAccess(output);
  outputPic = CastToIpPicDescriptor(output, &imageAccess, outputPic);
  mitkIpPicDescriptor* pic=MITKipPicGet(const_cast<char *>(this->GetLocalFileName().c_str()),
                                        outputPic);
  // comes upside-down (in MITK coordinates) from PIC file
  ConvertHandedness(pic);

  mitkIpPicTSV_t *tsv;
  if ( (tsv = mitkIpPicQueryTag( pic, "SOURCE HEADER" )) != NULL)
  {
    if(tsv->n[0]>1e+06)
    {
      mitkIpPicTSV_t *tsvSH;
      tsvSH = mitkIpPicDelTag( pic, "SOURCE HEADER" );
      mitkIpPicFreeTag(tsvSH);
    }
  }
  if ( (tsv = mitkIpPicQueryTag( pic, "ICON80x80" )) != NULL)
  {
    mitkIpPicTSV_t *tsvSH;
    tsvSH = mitkIpPicDelTag( pic, "ICON80x80" );
    mitkIpPicFreeTag(tsvSH);
  }
  if ( (tsv = mitkIpPicQueryTag( pic, "VELOCITY" )) != NULL)
  {
    mitkIpPicDescriptor* header = mitkIpPicCopyHeader(pic, NULL);
    header->data = tsv->value;
    ConvertHandedness(header);
    output->SetChannel(header->data, 1);
    header->data = NULL;
    mitkIpPicFree(header);
    mitkIpPicDelTag( pic, "VELOCITY" );
  }
}
