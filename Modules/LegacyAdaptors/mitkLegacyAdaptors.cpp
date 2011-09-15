#include "mitkLegacyAdaptors.h"

mitkIpPicDescriptor* mitk::CastToIpPicDescriptor(mitk::Image::Pointer refImg)
{
  // create descriptor
  mitkIpPicDescriptor *picDesc = mitkIpPicNew();

  const mitk::ImageDescriptor::Pointer imDesc = refImg->GetImageDescriptor();

  // set dimension information
  picDesc->dim = refImg->GetDimension();
  memcpy( picDesc->n, imDesc->GetDimensions(), picDesc->dim );

  if( refImg->IsValidSlice(0,0,0) )
  {
    // set pixel type
    picDesc->type = CastToIpPicType( refImg->GetPixelType().GetTypeId() );
    picDesc->data = refImg->GetSliceData(0,0,0);//imDesc->GetChannelDescriptor().GetData();
  }
  else
  {
    picDesc->type = mitkIpPicUnknown;
  }

  return picDesc;
}

mitk::ImageDescriptor::Pointer mitk::CastToImageDescriptor(mitkIpPicDescriptor *desc)
{
  mitk::ImageDescriptor::Pointer imDescriptor = mitk::ImageDescriptor::New();

  imDescriptor->Initialize( desc->n, desc->dim );

  // FIXME cast IpPicType to PixelType
  mitk::PixelType ptype = MakePixelType<short,short,1>();
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

mitk::PixelType mitk::CastToPixelType(mitkIpPicType_t pictype)
{
  /*switch( pictype )
  {
    case mitkIpPicInt:

  }*/
}
