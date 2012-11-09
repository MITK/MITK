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

#ifndef _mitkContourSetToPointSetFilter_h__
#define _mitkContourSetToPointSetFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include "mitkContourSet.h"

namespace mitk {

/**
 *
 * @brief Converts a contour set to a point set.
 *
 * The resulting pointset consists of sample points of all the contours
 *
 * @ingroup SurfaceFilters
 * @ingroup Process
 */
class Segmentation_EXPORT ContourSetToPointSetFilter : public PointSetSource
{
public:
  mitkClassMacro(ContourSetToPointSetFilter, PointSetSource);
  itkNewMacro(Self);


  itkSetMacro(Frequency, unsigned int);

  itkGetMacro(Frequency, unsigned int);

  virtual void GenerateOutputInformation();

  virtual void GenerateData();

  const mitk::ContourSet* GetInput(void);

  virtual void SetInput(const mitk::ContourSet *contourSet);

protected:
  ContourSetToPointSetFilter();

  virtual ~ContourSetToPointSetFilter();

protected:
  unsigned int m_Frequency;

};

} // namespace mitk

#endif // _mitkContourSetToPointSetFilter_h__
