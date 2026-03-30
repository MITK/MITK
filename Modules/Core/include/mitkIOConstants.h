/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIOConstants_h
#define mitkIOConstants_h

#include <MitkCoreExports.h>

#include <string>

namespace mitk
{
  /**
   * \ingroup IO
   * \brief Defines string constants for I/O option keys and values.
   *
   * These constants are used by raw image readers and other I/O classes to
   * specify pixel types, dimensions, endianness, and image sizes as
   * key-value option pairs.
   */
  struct MITKCORE_EXPORT IOConstants
  {
    /** \brief Key string for pixel type option. */
    static std::string PIXEL_TYPE();
    /** \brief Value string for char pixel type. */
    static std::string PIXEL_TYPE_CHAR();
    /** \brief Value string for unsigned char pixel type. */
    static std::string PIXEL_TYPE_UCHAR();
    /** \brief Value string for short pixel type. */
    static std::string PIXEL_TYPE_SHORT();
    /** \brief Value string for unsigned short pixel type. */
    static std::string PIXEL_TYPE_USHORT();
    /** \brief Value string for int pixel type. */
    static std::string PIXEL_TYPE_INT();
    /** \brief Value string for unsigned int pixel type. */
    static std::string PIXEL_TYPE_UINT();
    /** \brief Value string for float pixel type. */
    static std::string PIXEL_TYPE_FLOAT();
    /** \brief Value string for double pixel type. */
    static std::string PIXEL_TYPE_DOUBLE();
    /** \brief Key string for pixel type enum option. */
    static std::string PIXEL_TYPE_ENUM();

    /** \brief Key string for dimension option. */
    static std::string DIMENSION();
    /** \brief Key string for dimension enum option. */
    static std::string DIMENSION_ENUM();

    /** \brief Key string for endianness option. */
    static std::string ENDIANNESS();
    /** \brief Value string for little endian byte order. */
    static std::string ENDIANNESS_LITTLE();
    /** \brief Value string for big endian byte order. */
    static std::string ENDIANNESS_BIG();
    /** \brief Key string for endianness enum option. */
    static std::string ENDIANNESS_ENUM();

    /** \brief Key string for image size in X direction. */
    static std::string SIZE_X();
    /** \brief Key string for image size in Y direction. */
    static std::string SIZE_Y();
    /** \brief Key string for image size in Z direction. */
    static std::string SIZE_Z();
    /** \brief Key string for image size in T (time) direction. */
    static std::string SIZE_T();
  };
}

#endif
