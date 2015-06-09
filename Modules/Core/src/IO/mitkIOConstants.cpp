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

#include "mitkIOConstants.h"

namespace mitk {

std::string IOConstants::PIXEL_TYPE()
{
  static std::string s("org.mitk.io.Pixel Type");
  return s;
}

std::string IOConstants::PIXEL_TYPE_CHAR()
{
  static std::string s("char");
  return s;
}

std::string IOConstants::PIXEL_TYPE_UCHAR()
{
  static std::string s("unsigned char");
  return s;
}

std::string IOConstants::PIXEL_TYPE_SHORT()
{
  static std::string s("short");
  return s;
}

std::string IOConstants::PIXEL_TYPE_USHORT()
{
  static std::string s("unsigned short");
  return s;
}

std::string IOConstants::PIXEL_TYPE_INT()
{
  static std::string s("int");
  return s;
}

std::string IOConstants::PIXEL_TYPE_UINT()
{
  static std::string s("unsigned int");
  return s;
}

std::string IOConstants::PIXEL_TYPE_FLOAT()
{
  static std::string s("float");
  return s;
}

std::string IOConstants::PIXEL_TYPE_DOUBLE()
{
  static std::string s("double");
  return s;
}

std::string IOConstants::PIXEL_TYPE_ENUM()
{
  static std::string s("org.mitk.io.Pixel Type.enum");
  return s;
}

std::string IOConstants::DIMENSION()
{
  static std::string s("org.mitk.io.Dimension");
  return s;
}

std::string IOConstants::DIMENSION_ENUM()
{
  static std::string s("org.mitk.io.Dimension.enum");
  return s;
}

std::string IOConstants::ENDIANNESS()
{
  static std::string s("org.mitk.io.Endianness");
  return s;
}

std::string IOConstants::ENDIANNESS_LITTLE()
{
  static std::string s("Little Endian");
  return s;
}

std::string IOConstants::ENDIANNESS_BIG()
{
  static std::string s("Big Endian");
  return s;
}

std::string IOConstants::ENDIANNESS_ENUM()
{
  static std::string s("org.mitk.io.Endianness.enum");
  return s;
}

std::string IOConstants::SIZE_X()
{
  static std::string s("org.mitk.io.Size X");
  return s;
}

std::string IOConstants::SIZE_Y()
{
  static std::string s("org.mitk.io.Size Y");
  return s;
}

std::string IOConstants::SIZE_Z()
{
  static std::string s("org.mitk.io.Size Z");
  return s;
}

std::string IOConstants::SIZE_T()
{
  static std::string s("org.mitk.io.Size t");
  return s;
}

}
