/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAterialInputFunctionGenerator_h
#define mitkAterialInputFunctionGenerator_h

#include <mitkImage.h>
#include <mitkAIFBasedModelBase.h>
#include <MitkPharmacokineticsExports.h>


namespace mitk
{
  /** \class AterialInputFunctionGenerator
   * \brief Computes the Arterial Input Function (AIF) from a 4D dynamic image and an artery mask.
   *
   * This generator takes a 4D dynamic image (already converted to concentration values) and a
   * corresponding 3D mask defining the tumour-supplying artery. It computes the mean concentration
   * within the mask at each time point and returns the resulting AIF values and time grid.
   *
   * No signal-to-concentration conversion is performed internally; the input image must already
   * contain concentration values. The generator uses itkMaskedNaryStatisticsImageFilter for spatial
   * averaging and mitkExtractTimeGrid for time grid extraction.
   *
   * \pre Both dynamic image and mask must be set before calling GetAterialInputFunction().
   * \sa AIFBasedModelBase, ConcentrationCurveGenerator
   */
  class MITKPHARMACOKINETICS_EXPORT AterialInputFunctionGenerator : public itk::Object
  {
  public:

    mitkClassMacroItkParent(AterialInputFunctionGenerator, itk::Object);
    itkNewMacro(Self);

    /** \brief Sets the 4D input image (already in concentration units) for AIF computation.
     *  \param[in] _arg The 4D dynamic concentration image. */
    itkSetConstObjectMacro(DynamicImage, Image);
    /** \brief Returns the currently set 4D dynamic input image. */
    itkGetConstObjectMacro(DynamicImage, Image);

    /** \brief Sets the 3D mask image defining the tumour-supplying artery ROI.
     *  \param[in] _arg The artery mask image. */
    itkSetConstObjectMacro(Mask, Image);
    /** \brief Returns the currently set artery mask image. */
    itkGetConstObjectMacro(Mask, Image);

    /** \brief Sets the hematocrit level for plasma curve conversion.
     *  \param[in] _arg Hematocrit level as a fraction (e.g. 0.45). */
    itkSetMacro(HCL, double);
    /** \brief Returns the currently set hematocrit level.
     *  \return The hematocrit level. */
    itkGetConstReferenceMacro(HCL, double);

    /** \brief Default hematocrit level value (0.45). */
    static const double DEFAULT_HEMATOCRIT_LEVEL;

    /** \brief Resets the hematocrit level to the default value (0.45). */
    void SetDefaultHematocritLevel()
    {
        this->m_HCL = DEFAULT_HEMATOCRIT_LEVEL;
    };
    /** \brief Returns the default hematocrit level constant (0.45).
     *  \return The default hematocrit level. */
    double GetDefaultHematocritLevel()
    {
        return DEFAULT_HEMATOCRIT_LEVEL;
    }

    /** \brief Computes and returns the AIF concentration values.
     *
     * Triggers recalculation if inputs have changed since the last computation.
     * \pre DynamicImage and Mask must be set.
     * \return Array of mean concentration values within the artery mask at each time point. */
    AIFBasedModelBase::AterialInputFunctionType GetAterialInputFunction();
    /** \brief Computes and returns the AIF time grid.
     *
     * Triggers recalculation if inputs have changed since the last computation.
     * \pre DynamicImage and Mask must be set.
     * \return Time grid extracted from the dynamic image. */
    ModelBase::TimeGridType GetAterialInputFunctionTimeGrid();

  protected:
    AterialInputFunctionGenerator()
    {
      m_Mask = nullptr;
      m_DynamicImage = nullptr;
      this->SetDefaultHematocritLevel();
    };

    ~AterialInputFunctionGenerator() override {};

    //template <typename TPixel, unsigned int VDim>
    //void DoCalculateAIF(itk::Image<TPixel, VDim>* image);

    /** \brief Passes m_DynamicImage and m_Mask to the itkMaskedNaryStatisticsImageFilter and mitkExtractTimeGrid
     * and inserts the result into m_AIFValues and m_AIFTimeGrid and modifies the Timestamp.*/
    virtual void CalculateAIFAndGetResult();

    /** \brief Makes sure that m_DynamicImage and m_Mask are set. */
    virtual void CheckValidInputs() const;

    bool HasOutdatedResults();


    itk::TimeStamp m_GenerationTimeStamp;

  private:
    Image::ConstPointer m_DynamicImage;
    Image::ConstPointer m_Mask;


    AIFBasedModelBase::AterialInputFunctionType m_AIFValues;
    ModelBase::TimeGridType m_AIFTimeGrid;

    double m_HCL;
  };

}

#endif
