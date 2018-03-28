%{

#include <mitkBaseData.h>
#include <mitkIOUtil.h>
#include <mitkSlicedData.h>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkChannelDescriptor.h>
#include <mitkPixelType.h>
#include <mitkIntensityQuantifier.h>

// SWIG Doesn't wrap namespaces. This leads to some problems, if the namespaces are not used.
using mitk::DataStorage;
using mitk::IFileReader;
using mitk::IFileWriter;
using mitk::ScalarType;
using mitk::Operation;
using mitk::GeometryTransformHolder;
using mitk::AffineTransform3D;
using mitk::BaseProperty;
using mitk::ImageDescriptor;
using mitk::PropertyList;
using mitk::ImageDataItem;
using mitk::PointSpecificationType;
using mitk::IntensityQuantifier;

using itk::LightObject;
using itk::ModifiedTimeType;
using itk::TimeStamp;
using itk::EventObject;
using itk::MetaDataDictionary;
using itk::SmartPointerForwardReference;
using itk::RealTimeStamp;


std::vector<unsigned int> GetImageSize(mitk::Image::Pointer image)
{
  std::vector< unsigned int > size;
  unsigned int dimension = image->GetDimension();
  for (int i = 0; i < dimension; ++i)
  {
    size.push_back(image->GetDimension(i));
  }
  return size;
}

std::vector<unsigned int> GetImageSize(mitk::Image* image)
{
  std::vector< unsigned int > size;
  unsigned int dimension = image->GetDimension();
  for (int i = 0; i < dimension; ++i)
  {
    size.push_back(image->GetDimension(i));
  }
  return size;
}

struct TypeDefinitions
{
   static const int ComponentTypeUInt8 = mitk::MapPixelType<uint8_t, mitk::isPrimitiveType<uint8_t>::value>::IOComponentType;
   static const int ComponentTypeInt8 = mitk::MapPixelType<int8_t, mitk::isPrimitiveType<int8_t>::value>::IOComponentType;
   static const int ComponentTypeUInt16 = mitk::MapPixelType<uint16_t, mitk::isPrimitiveType<uint16_t>::value>::IOComponentType;
   static const int ComponentTypeInt16 = mitk::MapPixelType<int16_t, mitk::isPrimitiveType<int16_t>::value>::IOComponentType;
   static const int ComponentTypeUInt32 = mitk::MapPixelType<uint32_t, mitk::isPrimitiveType<uint32_t>::value>::IOComponentType;
   static const int ComponentTypeInt32 = mitk::MapPixelType<int32_t, mitk::isPrimitiveType<int32_t>::value>::IOComponentType;
   static const int ComponentTypeFloat = mitk::MapPixelType<float, mitk::isPrimitiveType<float>::value>::IOComponentType;
   static const int ComponentTypeDouble = mitk::MapPixelType<double, mitk::isPrimitiveType<double>::value>::IOComponentType;
};

mitk::PixelType MakePixelTypeFromTypeID(int componentTypeID, int numberOfComponents)
{
   switch (componentTypeID)
   {
      case TypeDefinitions::ComponentTypeUInt8 :
      return mitk::MakePixelType<uint8_t, uint8_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt8 :
      return mitk::MakePixelType<int8_t, int8_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeUInt16 :
      return mitk::MakePixelType<uint16_t, uint16_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt16 :
      return mitk::MakePixelType<int16_t, int16_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeUInt32 :
      return mitk::MakePixelType<uint32_t, uint32_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeInt32 :
      return mitk::MakePixelType<int32_t, int32_t>(numberOfComponents);
      case TypeDefinitions::ComponentTypeFloat :
      return mitk::MakePixelType<float, float>(numberOfComponents);
      case TypeDefinitions::ComponentTypeDouble :
      return mitk::MakePixelType<double, double>(numberOfComponents);
    default:
      return mitk::MakePixelType<double, double>(numberOfComponents);
   }
}

mitk::Image::Pointer MakeImage(mitk::PixelType pixelType, std::vector<unsigned int> shape)
{
    mitk::Image::Pointer image = mitk::Image::New();
  image->Initialize(pixelType, shape.size(), shape.data());
  return image;
}

template<class T>
typename T::Pointer ConvertTo(itk::Object::Pointer base)
{
  typename T::Pointer erg = dynamic_cast<T*>(base.GetPointer());
  return erg;
}

%}

std::vector<unsigned int> GetImageSize(mitk::Image::Pointer image);
std::vector<unsigned int> GetImageSize(mitk::Image* image);
mitk::PixelType MakePixelTypeFromTypeID(int componentTypeID, int numberOfComponents);
mitk::Image::Pointer MakeImage(mitk::PixelType pixelType, std::vector<unsigned int> shape);

%constant int ComponentTypeUInt8 = TypeDefinitions::ComponentTypeUInt8;
%constant int ComponentTypeInt8 = TypeDefinitions::ComponentTypeInt8;
%constant int ComponentTypeUInt16 = TypeDefinitions::ComponentTypeUInt16;
%constant int ComponentTypeInt16 = TypeDefinitions::ComponentTypeInt16;
%constant int ComponentTypeUInt32 = TypeDefinitions::ComponentTypeUInt32;
%constant int ComponentTypeInt32 = TypeDefinitions::ComponentTypeInt32;
%constant int ComponentTypeFloat = TypeDefinitions::ComponentTypeFloat;
%constant int ComponentTypeDouble = TypeDefinitions::ComponentTypeDouble;

template<class T>
typename T::Pointer ConvertTo(itk::Object::Pointer base);

