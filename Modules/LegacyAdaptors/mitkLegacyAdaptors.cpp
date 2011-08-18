#include "mitkLegacyAdaptors.h"

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(mitk::Image::Pointer refImg)
{
  // create descriptor
  mitkIpPicDescriptor *picDesc = mitkIpPicNew();

  const mitk::ImageDescriptor::Pointer imDesc = refImg->GetImageDescriptor();

  // set dimension information
  picDesc->dim = refImg->GetDimension();
  memcpy( picDesc->n, imDesc->GetDimensions(), picDesc->dim );

  // set pixel type
  picDesc->type = CastToIpPicType( refImg->GetPixelType() );

  // FIXME:
  picDesc->data = imDesc->GetChannelDescriptor(0)->GetData();

  return picDesc;
}

mitk::ImageDescriptor::Pointer mitk::CastToImageDescriptor(mitkIpPicDescriptor *desc)
{

}

mitkIpPicType_t mitk::CastToIpPicType(const mitk::PixelType &ptype)
{
  const std::type_info& intype = ptype.GetTypeId();

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
