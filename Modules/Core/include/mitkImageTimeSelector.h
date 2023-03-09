/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageTimeSelector_h
#define mitkImageTimeSelector_h

#include "mitkSubImageSelector.h"
#include <MitkCoreExports.h>

namespace mitk
{
  //##Documentation
  //## @brief Provides access to a volume at a specific time of the input image
  //##
  //## If the input is generated by a ProcessObject, only the required data is
  //## requested.
  //## @ingroup Process
  class MITKCORE_EXPORT ImageTimeSelector : public SubImageSelector
  {
  public:
    mitkClassMacro(ImageTimeSelector, SubImageSelector);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    itkGetConstMacro(TimeNr, int);
    itkSetMacro(TimeNr, int);

    itkGetConstMacro(ChannelNr, int);
    itkSetMacro(ChannelNr, int);

  protected:
    ImageTimeSelector();

    ~ImageTimeSelector() override;

    void GenerateOutputInformation() override;

    void GenerateInputRequestedRegion() override;

    void GenerateData() override;

    int m_TimeNr;

    int m_ChannelNr;
  };


  /** Convenience helper that makes the application of the ImageTimeSelector one function call. It extracts the
    image for the passed timestep, if the image has multiple time steps.
    If a nullptr is passed as image, a nullptr will be retureed.
    If an image without timesteps is passed, the image will be returned unaltered. The behavior of invalid time definition
    is similar to the ImageTimeSelector filter.*/
  MITKCORE_EXPORT Image::ConstPointer SelectImageByTimeStep(const Image* image, unsigned int timestep);
  /** Non const version.*/
  MITKCORE_EXPORT Image::Pointer SelectImageByTimeStep(Image* image, unsigned int timestep);
  /** Convenience helper that makes the application of the ImageTimeSelector one function call. It extracts the
    image for the passed time point, if the image has multiple time steps.
    If a nullptr is passed as image, a nullptr will be returned.
    If an image without timesteps is passed, the image will be returned unaltered. The behavior of invalid time definition
    is similar to the ImageTimeSelector filter.*/
  MITKCORE_EXPORT Image::ConstPointer SelectImageByTimePoint(const Image* image, TimePointType timePoint);
  /** Non const version.*/
  MITKCORE_EXPORT Image::Pointer SelectImageByTimePoint(Image* image, TimePointType timePoint);

} // namespace mitk

#endif /* IMAGETIMESELECTOR_H_HEADER_INCLUDED_C1E4861D */
