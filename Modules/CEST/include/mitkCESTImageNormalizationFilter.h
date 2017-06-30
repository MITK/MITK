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
#ifndef __mitkCESTImageNormalizationFilter_h
#define __mitkCESTImageNormalizationFilter_h

#include <MitkCESTExports.h>

// MITK
#include "mitkImageToImageFilter.h"

namespace mitk
{
  /** \brief Normalization filter for CEST images.
   *
   * This filter can be used to normalize CEST images, it only works with 4D images and assumes that the input
   * mitk::Image has a property called mitk::CustomTagParser::m_OffsetsPropertyName, whith offsets separated by
   * spaces. The number of offsets has to match the number of timesteps.
   *
   * Each timestep with a corresponding offset greater than 299 or less than -299 will be interpreted as normalization (M0) image.
   * If only one M0 image is present normalization will be done by dividing the voxel value by the corresponding
   * M0 voxel value. If multiple M0 images are present normalization between any two M0 images will be done by
   * dividing by a linear interpolation between the two.
   * The M0 images themselves will be transferred to the result without any processing.
   *
   * The output image will have the same geometry as the input image and a double pixel type.
   */
  class MITKCEST_EXPORT CESTImageNormalizationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(CESTImageNormalizationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

  protected:
    /*!
    \brief standard constructor
    */
    CESTImageNormalizationFilter();
    /*!
    \brief standard destructor
    */
    ~CESTImageNormalizationFilter();
    /*!
    \brief Method generating the output information of this filter (e.g. image dimension, image type, etc.).
    The interface ImageToImageFilter requires this implementation. Everything is taken from the input image.
    */
    virtual void GenerateOutputInformation() override;
    /*!
    \brief Method generating the output of this filter. Called in the updated process of the pipeline.
    This method generates the normalized output image.
    */
    virtual void GenerateData() override;

    /** Internal templated method that normalizes across timesteps
    */
    template <typename TPixel, unsigned int VImageDimension>
    void NormalizeTimeSteps(const itk::Image<TPixel, VImageDimension>* image);

    /// Offsets without M0s
    std::string m_RealOffsets;

    /// non M0 indices
    std::vector< unsigned int > m_NonM0Indices;

  };
} // END mitk namespace
#endif
