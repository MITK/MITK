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
#include <mitkConvertToConcentrationAbsoluteFunctor.h>
#include <mitkConvertToConcentrationRelativeFunctor.h>

#include <MitkPharmacokineticsExports.h>

namespace mitk {

/** \class ConcentrationCurveGenerator
 * \brief Converts a 4D MR signal image into a 4D contrast agent concentration image.
 *
 * Given a 4D dynamic image with MR signal intensities, this generator computes the
 * baseline image from the specified time step range and then converts each 3D time frame
 * to concentration values using the appropriate conversion functor, selected based on the
 * MR sequence type (TurboFLASH, absolute/relative signal enhancement, T1 mapping via
 * variable flip angle, or T2-weighted).
 *
 * The result is a 4D image where each voxel contains the computed contrast agent
 * concentration at each time point.
 *
 * \pre The 4D dynamic image must be set before calling GetConvertedImage().
 * \pre The appropriate sequence flags and MR parameters must be configured.
 * \sa AterialInputFunctionGenerator, ConvertToConcentrationAbsoluteFunctor,
 *     ConvertToConcentrationRelativeFunctor, ConvertToConcentrationTurboFlashFunctor,
 *     ConvertToConcentrationViaT1CalcFunctor, ConvertT2ConcentrationFunctor
 */
class MITKPHARMACOKINETICS_EXPORT ConcentrationCurveGenerator : public itk::Object
{
public:

    mitkClassMacroItkParent(ConcentrationCurveGenerator, itk::Object);
    itkNewMacro(Self);

    /** \brief ITK image type for converted 3D concentration frames. */
    typedef itk::Image<double,3> ConvertedImageType;


    /** \brief Sets the input 4D dynamic MR signal image.
     *  \param[in] _arg The 4D dynamic image to convert. */
    itkSetConstObjectMacro(DynamicImage,Image);
    /** \brief Returns the currently set 4D dynamic input image. */
    itkGetConstObjectMacro(DynamicImage,Image);

    /** \brief Sets the pre-contrast T1 relaxation time (T10) in ms.
     *  \param[in] _arg The relaxation time value. */
    itkSetMacro(RelaxationTime, double);
    /** \brief Returns the pre-contrast T1 relaxation time. */
    itkGetConstReferenceMacro(RelaxationTime, double);

    /** \brief Sets the contrast agent relaxivity in 1/(mM*s).
     *  \param[in] _arg The relaxivity value. */
    itkSetMacro(Relaxivity, double);
    /** \brief Returns the contrast agent relaxivity. */
    itkGetConstReferenceMacro(Relaxivity, double);

    /** \brief Sets the recovery time for TurboFLASH sequences.
     *  \param[in] _arg The recovery time value. */
    itkSetMacro(RecoveryTime, double);
    /** \brief Returns the recovery time. */
    itkGetConstReferenceMacro(RecoveryTime, double);

    /** \brief Sets the repetition time (TR) in ms.
     *  \param[in] _arg The repetition time value. */
    itkSetMacro(RepetitionTime, double);
    /** \brief Returns the repetition time (TR). */
    itkGetConstReferenceMacro(RepetitionTime, double);

    /** \brief Sets the flip angle in radians.
     *  \param[in] _arg The flip angle value. */
    itkSetMacro(FlipAngle, double);
    /** \brief Returns the flip angle. */
    itkGetConstReferenceMacro(FlipAngle, double);

    /** \brief Sets the flip angle of the proton density weighted (PDW) image in radians.
     *  \param[in] _arg The PDW flip angle value. */
    itkSetMacro(FlipAnglePDW, double);
    /** \brief Returns the PDW flip angle. */
    itkGetConstReferenceMacro(FlipAnglePDW, double);

    /** \brief Sets the conversion scaling factor (k).
     *  \param[in] _arg The scaling factor value. */
    itkSetMacro(Factor, double);
    /** \brief Returns the conversion scaling factor. */
    itkGetConstReferenceMacro(Factor, double);

    /** \brief Sets the proton density weighted (PDW) map image for T1 mapping via variable flip angle.
     *  \param[in] _arg The 3D PDW image. */
    itkSetConstObjectMacro(PDWImage,Image);
    /** \brief Returns the currently set PDW image. */
    itkGetConstObjectMacro(PDWImage,Image);

    /** \brief Sets the T2* relaxivity factor for T2-weighted conversion.
     *  \param[in] _arg The T2 factor value. */
    itkSetMacro(T2Factor, double);
    /** \brief Returns the T2 factor. */
    itkGetConstReferenceMacro(T2Factor, double);

    /** \brief Sets the echo time (TE) for T2-weighted conversion in ms.
     *  \param[in] _arg The echo time value. */
    itkSetMacro(T2EchoTime, double);
    /** \brief Returns the T2 echo time. */
    itkGetConstReferenceMacro(T2EchoTime, double);

    /** \brief Sets the first time step index included in baseline averaging (0-based).
     *  \param[in] _arg The start time step index. */
    itkSetMacro(BaselineStartTimeStep, unsigned int);
    /** \brief Returns the baseline start time step index. */
    itkGetConstReferenceMacro(BaselineStartTimeStep, unsigned int);

    /** \brief Sets the last time step index included in baseline averaging.
     *  \param[in] _arg The end time step index. */
    itkSetMacro(BaselineEndTimeStep, unsigned int);
    /** \brief Returns the baseline end time step index. */
    itkGetConstReferenceMacro(BaselineEndTimeStep, unsigned int);

    /** \brief Sets whether the input is a TurboFLASH sequence.
     *  \param[in] _arg True if TurboFLASH conversion should be used. */
    itkSetMacro(isTurboFlashSequence,bool);
    /** \brief Returns whether TurboFLASH conversion mode is active. */
    itkGetConstReferenceMacro(isTurboFlashSequence,bool);

    /** \brief Sets whether to use absolute signal enhancement for conversion.
     *  \param[in] _arg True to use absolute signal enhancement. */
    itkSetMacro(AbsoluteSignalEnhancement,bool);
    /** \brief Returns whether absolute signal enhancement mode is active. */
    itkGetConstReferenceMacro(AbsoluteSignalEnhancement,bool);

    /** \brief Sets whether to use relative signal enhancement for conversion.
     *  \param[in] _arg True to use relative signal enhancement. */
    itkSetMacro(RelativeSignalEnhancement,bool);
    /** \brief Returns whether relative signal enhancement mode is active. */
    itkGetConstReferenceMacro(RelativeSignalEnhancement,bool);

    /** \brief Sets whether to compute concentration via T1 mapping (variable flip angle method).
     *  \param[in] _arg True to use T1 mapping conversion. */
    itkSetMacro(UsingT1Map,bool);
    /** \brief Returns whether T1 mapping conversion mode is active. */
    itkGetConstReferenceMacro(UsingT1Map,bool);

    /** \brief Sets whether the input is a T2-weighted image.
     *  \param[in] _arg True if the image is T2-weighted. */
    itkSetMacro(isT2weightedImage,bool);
    /** \brief Returns whether T2-weighted conversion mode is active. */
    itkGetConstReferenceMacro(isT2weightedImage,bool);

    /** \brief Performs the conversion and returns the 4D concentration image.
     *
     * Triggers the full conversion pipeline: baseline computation, per-frame conversion
     * using the configured functor, and assembly of the 4D result image.
     * \return Smart pointer to the 4D concentration image. */
    Image::Pointer GetConvertedImage();

protected:

    ConcentrationCurveGenerator();
     ~ConcentrationCurveGenerator() override;


     template<class TPixel_input, class TPixel_baseline>
     mitk::Image::Pointer convertToConcentration(const itk::Image<TPixel_input, 3> *itkInputImage, const itk::Image<TPixel_baseline, 3> *itkBaselineImage);

    /** Calls ConvertToconcentrationFunctor for passed 3D itk::image*/
    mitk::Image::Pointer ConvertSignalToConcentrationCurve(const mitk::Image* inputImage, const mitk::Image* baselineImage);



    /** \brief Takes the 3D image of the first timepoint to set as baseline image.*/
    void PrepareBaselineImage();

    template<class TPixel>
    void CalculateAverageBaselineImage(const itk::Image<TPixel,4> *itkBaselineImage);

    /** \brief Loops over all timepoints, casts the current timepoint 3D mitk::image to itk and passes it to ConvertSignalToConcentrationCurve. */
    virtual void Convert();


private:
    Image::ConstPointer m_DynamicImage;
    Image::ConstPointer m_BaselineImage;
    Image::ConstPointer m_PDWImage;
    Image::Pointer m_ConvertSignalToConcentrationCurve_OutputImage;
    Image::Pointer m_ConvertedImage;

    bool m_isT2weightedImage;

    bool m_isTurboFlashSequence;
    bool m_AbsoluteSignalEnhancement;
    bool m_RelativeSignalEnhancement;
    bool m_UsingT1Map;

    double m_Factor;
    //=Recovery Time
    double m_RecoveryTime;
    //=Repetition Time TR
    double m_RepetitionTime;
    //= pre-CA T1 time
    double m_RelaxationTime;
    //= contrast agent relaxivity
    double m_Relaxivity;

    double m_FlipAngle;

    double m_FlipAnglePDW;

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
