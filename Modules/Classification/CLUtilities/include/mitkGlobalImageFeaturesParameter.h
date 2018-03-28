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
      std::string imageName;
      std::string imageFolder;
      std::string maskPath;
      std::string maskName;
      std::string maskFolder;
      std::string outputPath;

      std::string morphPath;
      std::string morphName;
      bool useMorphMask;

      bool useLogfile;
      std::string logfilePath;
      bool writeAnalysisImage;
      std::string anaylsisImagePath;
      bool writeAnalysisMask;
      std::string analysisMaskPath;
      bool writePNGScreenshots;
      std::string pngScreenshotsPath;

      bool useHeader;
      bool useHeaderForFirstLineOnly;

      bool ensureSameSpace;
      bool resampleMask;
      bool resampleToFixIsotropic;
      double resampleResolution;

      bool ignoreMaskForHistogram;
      bool defineGlobalMinimumIntensity;
      double globalMinimumIntensity;
      bool defineGlobalMaximumIntensity;
      double globalMaximumIntensity;
      bool defineGlobalNumberOfBins;
      int globalNumberOfBins;
      bool useDecimalPoint;
      char decimalPoint;
      bool encodeParameter;

    private:
      void ParseFileLocations(std::map<std::string, us::Any> &parsedArgs);
      void ParseAdditionalOutputs(std::map<std::string, us::Any> &parsedArgs);
      void ParseHeaderInformation(std::map<std::string, us::Any> &parsedArgs);
      void ParseMaskAdaptation(std::map<std::string, us::Any> &parsedArgs);
      void ParseGlobalFeatureParameter(std::map<std::string, us::Any> &parsedArgs);

    };
  }
}



#endif //mitkGlobalImageFeaturesParameter_h