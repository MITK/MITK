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

#ifndef MITKIOCONSTANTS_H_
#define MITKIOCONSTANTS_H_

#include <MitkCoreExports.h>

#include <string>

namespace mitk {

struct MITK_CORE_EXPORT IOConstants
{
  static std::string PIXEL_TYPE();
  static std::string PIXEL_TYPE_CHAR();
  static std::string PIXEL_TYPE_UCHAR();
  static std::string PIXEL_TYPE_SHORT();
  static std::string PIXEL_TYPE_USHORT();
  static std::string PIXEL_TYPE_INT();
  static std::string PIXEL_TYPE_UINT();
  static std::string PIXEL_TYPE_FLOAT();
  static std::string PIXEL_TYPE_DOUBLE();
  static std::string PIXEL_TYPE_ENUM();

  static std::string DIMENSION();
  static std::string DIMENSION_ENUM();

  static std::string ENDIANNESS();
  static std::string ENDIANNESS_LITTLE();
  static std::string ENDIANNESS_BIG();
  static std::string ENDIANNESS_ENUM();

  static std::string SIZE_X();
  static std::string SIZE_Y();
  static std::string SIZE_Z();
  static std::string SIZE_T();
};

}

#endif // MITKIOCONSTANTS_H_
