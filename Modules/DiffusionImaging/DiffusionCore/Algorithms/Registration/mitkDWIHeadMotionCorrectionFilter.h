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

#include <DiffusionCoreExports.h>

#include "mitkDiffusionImageToDiffusionImageFilter.h"

namespace mitk
{

/**
 * @class DWIHeadMotionCorrectionFilter
 *
 * @brief Performs standard head-motion correction by using affine registration of the gradient images.
 *
 * (Head) motion correction is a essential pre-processing step before performing any further analysis of a diffusion-weighted
 * images since all model fits ( tensor, QBI ) rely on an aligned diffusion-weighted dataset. The correction is done in two steps. First the
 * unweighted images ( if multiple present ) are separately registered on the first one by means of rigid registration and normalized correlation
 * as error metric. Second, the weighted gradient images are registered to the unweighted reference ( computed as average from the aligned images from first step )
 * by an affine transformation using the MattesMutualInformation metric as optimizer guidance.
 *
 */
template< typename DiffusionPixelType>
class DiffusionCore_EXPORT DWIHeadMotionCorrectionFilter
    : public DiffusionImageToDiffusionImageFilter< DiffusionPixelType >
{
public:

  // class macros
  mitkClassMacro( DWIHeadMotionCorrectionFilter,
                  DiffusionImageToDiffusionImageFilter<DiffusionPixelType> )

  itkNewMacro(Self)

  // public typedefs
  typedef typename Superclass::InputImageType         DiffusionImageType;
  typedef typename Superclass::InputImagePointerType  DiffusionImagePointerType;

  typedef typename Superclass::OutputImageType        OutputImageType;
  typedef typename Superclass::OutputImagePointerType OutputImagePointerType;

protected:
  DWIHeadMotionCorrectionFilter();
  virtual ~DWIHeadMotionCorrectionFilter() {}

  virtual void GenerateData();

};

} //end namespace mitk

#include "mitkDWIHeadMotionCorrectionFilter.cpp"


#endif // MITKDWIHEADMOTIONCORRECTIONFILTER_H
