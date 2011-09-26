#include "mitkLegacyAdaptors.h"

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(mitk::Image::Pointer refImg, mitkIpPicDescriptor* picDesc)
{
  // create descriptor
  // mitkIpPicDescriptor *picDesc = mitkIpPicNew();

  const mitk::ImageDescriptor::Pointer imDesc = refImg->GetImageDescriptor();

  // set dimension information
  picDesc->dim = refImg->GetDimension();
  memcpy( picDesc->n, imDesc->GetDimensions(), picDesc->dim * sizeof(unsigned int) );

  if( refImg->IsValidChannel(0) )
  {
    // set pixel type
    picDesc->type = CastToIpPicType( refImg->GetPixelType().GetTypeId() );
    picDesc->data = refImg->GetChannelData(0);
  }
  else
  {
    picDesc->type = mitkIpPicUnknown;
    picDesc->data = NULL;
  }

  return picDesc;
}

mitk::ImageDescriptor::Pointer mitk::CastToImageDescriptor(mitkIpPicDescriptor *desc)
{
  mitk::ImageDescriptor::Pointer imDescriptor = mitk::ImageDescriptor::New();

  imDescriptor->Initialize( desc->n, desc->dim );

  mitk::PixelType ptype = CastToPixelType( desc->type, desc->bpe );
  imDescriptor->AddNewChannel(ptype, "imported by pic");

  return imDescriptor;
}

mitkIpPicType_t mitk::CastToIpPicType( const std::type_info& intype )
{
  //const std::type_info& intype = ptype.GetTypeId();

  MITK_INFO << "Casting to PicType from " << intype.name() << std::endl;

  const bool isSignedIntegralType = (   intype == typeid(int)
                                 || intype == typeid(short)
                                 || intype == typeid(char)
                                 || intype == typeid(long int) );

  const bool isUnsignedIntegralType = (   intype == typeid(unsigned int)
                                   || intype == typeid(unsigned short)
                                   || intype == typeid(unsigned char)
                                   || intype == typeid(unsigned long int) );

  const bool isFloatingPointType = (   intype == typeid(float)
                                || intype == typeid(double) );

  if( isSignedIntegralType ) return mitkIpPicInt;
  if( isUnsignedIntegralType ) return mitkIpPicUInt;
  if( isFloatingPointType ) return mitkIpPicFloat;
  return mitkIpPicUnknown;
}

mitk::PixelType mitk::CastToPixelType(mitkIpPicType_t pictype, size_t bpe)
{
    const bool isSignedIntegralType = (pictype == mitkIpTypeInt1
                                       || pictype == mitkIpTypeInt2
                                       || pictype == mitkIpTypeInt4 );

    const bool isUnsignedIntegralType = (pictype == mitkIpTypeUInt1
                                         || pictype == mitkIpTypeUInt2
                                         || pictype == mitkIpTypeUInt4 );

    const bool isFloatingPointType = (   pictype == mitkIpTypeFloat4
                                         || pictype == mitkIpTypeFloat8 );

    if(isSignedIntegralType)
    {
        switch(bpe)
        {
        case sizeof(char):
            return MakeSimpleType<char>();
        case sizeof(short):
            return MakeSimpleType<short>();
        default:
            return MakeSimpleType<int>();
        }
    }
    else if( isUnsignedIntegralType )
    {
        switch(bpe)
        {
        case sizeof(unsigned char):
            return MakeSimpleType<unsigned char>();
        case sizeof(unsigned short):
            return MakeSimpleType<unsigned short>();
        default:
            return MakeSimpleType<unsigned int>();
        }
    }
    else // is floating point type
    {
        switch(bpe)
        {
        case sizeof(float):
            return MakeSimpleType<float>();
        default:
            return MakeSimpleType<double>();
        }
    }

}
