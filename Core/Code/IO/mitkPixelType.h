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


#ifndef PIXELTYPE_H_HEADER_INCLUDED_C1EBF565
#define PIXELTYPE_H_HEADER_INCLUDED_C1EBF565

#include <MitkCoreExports.h>
#include "mitkCommon.h"
#include "mitkPixelTypeTraits.h"

#include <typeinfo>
#include <string>

#include <itkImageIOBase.h>
#include <itkImage.h>

namespace mitk {

template<typename T>
std::string PixelComponentTypeToString()
{
  return itk::ImageIOBase::GetComponentTypeAsString(itk::ImageIOBase::MapPixelType<T>::CType);
}

template<typename PixelT>
std::string PixelTypeToString()
{
  return std::string();
}


/**
 * @brief Class for defining the data type of pixels
 *
 * To obtain additional type information not provided by this class
 * itk::ImageIOBase can be used by passing the return value of
 * PixelType::GetItkTypeId() to itk::ImageIOBase::SetPixelTypeInfo
 * and using the itk::ImageIOBase methods GetComponentType,
 * GetComponentTypeAsString, GetPixelType, GetPixelTypeAsString.
 * @ingroup Data
 */
class MITK_CORE_EXPORT PixelType
{
public:

  typedef itk::ImageIOBase::IOPixelType ItkIOPixelType;
  typedef itk::ImageIOBase::IOComponentType ItkIOComponentType;

  PixelType(const mitk::PixelType & aPixelType);
  PixelType& operator=(const PixelType& other);

  itk::ImageIOBase::IOPixelType GetPixelType() const;

  /**
   * \brief Get the \a component type (the scalar (!) type). Each element
   * may contain m_NumberOfComponents (more than one) of these scalars.
   *
   */
  int GetComponentType() const;

  /**
   * \brief Returns a string containing the ITK pixel type name.
   */
  std::string GetPixelTypeAsString() const;

  /**
   * \brief Returns a string containing the name of the component.
   */
  std::string GetComponentTypeAsString() const;

  /**
   * \brief Returns a string representing the pixel type and pixel components.
   */
  std::string GetTypeAsString() const;

  /**
   * \brief Get size of the PixelType in bytes
   *
   * A RGBA PixelType of floats will return 4 * sizeof(float)
   */
  size_t GetSize() const;

  /**
   * \brief Get the number of bits per element (of an
   * element)
   *
   * A vector of double with three components will return
   * 8*sizeof(double)*3.
   * \sa GetBitsPerComponent
   * \sa GetItkTypeId
   * \sa GetTypeId
   */
  size_t GetBpe() const;

  /**
   * \brief Get the number of components of which each element consists
   *
   * Each pixel can consist of multiple components, e.g. RGB.
   */
  size_t GetNumberOfComponents() const;

  /**
   * \brief Get the number of bits per components
   * \sa GetBitsPerComponent
   */
  size_t GetBitsPerComponent() const;

  bool operator==(const PixelType& rhs) const;
  bool operator!=(const PixelType& rhs) const;

  ~PixelType();

private:

  friend PixelType MakePixelType(const itk::ImageIOBase* imageIO);

  template< typename ComponentT, typename PixelT, std::size_t numberOfComponents >
  friend PixelType MakePixelType();

  template< typename ItkImageType >
  friend PixelType MakePixelType();

  template< typename ItkImageType >
  friend PixelType MakePixelType(size_t);

  PixelType( const int componentType,
             const ItkIOPixelType pixelType,
             std::size_t bytesPerComponent,
             std::size_t numberOfComponents,
             const std::string& componentTypeName,
             const std::string& pixelTypeName);

  // default constructor is disabled on purpose
  PixelType(void);


  /** \brief the \a type_info of the scalar (!) component type. Each element
    may contain m_NumberOfComponents (more than one) of these scalars.
  */
  int m_ComponentType;

  ItkIOPixelType m_PixelType;

  std::string m_ComponentTypeName;

  std::string m_PixelTypeName;

  std::size_t m_NumberOfComponents;

  std::size_t m_BytesPerComponent;

};

/**
 * \brief A template method for creating a pixel type.
 */
template< typename ComponentT, typename PixelT, std::size_t numOfComponents >
PixelType MakePixelType()
{
  return PixelType( MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOComponentType,
                    MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOPixelType,
                    sizeof(ComponentT), numOfComponents,
                    PixelComponentTypeToString<ComponentT>(),
                    PixelTypeToString<PixelT>()
                   );
}

/**
 * \brief A helper template for compile-time checking of supported ITK image types.
 *
 * Unsupported image types will be marked by template specializations with
 * missing definitions;
 */
template< typename ItkImageType >
struct AssertImageTypeIsValid
{
};

// The itk::VariableLengthVector pixel type is not supported in MITK if it is
// used with an itk::Image (it cannot be represented as a mitk::Image object).
// Use a itk::VectorImage instead.
template< typename TPixelType, unsigned int VImageDimension >
struct AssertImageTypeIsValid<itk::Image<itk::VariableLengthVector<TPixelType>, VImageDimension> >;

/** \brief A template method for creating a MITK pixel type na ITK image type
 *
 * \param numOfComponents The number of components for the pixel type of the ITK image
 */
template< typename ItkImageType >
PixelType MakePixelType( std::size_t numOfComponents )
{
  AssertImageTypeIsValid<ItkImageType>();

  // define new type, since the ::PixelType is used to distinguish between simple and compound types
  typedef typename ItkImageType::PixelType ImportPixelType;

  // get the component type ( is either directly ImportPixelType or ImportPixelType::ValueType for compound types )
  typedef typename GetComponentType<ImportPixelType>::ComponentType ComponentT;

  // The PixelType is the same as the ComponentT for simple types
  typedef typename ItkImageType::PixelType PixelT;

  // call the constructor
  return PixelType(
            MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOComponentType,
            MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOPixelType,
            sizeof(ComponentT), numOfComponents,
            PixelComponentTypeToString<ComponentT>(),
            PixelTypeToString<PixelT>()
         );
}

/** \brief A template method for creating a MITK pixel type from an ITK image
 *         pixel type and dimension
 *
 * \param numOfComponents The number of components for the pixel type \c TPixelType
 */
template< typename TPixelType, unsigned int VImageDimension >
PixelType MakePixelType( std::size_t numOfComponents )
{
  typedef typename ImageTypeTrait<TPixelType, VImageDimension>::ImageType ItkImageType;
  return MakePixelType<ItkImageType>(numOfComponents);
}

/** \brief A template method for creating a MITK pixel type from an ITK image
 *         pixel type and dimension
 *
 * For images where the number of components of the pixel type is determined at
 * runtime (e.g. pixel types like itk::VariableLengthVector<short>) the
 * MakePixelType(std::size_t) function must be used.
 */
template< typename ItkImageType >
PixelType MakePixelType()
{
  if( ImageTypeTrait<ItkImageType>::IsVectorImage )
  {
    mitkThrow() << " Variable pixel type given but the length is not specified. Use the parametric MakePixelType( size_t ) method instead.";
  }

  // Use the InternalPixelType to get "1" for the number of components in case of
  // a itk::VectorImage
  typedef typename ItkImageType::InternalPixelType PixelT;

  const std::size_t numComp = ComponentsTrait<isPrimitiveType<PixelT>::value, ItkImageType>::Size;

  // call the constructor
  return MakePixelType<ItkImageType>(numComp);
}

/**
 * \brief Create a MITK pixel type based on a itk::ImageIOBase object
 */
inline PixelType MakePixelType(const itk::ImageIOBase* imageIO)
{
  return mitk::PixelType(imageIO->GetComponentType(), imageIO->GetPixelType(),
                         imageIO->GetComponentSize(), imageIO->GetNumberOfComponents(),
                         imageIO->GetComponentTypeAsString(imageIO->GetComponentType()),
                         imageIO->GetPixelTypeAsString(imageIO->GetPixelType()));
}

/** \brief An interface to the MakePixelType method for creating scalar pixel types.
  *
  * Usage: for example MakeScalarPixelType<short>() for a scalar short image
  */
template< typename T>
PixelType MakeScalarPixelType()
{
    return MakePixelType<T,T,1>();
}

} // namespace mitk



#endif /* PIXELTYPE_H_HEADER_INCLUDED_C1EBF565 */
