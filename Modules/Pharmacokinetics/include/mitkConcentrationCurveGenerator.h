/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkConcentrationCurveGenerator_h
#define mitkConcentrationCurveGenerator_h

#include <mitkImage.h>
#include <itkBinaryFunctorImageFilter.h>
#include "mitkConvertToConcentrationAbsoluteFunctor.h"
#include "mitkConvertToConcentrationRelativeFunctor.h"

#include "MitkPharmacokineticsExports.h"

namespace mitk {

/** \class ConcentrationCurveGenerator
* \brief Converts a given 4D mitk::Image with MR signal values into a 4D mitk::Image with corresponding contrast agent concentration values
*
* From a given 4D image, the Generator takes the 3D image of the first time point as baseline image. It then loops over all time steps, casts
* the current 3D image to itk and passes it to the ConvertToconcentrationFunctor. The returned 3D image has now values of concentration type and is stored at its timepoint
* in the return image.
*/
class MITKPHARMACOKINETICS_EXPORT ConcentrationCurveGenerator : public itk::Object
{
public:

    mitkClassMacroItkParent(ConcentrationCurveGenerator, itk::Object);
    itkNewMacro(Self);

    //typedef itk::Image<double,3> ImageType;
    typedef itk::Image<double,3> ConvertedImageType;


    /** Getter and Setter for 4D mitk::Image*/
    itkSetConstObjectMacro(DynamicImage,Image);
    itkGetConstObjectMacro(DynamicImage,Image);

    /** Parameters Relevant for conversion Calculation; Have to be Set externally (Sequence Dependend)*/
    itkSetMacro(RelaxationTime, double);
    itkGetConstReferenceMacro(RelaxationTime, double);

    itkSetMacro(Relaxivity, double);
    itkGetConstReferenceMacro(Relaxivity, double);

    itkSetMacro(RecoveryTime, double);
    itkGetConstReferenceMacro(RecoveryTime, double);

    itkSetMacro(FlipAngle, double);
    itkGetConstReferenceMacro(FlipAngle, double);

    itkSetMacro(Factor, double);
    itkGetConstReferenceMacro(Factor, double);

    /** Getter and Setter for T10 Map image*/
    itkSetConstObjectMacro(T10Image,Image);
    itkGetConstObjectMacro(T10Image,Image);

    itkSetMacro(T2Factor, double);
    itkGetConstReferenceMacro(T2Factor, double);

    itkSetMacro(T2EchoTime, double);
    itkGetConstReferenceMacro(T2EchoTime, double);


    /** @brief Calls Convert and returns the 4D mitk::image in Concentration units*/

    itkSetMacro(BaselineStartTimeStep, unsigned int);
    itkGetConstReferenceMacro(BaselineStartTimeStep, unsigned int);

    itkSetMacro(BaselineEndTimeStep, unsigned int);
    itkGetConstReferenceMacro(BaselineEndTimeStep, unsigned int);

    itkSetMacro(isTurboFlashSequence,bool);
    itkGetConstReferenceMacro(isTurboFlashSequence,bool);

    itkSetMacro(AbsoluteSignalEnhancement,bool);
    itkGetConstReferenceMacro(AbsoluteSignalEnhancement,bool);

    itkSetMacro(RelativeSignalEnhancement,bool);
    itkGetConstReferenceMacro(RelativeSignalEnhancement,bool);

    itkSetMacro(UsingT1Map,bool);
    itkGetConstReferenceMacro(UsingT1Map,bool);


    itkSetMacro(isT2weightedImage,bool);
    itkGetConstReferenceMacro(isT2weightedImage,bool);

    Image::Pointer GetConvertedImage();

protected:

    ConcentrationCurveGenerator();
     ~ConcentrationCurveGenerator() override;


     template<class TPixel_input, class TPixel_baseline>
     mitk::Image::Pointer convertToConcentration(const itk::Image<TPixel_input, 3> *itkInputImage, const itk::Image<TPixel_baseline, 3> *itkBaselineImage);

    /** Calls ConvertToconcentrationFunctor for passed 3D itk::image*/
    mitk::Image::Pointer ConvertSignalToConcentrationCurve(const mitk::Image* inputImage, const mitk::Image* baselineImage);



    /** @brief Takes the 3D image of the first timepoint to set as baseline image*/
    void PrepareBaselineImage();

    template<class TPixel>
    void CalculateAverageBaselineImage(const itk::Image<TPixel,4> *itkBaselineImage);

    /** @brief loops over all timepoints, casts the current timepoint 3D mitk::image to itk and passes it to ConvertSignalToConcentrationCurve */
    virtual void Convert();


private:
    Image::ConstPointer m_DynamicImage;
    Image::ConstPointer m_BaselineImage;
    Image::ConstPointer m_T10Image;
    Image::Pointer m_ConvertSignalToConcentrationCurve_OutputImage;
    Image::Pointer m_ConvertedImage;

    bool m_isT2weightedImage;

    bool m_isTurboFlashSequence;
    bool m_AbsoluteSignalEnhancement;
    bool m_RelativeSignalEnhancement;
    bool m_UsingT1Map;

    double m_Factor;
    //=Repetition Time TR
    double m_RecoveryTime;
    //= pre-CA T1 time
    double m_RelaxationTime;
    //= contrast agent relaxivity
    double m_Relaxivity;

    double m_FlipAngle;

    double m_T2Factor;
    double m_T2EchoTime;
    // The baseline image is averaged from the signal within time step range [m_BaselineStartTimeStep, m_BaselineEndTimeStep].
    // m_BaselineStartTimeStep is the first time frame, that is included into the baseline averaging (starting with 0).
    unsigned int m_BaselineStartTimeStep;
    // m_BaselinStopTimeStep is the last time frame, that is included into the baseline averaging.
    unsigned int m_BaselineEndTimeStep;
};

}
#endif
