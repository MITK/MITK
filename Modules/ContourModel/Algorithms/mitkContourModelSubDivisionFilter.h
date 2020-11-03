/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitkContourModelSubDivisionFilter_h__
#define _mitkContourModelSubDivisionFilter_h__

#include "mitkCommon.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"
#include <MitkContourModelExports.h>

namespace mitk
{
  /**
  *
  * \brief This filter interpolates a subdivision curve between control points of the contour.
  * For inserting subpoints Dyn-Levin-Gregory (DLG) interpolation scheme is used.
  * Interpolating a cruve subdivision is done by:
  * F2i = Ci
  * F2i+1 = -1/16Ci-1 + 9/16Ci + 9/16Ci+1 - 1/16Ci+2
  *
  * The number of interpolation iterations can be set via SetNumberOfIterations(int) which are 4 by dafault.
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ContourModelSubDivisionFilter : public ContourModelSource
  {
  public:
    mitkClassMacro(ContourModelSubDivisionFilter, ContourModelSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef ContourModel OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::ContourModel InputType;

    /**
    * \brief Set the number of iterations for inserting new interpolated control points.
    *
    */
    void SetNumberOfIterations(int iterations) { this->m_InterpolationIterations = iterations; }
    using Superclass::SetInput;

    virtual void SetInput(const InputType *input);

    virtual void SetInput(unsigned int idx, const InputType *input);

    const InputType *GetInput(void);

    const InputType *GetInput(unsigned int idx);

  protected:
    ContourModelSubDivisionFilter();

    ~ContourModelSubDivisionFilter() override;

    void GenerateOutputInformation() override{};

    void GenerateData() override;

    int m_InterpolationIterations;
  };
}

#endif
