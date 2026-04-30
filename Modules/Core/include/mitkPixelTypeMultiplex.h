/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/**
 * \file mitkPixelTypeMultiplex.h
 * \brief Preprocessor macros that dispatch a templated function call based on an image's pixel component type at run time.
 *
 * Each macro checks the component type of a mitk::PixelType and instantiates the
 * given templated function with the matching C++ scalar type (char, unsigned char,
 * short, unsigned short, int, unsigned int, long int, unsigned long int, float, double).
 *
 * Variants are provided for zero to five additional parameters beyond the PixelType:
 * - \c mitkPixelTypeMultiplex0(function, ptype)
 * - \c mitkPixelTypeMultiplex1(function, ptype, param1)
 * - \c mitkPixelTypeMultiplex2(function, ptype, param1, param2)
 * - \c mitkPixelTypeMultiplex3(function, ptype, param1, param2, param3)
 * - \c mitkPixelTypeMultiplex4(function, ptype, param1, param2, param3, param4)
 * - \c mitkPixelTypeMultiplex5(function, ptype, param1, param2, param3, param4, param5)
 *
 * \note For \c mitkPixelTypeMultiplex4 and \c mitkPixelTypeMultiplex5, if no component type
 *       matches, the function is called with \c double as a fallback to avoid compiler warnings.
 *
 * \ingroup Core
 */

#ifndef mitkPixelTypeMultiplex_h
#define mitkPixelTypeMultiplex_h

/**
 * \brief Dispatch a templated function with zero extra parameters based on pixel component type.
 *
 * \param function The templated function to call (instantiated with the matching scalar type).
 * \param ptype    A mitk::PixelType whose component type drives the dispatch.
 */
#define mitkPixelTypeMultiplex0(function, ptype)                                                                       \
                                                                                                                       \
  {                                                                                                                    \
    if (ptype.GetComponentType() == itk::IOComponentEnum::CHAR)                                                        \
      function<char>(ptype);                                                                                           \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UCHAR)                                                  \
      function<unsigned char>(ptype);                                                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::SHORT)                                                  \
      function<short>(ptype);                                                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::USHORT)                                                 \
      function<unsigned short>(ptype);                                                                                 \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::INT)                                                    \
      function<int>(ptype);                                                                                            \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UINT)                                                   \
      function<unsigned int>(ptype);                                                                                   \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::LONG)                                                   \
      function<long int>(ptype);                                                                                       \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::ULONG)                                                  \
      function<unsigned long int>(ptype);                                                                              \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::FLOAT)                                                  \
      function<float>(ptype);                                                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::DOUBLE)                                                 \
      function<double>(ptype);                                                                                         \
  }

/**
 * \brief Dispatch a templated function with one extra parameter based on pixel component type.
 *
 * \param function The templated function to call.
 * \param ptype    A mitk::PixelType whose component type drives the dispatch.
 * \param param1   First additional parameter forwarded to \a function.
 */
#define mitkPixelTypeMultiplex1(function, ptype, param1)                                                               \
                                                                                                                       \
  {                                                                                                                    \
    if (ptype.GetComponentType() == itk::IOComponentEnum::CHAR)                                                        \
      function<char>(ptype, param1);                                                                                   \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UCHAR)                                                  \
      function<unsigned char>(ptype, param1);                                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::SHORT)                                                  \
      function<short>(ptype, param1);                                                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::USHORT)                                                 \
      function<unsigned short>(ptype, param1);                                                                         \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::INT)                                                    \
      function<int>(ptype, param1);                                                                                    \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UINT)                                                   \
      function<unsigned int>(ptype, param1);                                                                           \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::LONG)                                                   \
      function<long int>(ptype, param1);                                                                               \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::ULONG)                                                  \
      function<unsigned long int>(ptype, param1);                                                                      \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::FLOAT)                                                  \
      function<float>(ptype, param1);                                                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::DOUBLE)                                                 \
      function<double>(ptype, param1);                                                                                 \
  }

/**
 * \brief Dispatch a templated function with two extra parameters based on pixel component type.
 *
 * \param function The templated function to call.
 * \param ptype    A mitk::PixelType whose component type drives the dispatch.
 * \param param1   First additional parameter forwarded to \a function.
 * \param param2   Second additional parameter forwarded to \a function.
 */
#define mitkPixelTypeMultiplex2(function, ptype, param1, param2)                                                       \
                                                                                                                       \
  {                                                                                                                    \
    if (ptype.GetComponentType() == itk::IOComponentEnum::CHAR)                                                        \
      function<char>(ptype, param1, param2);                                                                           \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UCHAR)                                                  \
      function<unsigned char>(ptype, param1, param2);                                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::SHORT)                                                  \
      function<short>(ptype, param1, param2);                                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::USHORT)                                                 \
      function<unsigned short>(ptype, param1, param2);                                                                 \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::INT)                                                    \
      function<int>(ptype, param1, param2);                                                                            \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UINT)                                                   \
      function<unsigned int>(ptype, param1, param2);                                                                   \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::LONG)                                                   \
      function<long int>(ptype, param1, param2);                                                                       \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::ULONG)                                                  \
      function<unsigned long int>(ptype, param1, param2);                                                              \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::FLOAT)                                                  \
      function<float>(ptype, param1, param2);                                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::DOUBLE)                                                 \
      function<double>(ptype, param1, param2);                                                                         \
  }

/**
 * \brief Dispatch a templated function with three extra parameters based on pixel component type.
 *
 * \param function The templated function to call.
 * \param ptype    A mitk::PixelType whose component type drives the dispatch.
 * \param param1   First additional parameter forwarded to \a function.
 * \param param2   Second additional parameter forwarded to \a function.
 * \param param3   Third additional parameter forwarded to \a function.
 */
#define mitkPixelTypeMultiplex3(function, ptype, param1, param2, param3)                                               \
                                                                                                                       \
  {                                                                                                                    \
    if (ptype.GetComponentType() == itk::IOComponentEnum::CHAR)                                                        \
      function<char>(ptype, param1, param2, param3);                                                                   \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UCHAR)                                                  \
      function<unsigned char>(ptype, param1, param2, param3);                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::SHORT)                                                  \
      function<short>(ptype, param1, param2, param3);                                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::USHORT)                                                 \
      function<unsigned short>(ptype, param1, param2, param3);                                                         \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::INT)                                                    \
      function<int>(ptype, param1, param2, param3);                                                                    \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UINT)                                                   \
      function<unsigned int>(ptype, param1, param2, param3);                                                           \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::LONG)                                                   \
      function<long int>(ptype, param1, param2, param3);                                                               \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::ULONG)                                                  \
      function<unsigned long int>(ptype, param1, param2, param3);                                                      \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::FLOAT)                                                  \
      function<float>(ptype, param1, param2, param3);                                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::DOUBLE)                                                 \
      function<double>(ptype, param1, param2, param3);                                                                 \
  }

/**
 * \brief Dispatch a templated function with four extra parameters based on pixel component type.
 *
 * If no component type matches, falls back to \c double to avoid Clang compiler warnings.
 *
 * \param function The templated function to call.
 * \param ptype    A mitk::PixelType whose component type drives the dispatch.
 * \param param1   First additional parameter forwarded to \a function.
 * \param param2   Second additional parameter forwarded to \a function.
 * \param param3   Third additional parameter forwarded to \a function.
 * \param param4   Fourth additional parameter forwarded to \a function.
 */
// we have to have a default for, else Clang 3.6.1 complains about problems if 'if evaluates to false'
// therefore if type does not match double is assumed
#define mitkPixelTypeMultiplex4(function, ptype, param1, param2, param3, param4)                                       \
                                                                                                                       \
  {                                                                                                                    \
    if (ptype.GetComponentType() == itk::IOComponentEnum::CHAR)                                                        \
      function<char>(ptype, param1, param2, param3, param4);                                                           \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UCHAR)                                                  \
      function<unsigned char>(ptype, param1, param2, param3, param4);                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::SHORT)                                                  \
      function<short>(ptype, param1, param2, param3, param4);                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::USHORT)                                                 \
      function<unsigned short>(ptype, param1, param2, param3, param4);                                                 \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::INT)                                                    \
      function<int>(ptype, param1, param2, param3, param4);                                                            \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UINT)                                                   \
      function<unsigned int>(ptype, param1, param2, param3, param4);                                                   \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::LONG)                                                   \
      function<long int>(ptype, param1, param2, param3, param4);                                                       \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::ULONG)                                                  \
      function<unsigned long int>(ptype, param1, param2, param3, param4);                                              \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::FLOAT)                                                  \
      function<float>(ptype, param1, param2, param3, param4);                                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::DOUBLE)                                                 \
      function<double>(ptype, param1, param2, param3, param4);                                                         \
    else if (true)                                                                                                     \
      function<double>(ptype, param1, param2, param3, param4);                                                         \
  }

/**
 * \brief Dispatch a templated function with five extra parameters based on pixel component type.
 *
 * If no component type matches, falls back to \c double to avoid Clang compiler warnings.
 *
 * \param function The templated function to call.
 * \param ptype    A mitk::PixelType whose component type drives the dispatch.
 * \param param1   First additional parameter forwarded to \a function.
 * \param param2   Second additional parameter forwarded to \a function.
 * \param param3   Third additional parameter forwarded to \a function.
 * \param param4   Fourth additional parameter forwarded to \a function.
 * \param param5   Fifth additional parameter forwarded to \a function.
 */
// we have to have a default for, else Clang 3.6.1 complains about problems if 'if evaluates to false'
// therefore if type does not match double is assumed
#define mitkPixelTypeMultiplex5(function, ptype, param1, param2, param3, param4, param5)                               \
                                                                                                                       \
  {                                                                                                                    \
    if (ptype.GetComponentType() == itk::IOComponentEnum::CHAR)                                                        \
      function<char>(ptype, param1, param2, param3, param4, param5);                                                   \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UCHAR)                                                  \
      function<unsigned char>(ptype, param1, param2, param3, param4, param5);                                          \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::SHORT)                                                  \
      function<short>(ptype, param1, param2, param3, param4, param5);                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::USHORT)                                                 \
      function<unsigned short>(ptype, param1, param2, param3, param4, param5);                                         \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::INT)                                                    \
      function<int>(ptype, param1, param2, param3, param4, param5);                                                    \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::UINT)                                                   \
      function<unsigned int>(ptype, param1, param2, param3, param4, param5);                                           \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::LONG)                                                   \
      function<long int>(ptype, param1, param2, param3, param4, param5);                                               \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::ULONG)                                                  \
      function<unsigned long int>(ptype, param1, param2, param3, param4, param5);                                      \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::FLOAT)                                                  \
      function<float>(ptype, param1, param2, param3, param4, param5);                                                  \
    else if (ptype.GetComponentType() == itk::IOComponentEnum::DOUBLE)                                                 \
      function<double>(ptype, param1, param2, param3, param4, param5);                                                 \
    else                                                                                                               \
      function<double>(ptype, param1, param2, param3, param4, param5);                                                 \
  }

#endif
