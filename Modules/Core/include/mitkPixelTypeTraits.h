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

#ifndef PIXELTYPETRAITS_H
#define PIXELTYPETRAITS_H

#include <itkImageIOBase.h>
#include <itkRGBPixel.h>
#include <itkRGBAPixel.h>
#include <itkDiffusionTensor3D.h>
#include <itkImage.h>
#include <itkVectorImage.h>

/** \file mitkPixelTypeTraits.h
  *
  * The pixel type traits are in general used for compile time resolution of the component type and
  * the number of components for compound types like the ones in ItkImageType.
  * The default values are used to define the corresponding variable also for scalar types
  */

namespace itk
{
  /** Forward declaration of the Variable Length Vector class from ITK */
  template < typename TValueType > class VariableLengthVector;
}

#define MITK_PIXEL_COMPONENT_TYPE(type,ctype,name)     \
  template <> struct mitk::MapPixelComponentType<type> \
  {                                                    \
    static const int value = ctype; \
  } \
  template <> std::string mitk::PixelComponentTypeToString() \
  { \
    return name; \
  }

namespace mitk {

static const int PixelUserType = itk::ImageIOBase::MATRIX + 1;
static const int PixelComponentUserType = itk::ImageIOBase::DOUBLE + 1;

/**
 * Maps pixel component types (primitive types like int, short, double, etc. and custom
 * types) to and integer constant. Specialize this template for custom types by using the
 * #MITK_PIXEL_COMPONENT_TYPE macro.
 */
template<typename T>
struct MapPixelComponentType
{
  static const int value = itk::ImageIOBase::MapPixelType<T>::CType;
};

/**
  \brief This is an implementation of a type trait to provide a compile-time check for PixelType used in
  the instantiation of an itk::Image
*/
template< typename T>
struct isPrimitiveType
{
    static const bool value = false;
};

/** \def DEFINE_TYPE_PRIMITIVE macro which provides a partial specialization for the \sa isPrimitiveType
  object */
#define DEFINE_TYPE_PRIMITIVE(_TYPEIN) \
  template<> struct isPrimitiveType<_TYPEIN>{ static const bool value = true; }

/** \brief Partial specialization (unsigned char) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(unsigned char);
/** \brief Partial specialization (char) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(char);
/** \brief Partial specialization (signed char) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(signed char);
/** \brief Partial specialization (unsigned short) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(unsigned short);
/** \brief Partial specialization (short) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(short);
/** \brief Partial specialization (unsigned int) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(unsigned int);
/** \brief Partial specialization (int) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(int);
/** \brief Partial specialization (long int) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(long int);
/** \brief Partial specialization (long unsigned int) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(long unsigned int);
/** \brief Partial specialization (float) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(float);
/** \brief Partial specialization (double) for the isPrimitiveType object */
DEFINE_TYPE_PRIMITIVE(double);

template<typename TPixelType, unsigned int VDimension = 0>
struct ImageTypeTrait
{
  typedef itk::Image<TPixelType, VDimension> ImageType;
  static const bool IsVectorImage = false;
};

template<typename TPixelType, unsigned int VDimension>
struct ImageTypeTrait<itk::VariableLengthVector<TPixelType>, VDimension >
{
  typedef itk::VectorImage<TPixelType, VDimension> ImageType;
  static const bool IsVectorImage = true;
};

template<typename T>
struct ImageTypeTrait<T, 0>
{
  typedef T ImageType;
  static const bool IsVectorImage = false;
};

template<typename TPixelType, unsigned int VDimension>
struct ImageTypeTrait<itk::VectorImage<TPixelType, VDimension>, 0>
{
  typedef itk::VectorImage<TPixelType, VDimension> ImageType;
  static const bool IsVectorImage = true;
};

/** \brief Compile-time trait for resolving the ValueType from an ItkImageType */
template<bool flag,typename T>
struct PixelTypeTrait
{
  typedef T ValueType;
};

/** \brief Partial specialization for the PixelTypeTrait
*
* Specialization for the false value. Used to define the value type for non-primitive pixel types
*/
template<typename T>
struct PixelTypeTrait<false, T>
{
    typedef typename T::ValueType ValueType;
};

/** \brief Compile time resolving of the type of a component */
template<typename T>
struct GetComponentType
{
    typedef typename PixelTypeTrait<isPrimitiveType<T>::value, T>::ValueType ComponentType;
};

/** \brief Object for compile-time resolving of the number of components for given type.
  *
  * Default value for the component number is 1
*/
template<bool V, typename T>
struct ComponentsTrait
{
  static const size_t Size = 1;
};

/** \brief Partial specialization for the ComponentsTraits in case of compound types */
template<typename T>
struct ComponentsTrait<false, T>
{
  static const size_t Size = T::ValueType::Length;
};

typedef itk::ImageIOBase::IOPixelType itkIOPixelType;

/** \brief Object for compile-time translation of a composite pixel type into an itk::ImageIOBase::IOPixelType information
 *
 * The default value of the IOCompositeType is the UNKNOWNPIXELTYPE, the default value will be used for all but the
 * types below with own partial specialization. The values of the IOCompositeType member in the specializations correspond
 * to the values of the itk::ImageIOBase::IOPixelType enum values.
 */
template< class T>
struct MapCompositePixelType
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::UNKNOWNPIXELTYPE;
};

//------------------------
// Partial template specialization for fixed-length types
//------------------------

template< class C>
struct MapCompositePixelType< itk::RGBPixel<C> >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::RGB;
};

template< class C>
struct MapCompositePixelType< itk::RGBAPixel<C> >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::RGBA;
};

template< class C>
struct MapCompositePixelType< itk::DiffusionTensor3D<C> >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::DIFFUSIONTENSOR3D;
};

template< class C>
struct MapCompositePixelType< itk::VariableLengthVector<C> >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::VECTOR;
};

//------------------------
// Partial template specialization for variable-length types
//------------------------
template< class C, unsigned int N>
struct MapCompositePixelType< itk::Vector< C,N > >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::VECTOR;
};

template< class C, unsigned int N>
struct MapCompositePixelType< itk::FixedArray< C,N > >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::COVARIANTVECTOR;
};

template< class C, unsigned int N>
struct MapCompositePixelType< itk::CovariantVector< C,N > >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::COVARIANTVECTOR;
};

template< class C, unsigned int N>
struct MapCompositePixelType< itk::Matrix< C,N > >
{
  static const itkIOPixelType IOCompositeType = itk::ImageIOBase::MATRIX;
};

/** \brief Object for compile-time translation of a pixel type into an itk::ImageIOBase::IOPixelType information
 *
 * The first template parameter is the pixel type to be translated, the second parameter determines the processing
 * way. For non-primitive types the first template parameter is passed to the MapCompositePixelType object to be resolved there
 * for primitive types the value is set to SCALAR.
 *
 * To initalize the flag correctly in compile-time use the \sa isPrimitiveType<T> trait.
 */
template< class T, bool Primitive>
struct MapPixelType
{
  static const itkIOPixelType IOPixelType = MapCompositePixelType<T>::IOCompositeType;
  static const int IOComponentType = MapPixelComponentType<typename GetComponentType<T>::ComponentType>::value;
};

/** \brief Partial specialization for setting the IOPixelType for primitive types to SCALAR */
template<class T>
struct MapPixelType< T, true>
{
  static const itkIOPixelType IOPixelType = itk::ImageIOBase::SCALAR;
  static const int IOComponentType = MapPixelComponentType<T>::value;
};

} // end namespace mitk

#endif // PIXELTYPETRAITS_H
