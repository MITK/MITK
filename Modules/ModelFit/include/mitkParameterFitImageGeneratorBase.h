/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __MITK_PARAMETER_FIT_IMAGE_GENERATOR_BASE_H_
#define __MITK_PARAMETER_FIT_IMAGE_GENERATOR_BASE_H_

#include <map>

#include <mitkImage.h>

#include "mitkModelBase.h"

#include "MitkModelFitExports.h"

namespace mitk
{
  /** Base class for generators for parameter fits of a given model.
   * The generators creates 4 types of images:
   * - parameter images: The images that encode the results of each fitted parameter
   * - derived parameter images: Images that encode the results of derived parameters defined by the model
   * - criterion images: Images that encode the criterion value of the fitting strategy for the fitted parameters
   * - evaluation parameter images: Images that encode measures of additional evaluation cost functions defined by the user. (These were not part of the fitting strategy)
   * .
   */
  class MITKMODELFIT_EXPORT ParameterFitImageGeneratorBase: public ::itk::Object
  {
  public:
    mitkClassMacroItkParent(ParameterFitImageGeneratorBase, ::itk::Object);

    using ParameterImagePixelType = ScalarType;

    using ModelBaseType = ModelBase;
    using ParameterNameType = ModelBaseType::ParameterNameType;
    using ParameterNamesType = ModelBaseType::ParameterNamesType;

    using ParameterImageMapType = std::map<ParameterNameType,Image::Pointer>;

    /** Returns the progress of the current fit. e.g. 0 : none; 0.5 = 50%;  1: complete*/
    virtual double GetProgress() const = 0;

    /** Commences the model fit over the dynamic image data. Stores the fitted parameter in
    * parameter images. After this method call is finished the parameter images can be retrieved via
    * GetParameterImages.
    * @pre Model must be set
    * @pre DynamicImage must be set
    * @post Parameter image map contains an image for every parameter of the model referenced by the parameter name.*/
    void Generate();

    /** Returns the fitted/generated parameter images. Triggers Generate() if result is outdated.*/
    ParameterImageMapType GetParameterImages();

    /** Returns the generated derived parameter images. Triggers Generate() if result is outdated.*/
    ParameterImageMapType GetDerivedParameterImages();

    /** Returns the generated criterion images. Triggers Generate() if result is outdated.*/
    ParameterImageMapType GetCriterionImages();

    /** Returns the generated evaluation parameter images. Triggers Generate() if result is outdated.*/
    ParameterImageMapType GetEvaluationParameterImages();

    /** Returns the names of the fitted/generated parameters, that will be generated. These are also the keys of the related image map.*/
    virtual ParameterNamesType GetParameterNames() const = 0;

    /** Returns the names of the derived parameters, that will be generated. These are also the keys of the related image map.*/
    virtual ParameterNamesType GetDerivedParameterNames() const = 0;

    /** Returns the names of the criteria, that will be generated. These are also the keys of the related image map.*/
    virtual ParameterNamesType GetCriterionNames() const = 0;

    /** Returns the names of the evaluation parameters, that will be generated. These are also the keys of the related image map.*/
    virtual ParameterNamesType GetEvaluationParameterNames() const = 0;

  protected:
    ParameterFitImageGeneratorBase() {};
    ~ParameterFitImageGeneratorBase() override {};

    virtual bool HasOutdatedResult() const;
    /** Check if the fit can be generated and all needed inputs are valid.
    * Throw an exception for a non valid or missing input.*/
    virtual void CheckValidInputs() const;
    virtual void DoFitAndGetResults(ParameterImageMapType& parameterImages, ParameterImageMapType& derivedParameterImages, ParameterImageMapType& criterionImages, ParameterImageMapType& evaluationParameterImages) = 0;

    itk::TimeStamp m_GenerationTimeStamp;

  private:

    ParameterImageMapType m_ParameterImageMap;
    ParameterImageMapType m_DerivedParameterImageMap;
    ParameterImageMapType m_CriterionImageMap;
    ParameterImageMapType m_EvaluationParameterImageMap;
  };

}

#endif // __MITK_PARAMETER_FIT_IMAGE_GENERATOR_H_
