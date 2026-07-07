/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkContourModelToPointSetFilter_h
#define mitkContourModelToPointSetFilter_h

#include <mitkCommon.h>
#include <mitkContourModel.h>
#include <mitkPointSet.h>
#include <mitkPointSetSource.h>
#include <MitkContourModelExports.h>

namespace mitk
{
  /** \brief Filter that converts a mitk::ContourModel into a mitk::PointSet.
   *
   * Each vertex of the input contour model is added as a point to the output
   * PointSet. All time steps of the input are processed.
   *
   * \sa ContourModel, PointSet, PointSetSource
   * \ingroup MitkContourModelModule
   */
  class MITKCONTOURMODEL_EXPORT ContourModelToPointSetFilter : public PointSetSource
  {
  public:
    mitkClassMacro(ContourModelToPointSetFilter, PointSetSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** \brief The output data type. */
      typedef PointSet OutputType;
    /** \brief Smart pointer type for the output. */
    typedef OutputType::Pointer OutputTypePointer;
    /** \brief The input data type. */
    typedef mitk::ContourModel InputType;

  protected:
    ContourModelToPointSetFilter();

    ~ContourModelToPointSetFilter() override;

    void GenerateOutputInformation() override {}
    void GenerateData() override;
  };
}

#endif
