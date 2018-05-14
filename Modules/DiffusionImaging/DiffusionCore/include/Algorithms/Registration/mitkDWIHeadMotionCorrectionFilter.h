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

#ifndef MITKDWIHEADMOTIONCORRECTIONFILTER_H
#define MITKDWIHEADMOTIONCORRECTIONFILTER_H

#include "mitkImageToImageFilter.h"

#include <itkAccumulateImageFilter.h>
#include <itkExtractImageFilter.h>

#include "mitkITKImageImport.h"
#include <itkVectorImage.h>
#include <MitkDiffusionCoreExports.h>

namespace mitk
{

/**
 * @class DWIHeadMotionCorrectionFilter
 *
 * @brief Performs standard head-motion correction by using affine registration of the gradient images.
 *
 * (Head) motion correction is a essential pre-processing step before performing any further analysis of a diffusion-weighted
 * images since all model fits ( tensor, ODF ) rely on an aligned diffusion-weighted dataset. The correction is done in two steps. First the
 * unweighted images ( if multiple present ) are separately registered on the first one by means of rigid registration and normalized correlation
 * as error metric. Second, the weighted gradient images are registered to the unweighted reference ( computed as average from the aligned images from first step )
 * by an affine transformation using the MattesMutualInformation metric as optimizer guidance.
 *
 */

class MITKDIFFUSIONCORE_EXPORT DWIHeadMotionCorrectionFilter
    : public ImageToImageFilter
{
public:

  // class macros
  mitkClassMacro( DWIHeadMotionCorrectionFilter,
                  ImageToImageFilter )

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  // public typedefs
  typedef short         DiffusionPixelType;
  typedef Superclass::InputImageType         InputImageType;
  typedef Superclass::OutputImageType        OutputImageType;
  typedef itk::VectorImage<DiffusionPixelType, 3> ITKDiffusionImageType;
  typedef itk::Image<DiffusionPixelType, 3> ITKDiffusionVolumeType;

  mitk::Image::Pointer GetCorrectedImage() const;
  void UpdateOutputInformation() override;

protected:
  DWIHeadMotionCorrectionFilter();
  ~DWIHeadMotionCorrectionFilter() override {}

  mitk::Image::Pointer m_CorrectedImage;

  void GenerateData() override;

};

} //end namespace mitk


#endif // MITKDWIHEADMOTIONCORRECTIONFILTER_H
