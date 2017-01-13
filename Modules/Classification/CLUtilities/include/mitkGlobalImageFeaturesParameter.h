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

#ifndef mitkGlobalImageFeaturesParameter_h
#define mitkGlobalImageFeaturesParameter_h

#include "MitkCLUtilitiesExports.h"
#include "mitkCommandLineParser.h"

#include <string>

namespace mitk
{
  namespace cl
  {
    class MITKCLUTILITIES_EXPORT GlobalImageFeaturesParameter
    {
    public:
      void AddParameter(mitkCommandLineParser &parser);
      void ParseParameter(std::map<std::string, us::Any> parsedArgs);

      std::string imagePath;
      std::string maskPath;
      std::string outputPath;
    };
  }
}



#endif //mitkGlobalImageFeaturesParameter_h