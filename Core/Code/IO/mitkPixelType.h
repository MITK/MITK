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

#include <MitkExports.h>
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


//##Documentation
//## @brief Class for defining the data type of pixels
//##
//## To obtain additional type information not provided by this class
//## itk::ImageIOBase can be used by passing the return value of
//## PixelType::GetItkTypeId() to itk::ImageIOBase::SetPixelTypeInfo
//## and using the itk::ImageIOBase methods GetComponentType,
//## GetComponentTypeAsString, GetPixelType, GetPixelTypeAsString.
//## @ingroup Data
class MITK_CORE_EXPORT PixelType
{
public:

  typedef itk::ImageIOBase::IOPixelType ItkIOPixelType;
  typedef itk::ImageIOBase::IOComponentType ItkIOComponentType;

  PixelType(const mitk::PixelType & aPixelType);

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

  PixelType( const int componentType,
             const ItkIOPixelType pixelType,
             std::size_t bytesPerComponent,
             std::size_t numberOfComponents,
             const std::string& componentTypeName,
             const std::string& pixelTypeName);

  // default constructor is disabled on purpose
  PixelType(void);

  // assignment operator declared private on purpose
  PixelType& operator=(const PixelType& other);

  /** \brief the \a type_info of the scalar (!) component type. Each element
    may contain m_NumberOfComponents (more than one) of these scalars.
  */
  const int m_ComponentType;

  const ItkIOPixelType m_PixelType;

  const std::string m_ComponentTypeName;

  const std::string m_PixelTypeName;

  std::size_t m_NumberOfComponents;

  std::size_t m_BytesPerComponent;

};

/** \brief A template method for creating a pixel type.
  */
template< typename ComponentT, typename PixelT, std::size_t numOfComponents >
PixelType MakePixelType()
{
  typedef itk::Image< PixelT, numOfComponents> ItkImageType;

  return PixelType( MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOComponentType,
                    MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOPixelType,
                    sizeof(ComponentT), numOfComponents,
                    PixelComponentTypeToString<ComponentT>(),
                    PixelTypeToString<PixelT>()
                   );
}

/** \brief A template method for creating a pixel type from an ItkImageType
  *
  * For fixed size vector images ( i.e. images of type itk::FixedArray<3,float> ) also the number of components
  * is propagated to the constructor
  */
template< typename ItkImageType >
PixelType MakePixelType()
{
  // define new type, since the ::PixelType is used to distinguish between simple and compound types
  typedef typename ItkImageType::PixelType ImportPixelType;

  // get the component type ( is either directly ImportPixelType or ImportPixelType::ValueType for compound types )
  typedef typename GetComponentType<ImportPixelType>::ComponentType ComponentT;

  // The PixelType is the same as the ComponentT for simple types
  typedef typename ItkImageType::PixelType PixelT;

  // Get the length of compound type ( initialized to 1 for simple types and variable-length vector images)
  size_t numComp = ComponentsTrait<
    (isPrimitiveType<PixelT>::value || isVectorImage<PixelT, ComponentT>::value), ItkImageType >::Size;

  // call the constructor
  return PixelType(
            MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOComponentType,
            MapPixelType<PixelT, isPrimitiveType<PixelT>::value >::IOPixelType,
            sizeof(ComponentT), numComp,
            PixelComponentTypeToString<ComponentT>(),
            PixelTypeToString<PixelT>()
         );
}

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
