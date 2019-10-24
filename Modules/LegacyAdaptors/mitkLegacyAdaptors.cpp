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
#include "mitkLegacyAdaptors.h"
#include <mitkImageRegionAccessor.h>

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(mitk::Image::Pointer refImg, mitk::ImageRegionAccessor* imageAccess, mitkIpPicDescriptor* picDesc)
{
  const mitk::ImageDescriptor::Pointer imDesc = refImg->GetImageDescriptor();

  // initialize dimension information
  for (unsigned int i=0; i<8; i++)
  {
      picDesc->n[i] = 1;
  }

  // set dimension information
  picDesc->dim = refImg->GetDimension();
  memcpy( picDesc->n, imDesc->GetDimensions(), picDesc->dim * sizeof(unsigned int) );

  picDesc->type = CastToIpPicType( refImg->GetPixelType().GetComponentType() );
  picDesc->bpe = refImg->GetPixelType().GetBpe();
  if(imageAccess != nullptr)
  {
    picDesc->data = imageAccess->getData();
  }

  return picDesc;
}

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem> refItem, mitk::ImageRegionAccessor* imageAccess, mitkIpPicDescriptor *picDesc)
{
  // initialize dimension information
  for (unsigned int i=0; i<8; i++)
  {
      picDesc->n[i] = 1;
  }

  // get the dimensionality information from image item
  picDesc->dim = refItem->GetDimension();
  for( unsigned int i=0; i<picDesc->dim; i++)
  {
    picDesc->n[i] = refItem->GetDimension(i);
  }

  picDesc->type = CastToIpPicType( refItem->GetPixelType().GetComponentType() );
  picDesc->bpe = refItem->GetPixelType().GetBpe();
  if(imageAccess != nullptr)
  {
    picDesc->data = imageAccess->getData();
  }

  return picDesc;

}

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(mitk::Image::Pointer refImg, mitkIpPicDescriptor* picDesc)
{
  const mitk::ImageDescriptor::Pointer imDesc = refImg->GetImageDescriptor();

  // initialize dimension information
  for (unsigned int i=0; i<8; i++)
  {
      picDesc->n[i] = 1;
  }

  // set dimension information
  picDesc->dim = refImg->GetDimension();
  memcpy( picDesc->n, imDesc->GetDimensions(), picDesc->dim * sizeof(unsigned int) );

  picDesc->type = CastToIpPicType( refImg->GetPixelType().GetComponentType() );
  picDesc->bpe = refImg->GetPixelType().GetBpe();
  mitk::ImageRegionAccessor imAccess(refImg);
  mitk::ImageAccessLock lock(&imAccess);
  picDesc->data = imAccess.getData();

  return picDesc;
}

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(itk::SmartPointer<mitk::ImageDataItem> refItem, mitkIpPicDescriptor *picDesc)
{
  // initialize dimension information
  for (unsigned int i=0; i<8; i++)
  {
      picDesc->n[i] = 1;
  }

  // get the dimensionality information from image item
  picDesc->dim = refItem->GetDimension();
  for( unsigned int i=0; i<picDesc->dim; i++)
  {
    picDesc->n[i] = refItem->GetDimension(i);
  }

  picDesc->type = CastToIpPicType( refItem->GetPixelType().GetComponentType() );
  picDesc->bpe = refItem->GetPixelType().GetBpe();
  picDesc->data = refItem->GetData();

  return picDesc;

}


mitk::ImageDescriptor::Pointer mitk::CastToImageDescriptor(mitkIpPicDescriptor *desc)
{
  mitk::ImageDescriptor::Pointer imDescriptor = mitk::ImageDescriptor::New();

  imDescriptor->Initialize( desc->n, desc->dim );

  mitk::PixelType ptype = CastToPixelType( desc->type, (desc->bpe/8) );
  imDescriptor->AddNewChannel(ptype, "imported by pic");

  return imDescriptor;
}

mitkIpPicType_t mitk::CastToIpPicType( int intype )
{
  //const std::type_info& intype = ptype.GetTypeId();

  //MITK_INFO << "Casting to PicType from " << intype.name() << std::endl;

  const bool isSignedIntegralType = (   intype == itk::ImageIOBase::INT
                                 || intype == itk::ImageIOBase::SHORT
                                 || intype == itk::ImageIOBase::CHAR
                                 || intype == itk::ImageIOBase::LONG );

  const bool isUnsignedIntegralType = (   intype == itk::ImageIOBase::UINT
                                   || intype == itk::ImageIOBase::USHORT
                                   || intype == itk::ImageIOBase::UCHAR
                                   || intype == itk::ImageIOBase::ULONG );

  const bool isFloatingPointType = (   intype == itk::ImageIOBase::FLOAT
                                || intype == itk::ImageIOBase::DOUBLE );

  if( isSignedIntegralType ) return mitkIpPicInt;
  if( isUnsignedIntegralType ) return mitkIpPicUInt;
  if( isFloatingPointType ) return mitkIpPicFloat;
  return mitkIpPicUnknown;
}

mitk::PixelType mitk::CastToPixelType(mitkIpPicType_t pictype, size_t bpe)
{
    const bool isSignedIntegralType = (pictype == mitkIpPicInt );
    const bool isUnsignedIntegralType = (pictype == mitkIpPicUInt );

    if(isSignedIntegralType)
    {
        switch(bpe)
        {
        case sizeof(char):
            return MakeScalarPixelType<char>();
        case sizeof(short):
            return MakeScalarPixelType<short>();
        default:
            return MakeScalarPixelType<int>();
        }
    }
    else if( isUnsignedIntegralType )
    {
        switch(bpe)
        {
        case sizeof(unsigned char):
            return MakeScalarPixelType<unsigned char>();
        case sizeof(unsigned short):
            return MakeScalarPixelType<unsigned short>();
        default:
            return MakeScalarPixelType<unsigned int>();
        }
    }
    else // is floating point type
    {
        switch(bpe)
        {
        case sizeof(float):
            return MakeScalarPixelType<float>();
        default:
            return MakeScalarPixelType<double>();
        }
    }

}
