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
#ifndef CONCENTRATIONCURVEGENERATOR_H
#define CONCENTRATIONCURVEGENERATOR_H

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
     ~ConcentrationCurveGenerator();

    template<class Tpixel>
    mitk::Image::Pointer convertToConcentration(const mitk::Image* inputImage, const mitk::Image* baselineImage);


    /** Calls ConvertToconcentrationFunctor for passed 3D itk::image*/
    mitk::Image::Pointer ConvertSignalToConcentrationCurve(const mitk::Image* inputImage, const mitk::Image* baselineImage);



    /** @brief Takes the 3D image of the first timepoint to set as baseline image*/
    void PrepareBaselineImage();

    /** @brief loops over all timepoints, casts the current timepoint 3D mitk::image to itk and passes it to ConvertSignalToConcentrationCurve */
    virtual void Convert();


private:
    Image::ConstPointer m_DynamicImage;
    Image::ConstPointer m_BaselineImage;
    Image::ConstPointer m_T10Image;

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
};

}
#endif // CONCENTRATIONCURVEGENERATOR_H
