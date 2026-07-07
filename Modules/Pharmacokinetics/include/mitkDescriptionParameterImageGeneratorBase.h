/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDescriptionParameterImageGeneratorBase_h
#define mitkDescriptionParameterImageGeneratorBase_h

#include <map>

#include <mitkImage.h>

#include <mitkCurveDescriptionParameterBase.h>

#include <MitkPharmacokineticsExports.h>

namespace mitk
{
  /** \class DescriptionParameterImageGeneratorBase
   * \brief Abstract base class for generators that produce parameter images from curve descriptors.
   *
   * Subclasses implement the actual computation strategy (e.g. pixel-based or ROI-based)
   * to generate one or more 3D parameter images from a 4D dynamic image, where each output
   * image represents a specific curve descriptor value (e.g. AUC, maximum, time-to-peak).
   *
   * The generator caches results and only recomputes when inputs have been modified.
   *
   * \sa PixelBasedDescriptionParameterImageGenerator, CurveParameterFunctor
   */
  class MITKPHARMACOKINETICS_EXPORT DescriptionParameterImageGeneratorBase: public ::itk::Object
  {
  public:

    mitkClassMacroItkParent(DescriptionParameterImageGeneratorBase, ::itk::Object);

    /** \brief Pixel type used in generated parameter images. */
    typedef ScalarType ParameterImagePixelType;

    /** \brief String type for parameter names. */
    typedef CurveDescriptionParameterBase::CurveDescriptionParameterNameType ParameterNameType;

    /** \brief Map from parameter names to their corresponding output images. */
    typedef std::map<ParameterNameType,Image::Pointer> ParameterImageMapType;

    /** \brief Returns the current computation progress as a value between 0.0 and 1.0.
     *  \return The computation progress fraction. */
    virtual double GetProgress() const = 0;

    /** \brief Triggers the parameter image computation if results are outdated.
     *
     * Validates inputs, performs computation via DoParameterCalculationAndGetResults(),
     * and caches the results. Does nothing if the current results are still up-to-date. */
     void Generate();

    /** \brief Returns the computed parameter images.
     *
     * Triggers re-generation if results are outdated.
     * \return Map of parameter names to their corresponding output images. */
    ParameterImageMapType GetParameterImages();

  protected:
    DescriptionParameterImageGeneratorBase();
    ~DescriptionParameterImageGeneratorBase() override;

    virtual bool HasOutdatedResult() const;

    virtual void CheckValidInputs() const;
    virtual void DoParameterCalculationAndGetResults(ParameterImageMapType& parameterImages) = 0;

    itk::TimeStamp m_GenerationTimeStamp;

  private:

    ParameterImageMapType m_ParameterImageMap;

  };

}

#endif
