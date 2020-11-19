/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITK_MODEL_FIT_CMD_APPS_HELPER_H_
#define _MITK_MODEL_FIT_CMD_APPS_HELPER_H_

// std includes
#include <string>

// itk includes
#include "itksys/SystemTools.hxx"

// MITK includes
#include <mitkParameterFitImageGeneratorBase.h>
#include <mitkModelFitInfo.h>
#include <mitkModelFitResultHelper.h>
#include <mitkIOUtil.h>

#include "MitkModelFitExports.h"

namespace mitk
{

  /** Helper function that generates the file path that would be used to store an result image.
  The output path will be determined given the outputPathTemplate (which determines the directory,
  the basic file name and the file formate). The output file name is:  \<basic file name\>_\<parameterName\>.\<extension indicated by outputPathTemplate\>*/
  MITKMODELFIT_EXPORT std::string generateModelFitResultImagePath(const std::string& outputPathTemplate, const std::string& parameterName);

  /** Helper function that takes the given image and stores it based on a template path.
  The real output path will be determined given the outputPathTemplate (which determines the directory,
  the basic file name and the file formate). The output file name is:  \<basic file name\>_\<parameterName\>.\<extension indicated by outputPathTemplate\>*/
  MITKMODELFIT_EXPORT void storeParameterResultImage(const std::string& outputPathTemplate, const std::string& parameterName, mitk::Image* image, mitk::modelFit::Parameter::Type parameterType = mitk::modelFit::Parameter::ParameterType);

  /** Helper function that takes the given image, sets its properties according to the fit session and stores it.
  The output path will be determined given the outputPathTemplate (which determines the directory,
  the basic file name and the file formate). The output file name is:  \<basic file name\>_\<parameterName\>.\<extension indicated by outputPathTemplate\>*/
  MITKMODELFIT_EXPORT void storeModelFitResultImage(const std::string& outputPathTemplate, const std::string& parameterName, mitk::Image* image, mitk::modelFit::Parameter::Type nodeType, const mitk::modelFit::ModelFitInfo* modelFitInfo);

  /** Helper function that stores all results of the passed generator according to the passed outputPathTemplate.
   For further information regarding the output file path, please see storeModelFitResultImage().*/
  MITKMODELFIT_EXPORT void storeModelFitGeneratorResults(const std::string& outputPathTemplate, mitk::ParameterFitImageGeneratorBase* generator, const mitk::modelFit::ModelFitInfo* fitSession);

  /** Helper function that outputs on the std::cout the result images the generator would produces.*/
  MITKMODELFIT_EXPORT void previewModelFitGeneratorResults(const std::string& outputPathTemplate, mitk::ParameterFitImageGeneratorBase* generator);

}
#endif
