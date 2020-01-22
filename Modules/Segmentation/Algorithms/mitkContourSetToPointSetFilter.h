/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitkContourSetToPointSetFilter_h__
#define _mitkContourSetToPointSetFilter_h__

#include "mitkCommon.h"
#include "mitkContourSet.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include <MitkSegmentationExports.h>

namespace mitk
{
  /**
   *
   * @brief Converts a contour set to a point set.
   *
   * The resulting pointset consists of sample points of all the contours
   *
   * @ingroup SurfaceFilters
   * @ingroup Process
   */
  class MITKSEGMENTATION_EXPORT ContourSetToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(ContourSetToPointSetFilter, PointSetSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkSetMacro(Frequency, unsigned int);

    itkGetMacro(Frequency, unsigned int);

    void GenerateOutputInformation() override;

    void GenerateData() override;

    const mitk::ContourSet *GetInput(void);

    using Superclass::SetInput;
    virtual void SetInput(const mitk::ContourSet *contourSet);

  protected:
    ContourSetToPointSetFilter();

    ~ContourSetToPointSetFilter() override;

  protected:
    unsigned int m_Frequency;
  };

} // namespace mitk

#endif // _mitkContourSetToPointSetFilter_h__
