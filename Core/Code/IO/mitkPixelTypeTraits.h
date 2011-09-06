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

DEFINE_TYPE_PRIMITIVE(unsigned char);
DEFINE_TYPE_PRIMITIVE(char);
DEFINE_TYPE_PRIMITIVE(signed char);
DEFINE_TYPE_PRIMITIVE(unsigned short);
DEFINE_TYPE_PRIMITIVE(short);
DEFINE_TYPE_PRIMITIVE(unsigned int);
DEFINE_TYPE_PRIMITIVE(int);
DEFINE_TYPE_PRIMITIVE(long int);
DEFINE_TYPE_PRIMITIVE(long unsigned int);
DEFINE_TYPE_PRIMITIVE(float);
DEFINE_TYPE_PRIMITIVE(double);

/** \brief Type trait to provide compile-time check for T ?= itk::VectorImage */
template< class T, typename TValueType >
struct isVectorImage
{
  static const bool value = false;
};

/** Partial specification for the isVectorImage trait. */
template< typename TValueType >
struct isVectorImage< itk::VariableLengthVector<TValueType>, TValueType>
{
  static const bool value = true;
};

template<bool flag,typename T>
struct PixelTypeTrait
{
  typedef T ValueType;
};

template<typename T>
struct PixelTypeTrait<false, T>
{
    typedef typename T::ValueType ValueType;
};

template<typename T>
struct GetComponentType
{
    typedef typename PixelTypeTrait<isPrimitiveType<T>::value, T>::ValueType ComponentType;
};

template<bool V, typename T>
struct ComponentsTrait
{
  static const size_t Size = 1;
};

template<typename T>
struct ComponentsTrait<false, T>
{
  static const size_t Size = T::ValueType::Length;
};


#endif // PIXELTYPETRAITS_H
