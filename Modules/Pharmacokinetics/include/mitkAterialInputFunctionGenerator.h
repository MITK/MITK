/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef ATERIALINPUTFUNCTIONGENERATOR_H
#define ATERIALINPUTFUNCTIONGENERATOR_H

#include <mitkImage.h>
#include "mitkAIFBasedModelBase.h"
#include "MitkPharmacokineticsExports.h"


namespace mitk
{
  /** \class AterialInputFunctionGenerator
  * \brief Compute the Aterial Input Function from a given dynamic image and a mask defining the tumour supplying artery
  *
  * The AterialInputFunctionGenerator takes a given 4D dynamic image and a corresponding mask and returns an array of the averaged values and
  * an array of the corresponding TimeGrid
  * within the mask over time. No conversion is performed, so  conversion from signal to concentration has to be performed in advanced
  * and the resulting image is fed into the Generator.
  * The generator checks wether both image and mask  are set and passes them to the itkMaskedNaryStatisticsImageFilter and the mitkExtractTimeGrid, to
  * calculate the mean of every time slice within the ROI and extract the corresponding time grid from the date set.
  */
  class MITKPHARMACOKINETICS_EXPORT AterialInputFunctionGenerator : public itk::Object
  {
  public:

    mitkClassMacroItkParent(AterialInputFunctionGenerator, itk::Object);
    itkNewMacro(Self);

    /** @brief Setter and Getter for Input Image for calculation of AIF, already converted to concentrations
    * Getter calls CheckValidInputs() and CalculateAIFAndGetResult() if HasOutdatedResults() is true */
    itkSetConstObjectMacro(DynamicImage, Image);
    itkGetConstObjectMacro(DynamicImage, Image);

    /** @brief Setter and Getter for mask defining the tumour feeding atery
    * Getter calls CheckValidInputs() and CalculateAIFAndGetResult() if HasOutdatedResults() is true */
    itkSetConstObjectMacro(Mask, Image);
    itkGetConstObjectMacro(Mask, Image);

    /** @brief Setter and Getter for the hematocritlevel, important for conversion to plasma curve*/
    itkSetMacro(HCL, double);
    itkGetConstReferenceMacro(HCL, double);

    //Common Value for Hematocrit level is 0.45
    static const double DEFAULT_HEMATOCRIT_LEVEL;

    void SetDefaultHematocritLevel()
    {
        this->m_HCL = DEFAULT_HEMATOCRIT_LEVEL;
    };
    double GetDefaultHematocritLevel()
    {
        return DEFAULT_HEMATOCRIT_LEVEL;
    }

    AIFBasedModelBase::AterialInputFunctionType GetAterialInputFunction();
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

    /** @brief Passes m_DynamicImage and m_Mask to the itkMaskedNaryStatisticsImageFilter and mitkExtractTimeGrid
     * and inserts the result into m_AIFValues and m_AIFTimeGrid and modiefies the Timestamp*/
    virtual void CalculateAIFAndGetResult();

    /** @brief Makes sure that m_DynamicImage and m_Mask are set */
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

#endif // ATERIALINPUTFUNCTIONGENERATOR_H
