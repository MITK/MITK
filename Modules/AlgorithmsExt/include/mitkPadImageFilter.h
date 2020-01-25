/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPADIMAGEFILTER_H_HEADER_INCLUDED
#define MITKPADIMAGEFILTER_H_HEADER_INCLUDED

#include "MitkAlgorithmsExtExports.h"
#include "mitkCommon.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageToImageFilter.h"

#include "itkImage.h"

namespace mitk
{
  /**
   * \brief PadImageFilter class pads the first input image to the size of the second input image.
   * Two Images have to be set.
   * The first image is the image to pad. The second image defines the pad size.
   * It is also possible to use an included binary filter.
   *
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT PadImageFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(PadImageFilter, ImageToImageFilter);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /** \brief Sets the intensity of the pixel to pad */
      itkSetMacro(PadConstant, int);

    /** \brief sets the binary filter ON or OFF */
    itkSetMacro(BinaryFilter, bool);

    /** \brief Sets the lower threshold of the included binary filter */
    itkSetMacro(LowerThreshold, int);

    /** \brief Sets the upper threshold of the included binary filter */
    itkSetMacro(UpperThreshold, int);

  protected:
    PadImageFilter();

    ~PadImageFilter() override;

    void GenerateData() override;

  private:
    bool m_BinaryFilter;
    int m_PadConstant, m_LowerThreshold, m_UpperThreshold;
  };

} // namespace mitk

#endif /* MITKPADIMAGEFILTER_H_HEADER_INCLUDED */
