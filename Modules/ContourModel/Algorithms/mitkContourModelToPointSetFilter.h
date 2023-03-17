/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelToPointSetFilter_h
#define mitkContourModelToPointSetFilter_h

#include "mitkCommon.h"
#include "mitkContourModel.h"
#include "mitkPointSet.h"
#include "mitkPointSetSource.h"
#include <MitkContourModelExports.h>

namespace mitk
{
  /**
  *
  * @brief Converts a contour set to a point set.
  *
  * The resulting pointset consists of sample points of all the contours
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(ContourModelToPointSetFilter, PointSetSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef PointSet OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::ContourModel InputType;

  protected:
    ContourModelToPointSetFilter();

    ~ContourModelToPointSetFilter() override;

    void GenerateOutputInformation() override {}
    void GenerateData() override;
  };
}

#endif
