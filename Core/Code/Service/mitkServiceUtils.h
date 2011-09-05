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


#ifndef MITKSERVICEUTILS_H
#define MITKSERVICEUTILS_H

#ifndef MITK_HAS_HASH_STRING

#ifdef MITK_HAS_UNORDERED_MAP_H
#include <unordered_map>
namespace std {
#else

#include "mitkItkHashMap.h"
#if defined(__GNUC__)
namespace __gnu_cxx {
#else
namespace itk {
#endif

#endif // MITK_HAS_UNORDERED_MAP_H

template<>
struct hash<std::string>
{
  std::size_t operator()(const std::string& str) const
  {
    return hash<const char*>()(str.c_str());
  }
};

}

#endif // MITK_HAS_HASH_STRING

#endif // MITKSERVICEUTILS_H
