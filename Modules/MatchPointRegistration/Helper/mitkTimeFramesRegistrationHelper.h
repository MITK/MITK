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

#ifndef __MITK_TIME_FRAMES_REGISTRATION_HELPER_H_
#define __MITK_TIME_FRAMES_REGISTRATION_HELPER_H_


#include <mitkImage.h>
#include <mitkTimeGeometry.h>
#include <mitkImageMappingHelper.h>

#include <mapRegistrationAlgorithmBase.h>
#include <mapRegistrationBase.h>
#include <mapEvents.h>

#include "MitkMatchPointRegistrationExports.h"

namespace mitk
{

  mapEventMacro(FrameRegistrationEvent, ::map::events::TaskBatchEvent, MITKMATCHPOINTREGISTRATION_EXPORT);
  mapEventMacro(FrameMappingEvent, ::map::events::TaskBatchEvent, MITKMATCHPOINTREGISTRATION_EXPORT);

  /** Helper class that assumes that registeres time frames of an passed image and returns the resulting new image.
   * A typical use case for the helper class is motion correction in 3D+t images. Each selected frame will be registered
   * to the first frame of the image. The user can define frames that may be not registered. These frames will be copied directly.
   * Per default all frames will be registered.
   * The user may set a mask for the target frame (1st frame). If this mask image has mulitple time steps, the first time step will be used.
   * The helper class invokes three eventtypes: \n
   * - mitk::FrameRegistrationEvent: when ever a frame was registered.
   * - mitk::FrameMappingEvent: when ever a frame was mapped registered.
   * - itk::ProgressEvent: when ever a new frame was added to the result image.
   */
  class MITKMATCHPOINTREGISTRATION_EXPORT TimeFramesRegistrationHelper : public itk::Object
  {
  public:
    mitkClassMacroItkParent(TimeFramesRegistrationHelper, itk::Object);

    itkNewMacro(Self);

    typedef ::map::algorithm::RegistrationAlgorithmBase RegistrationAlgorithmBaseType;
    typedef RegistrationAlgorithmBaseType::Pointer RegistrationAlgorithmPointer;
    typedef ::map::core::RegistrationBase RegistrationType;
    typedef RegistrationType::Pointer RegistrationPointer;

    typedef std::vector<mitk::TimeStepType> IgnoreListType;

    itkSetConstObjectMacro(4DImage, Image);
    itkGetConstObjectMacro(4DImage, Image);

    itkSetConstObjectMacro(TargetMask, Image);
    itkGetConstObjectMacro(TargetMask, Image);

    itkSetObjectMacro(Algorithm, RegistrationAlgorithmBaseType);
    itkGetObjectMacro(Algorithm, RegistrationAlgorithmBaseType);

    itkSetMacro(AllowUndefPixels, bool);
    itkGetConstMacro(AllowUndefPixels, bool);

    itkSetMacro(PaddingValue, double);
    itkGetConstMacro(PaddingValue, double);

    itkSetMacro(AllowUnregPixels, bool);
    itkGetConstMacro(AllowUnregPixels, bool);

    itkSetMacro(ErrorValue, double);
    itkGetConstMacro(ErrorValue, double);

    itkSetMacro(InterpolatorType, mitk::ImageMappingInterpolator::Type);
    itkGetConstMacro(InterpolatorType, mitk::ImageMappingInterpolator::Type);

    /** cleares the ignore list. Therefore all frames will be processed.*/
    void ClearIgnoreList();
    void SetIgnoreList(const IgnoreListType& il);
    itkGetConstMacro(IgnoreList, IgnoreListType);

    virtual double GetProgress() const;

    /** Commences the generation of the registered 4D image. Stores the result internally.
    * After this method call is finished the result can be retrieved via
    * GetRegisteredImage.
    * @pre 4D image must be set
    * @pre 4D image must has more then one frame
    * @pre Reg algorithm must be set
    * @pre Ignore list values must be within the time geometry of the image
    * @post Result image was generated.*/
    void Generate();

    /** Returns the generated images. Triggers Generate() if result is outdated.
       * @pre 4D image must be set
       * @pre 4D image must has more then one frame
       * @pre Reg algorithm must be set
       * @pre Ignore list values must be within the time geometry of the image
    */
    Image::Pointer GetRegisteredImage();

  protected:
    TimeFramesRegistrationHelper() :
      m_AllowUndefPixels(true),
      m_PaddingValue(0),
      m_AllowUnregPixels(true),
      m_ErrorValue(0),
      m_InterpolatorType(mitk::ImageMappingInterpolator::Linear),
      m_Progress(0)
    {
      m_4DImage = nullptr;
      m_TargetMask = nullptr;
      m_Registered4DImage = nullptr;
    };

    ~TimeFramesRegistrationHelper() override {};

    RegistrationPointer DoFrameRegistration(const mitk::Image* movingFrame,
                                            const mitk::Image* targetFrame, const mitk::Image* targetMask) const;

    mitk::Image::Pointer DoFrameMapping(const mitk::Image* movingFrame, const RegistrationType* reg,
                                        const mitk::Image* targetFrame) const;

    bool HasOutdatedResult() const;
    /** Check if the fit can be generated and all needed inputs are valid.
    * Throw an exception for a non valid or missing input.*/
    void CheckValidInputs() const;

    mitk::Image::Pointer GetFrameImage(const mitk::Image* image, mitk::TimePointType timePoint) const;

    RegistrationAlgorithmPointer m_Algorithm;

  private:
    Image::ConstPointer m_4DImage;
    Image::ConstPointer m_TargetMask;
    Image::Pointer m_Registered4DImage;

    IgnoreListType m_IgnoreList;

    /**Indicates if the mapper should allow undefined pixels (true) or mapping should fail (false)*/
    bool m_AllowUndefPixels;
    /** Value of undefined pixels. Only relevant if m_allowUndefPixels is true. */
    double m_PaddingValue;
    /**Indicates if the mapper should allow pixels that are not covered by the registration (true) or mapping should fail (false)*/
    bool m_AllowUnregPixels;
    /** Value of unreged pixels. Only relevant if m_allowUnregPixels is true. */
    double m_ErrorValue;
    /** Type of interpolator. Only relevant for images and if m_doGeometryRefinement is false. */
    mitk::ImageMappingInterpolator::Type m_InterpolatorType;

    double m_Progress;
  };

}

#endif // __MITK_PARAMETER_FIT_IMAGE_GENERATOR_H_
