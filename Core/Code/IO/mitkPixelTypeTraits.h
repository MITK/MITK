/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef PIXELTYPETRAITS_H
#define PIXELTYPETRAITS_H

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

/** \brief Type trait to provide compile-time check for T ?= itk::VectorImage */
template< class T, typename TValueType >
struct isVectorImage
{
  static const bool value = false;
};

/** \brief Partial specification for the isVectorImage trait. */
template< typename TValueType >
struct isVectorImage< itk::VariableLengthVector<TValueType>, TValueType>
{
  static const bool value = true;
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


#endif // PIXELTYPETRAITS_H
