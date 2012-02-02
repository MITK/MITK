#include "mitkLegacyAdaptors.h"

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

  picDesc->type = CastToIpPicType( refImg->GetPixelType().GetTypeId() );
  picDesc->bpe = refImg->GetPixelType().GetBpe();
  picDesc->data = refImg->GetData();

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

  picDesc->type = CastToIpPicType( refItem->GetPixelType().GetTypeId() );
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

mitkIpPicType_t mitk::CastToIpPicType( const std::type_info& intype )
{
  //const std::type_info& intype = ptype.GetTypeId();

  //MITK_INFO << "Casting to PicType from " << intype.name() << std::endl;

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
