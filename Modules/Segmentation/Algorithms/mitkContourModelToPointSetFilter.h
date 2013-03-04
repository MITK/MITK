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

#ifndef _mitkContourModelToPointSetFilter_h__
#define _mitkContourModelToPointSetFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include "mitkContourModel.h"

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
  class Segmentation_EXPORT ContourModelToPointSetFilter : public PointSetSource
  {

  public:

    mitkClassMacro(ContourModelToPointSetFilter, PointSetSource);
    itkNewMacro(Self);

    typedef PointSet OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::ContourModel InputType;

    virtual void GraftOutput(OutputType *output);
    virtual void GraftNthOutput(unsigned int idx, OutputType *output);

    virtual void SetInput( const InputType *input);
    virtual void SetInput(DataObjectPointerArraySizeType idx, const InputType * input);
    const InputType * GetInput(void);
    const InputType * GetInput(unsigned int idx);

  protected:
    ContourModelToPointSetFilter();

    virtual ~ContourModelToPointSetFilter();

    void GenerateOutputInformation() {};

    void GenerateData();
  };

}

#endif
