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

#ifndef MITKIMIMETYPE_H
#define MITKIMIMETYPE_H

#include <MitkExports.h>

#include "usServiceInterface.h"

#include <string>

namespace mitk {

/**
 * @brief A service interface for mime type information.
 *
 * The service property PROP_ID() contains the actual mime-type,
 * e.g. "image/png". PROP_CATEGORY() is used in file dialogs to
 * group related types. The extensions associated with a mime-type
 * are specified in PROP_EXTENSIONS() (typcially only one, but
 * sometimes more) and a longer format description can be provided
 * with the PROP_DESCRIPTION() property.
 */
struct MITK_CORE_EXPORT IMimeType
{
  virtual ~IMimeType();

  virtual std::string GetMagicPatterns() const = 0;

  static std::string PROP_ID();
  static std::string PROP_CATEGORY();
  static std::string PROP_EXTENSIONS();
  static std::string PROP_DESCRIPTION();
};

}

US_DECLARE_SERVICE_INTERFACE(mitk::IMimeType, "org.mitk.IMimeType")

#endif // MITKIMIMETYPE_H
