/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkPixelTypeMultiplex_h
#define mitkPixelTypeMultiplex_h

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
