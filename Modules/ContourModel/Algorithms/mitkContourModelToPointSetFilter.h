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
#include <MitkContourModelExports.h>
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
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelToPointSetFilter : public PointSetSource
  {

  public:

    mitkClassMacro(ContourModelToPointSetFilter, PointSetSource)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef PointSet OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::ContourModel InputType;

  protected:

    ContourModelToPointSetFilter();

    virtual ~ContourModelToPointSetFilter();

    void GenerateOutputInformation() override {}

    void GenerateData() override;
  };

}

#endif
