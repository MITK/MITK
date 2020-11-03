/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _MITKPHOTOACOUSTICSBMODEFILTER_H_
#define _MITKPHOTOACOUSTICSBMODEFILTER_H_

#include <itkObject.h>
#include "mitkImageToImageFilter.h"

namespace mitk
{
  /*!
  * \brief Class implementing a mitk::ImageToImageFilter for BMode filtering on CPU
  *
  *  The only parameter that needs to be provided is whether it should use a logfilter.
  *  Currently this class only performs an absolute BMode filter.
  */
  class PhotoacousticBModeFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PhotoacousticBModeFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Set parameters for the filter
    *
    * @param useLogFilter If true, the filter will apply a logfilter on the processed image
    */
    void UseLogFilter(bool useLogFilter)
    {
      m_UseLogFilter = useLogFilter;
    }

  protected:

    PhotoacousticBModeFilter();

    ~PhotoacousticBModeFilter() override;

    void GenerateInputRequestedRegion() override;

    void GenerateOutputInformation() override;

    void GenerateData() override;

    //##Description
    //## @brief Time when Header was last initialized
    itk::TimeStamp m_TimeOfHeaderInitialization;

    bool m_UseLogFilter;
  };
}
#endif /* _MITKPHOTOACOUSTICSBMODEFILTER_H_ */
