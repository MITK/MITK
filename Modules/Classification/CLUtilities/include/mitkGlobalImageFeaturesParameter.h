/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGlobalImageFeaturesParameter_h
#define mitkGlobalImageFeaturesParameter_h

#include <MitkCLUtilitiesExports.h>
#include <mitkCommandLineParser.h>

#include <string>

namespace mitk
{
  namespace cl
  {
    /**
     * \brief Holds and parses command-line parameters for global image feature extraction.
     *
     * This class manages all parameters needed by the global image feature extraction
     * command-line tool, including image/mask paths, output options, histogram settings,
     * resampling options, and header configuration.
     */
    class MITKCLUTILITIES_EXPORT GlobalImageFeaturesParameter
    {
    public:
      /**
       * \brief Register all supported parameters with the command-line parser.
       * \param parser The command-line parser to add arguments to.
       */
      void AddParameter(mitkCommandLineParser &parser);

      /**
       * \brief Parse and store parameter values from the command-line arguments.
       * \param parsedArgs Map of parsed argument names to values.
       */
      void ParseParameter(std::map<std::string, us::Any> parsedArgs);

      std::string imagePath;
      std::string imageName;
      std::string imageFolder;
      std::string maskPath;
      std::string maskName;
      std::string maskFolder;
      std::string outputPath;
      std::string outputXMLPath;

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
      std::string pipelineUID;
      bool calculateAllFeatures;

    private:
      void ParseFileLocations(std::map<std::string, us::Any> &parsedArgs);
      void ParseAdditionalOutputs(std::map<std::string, us::Any> &parsedArgs);
      void ParseHeaderInformation(std::map<std::string, us::Any> &parsedArgs);
      void ParseMaskAdaptation(std::map<std::string, us::Any> &parsedArgs);
      void ParseGlobalFeatureParameter(std::map<std::string, us::Any> &parsedArgs);

    };
  }
}



#endif
