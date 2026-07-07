/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPadImageFilter_h
#define mitkPadImageFilter_h

#include <MitkAlgorithmsExtExports.h>
#include <mitkCommon.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageToImageFilter.h>

#include <itkImage.h>

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

      /**
       * \brief Set the intensity value used for padded pixels (default: -32766).
       * \param[in] _arg The pad constant value.
       */
      itkSetMacro(PadConstant, int);

    /**
     * \brief Enable or disable the binary threshold filter applied after padding.
     *
     * When enabled, the padded image is binarized using LowerThreshold and
     * UpperThreshold, producing an unsigned char output.
     *
     * \param[in] _arg True to enable the binary filter, false to disable.
     */
    itkSetMacro(BinaryFilter, bool);

    /**
     * \brief Set the lower threshold of the binary filter.
     *
     * Pixels with values at or above this threshold (and at or below UpperThreshold)
     * are set to 1; others are set to 0.
     *
     * \param[in] _arg The lower threshold value (default: -32766).
     */
    itkSetMacro(LowerThreshold, int);

    /**
     * \brief Set the upper threshold of the binary filter.
     *
     * Pixels with values at or below this threshold (and at or above LowerThreshold)
     * are set to 1; others are set to 0.
     *
     * \param[in] _arg The upper threshold value (default: -32765).
     */
    itkSetMacro(UpperThreshold, int);

  protected:
    PadImageFilter();

    ~PadImageFilter() override;

    void GenerateData() override;

  private:
    template <typename SourceImageType>
    void GenerateDataInternal(SourceImageType* sourceItkImage, mitk::Image::Pointer outputImage);

    bool m_BinaryFilter;
    int m_PadConstant, m_LowerThreshold, m_UpperThreshold;
  };

} // namespace mitk

#endif
