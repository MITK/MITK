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

#ifndef _mitkContourModelSubDivisionFilter_h__
#define _mitkContourModelSubDivisionFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"


namespace mitk {

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
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class Segmentation_EXPORT ContourModelSubDivisionFilter : public ContourModelSource
  {

  public:

    mitkClassMacro(ContourModelSubDivisionFilter, ContourModelSource);
    itkNewMacro(Self);


    typedef ContourModel OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::ContourModel InputType;

    /**
    * \brief Set the number of iterations for inserting new interpolated control points.
    *
    */
    void SetNumberOfIterations( int iterations)
    {
      this->m_InterpolationIterations = iterations;
    }

    virtual void SetInput( const InputType *input);

    virtual void SetInput( unsigned int idx, const InputType * input);

    const InputType* GetInput(void);

    const InputType* GetInput(unsigned int idx);

  protected:
    ContourModelSubDivisionFilter();

    virtual ~ContourModelSubDivisionFilter();

    void GenerateOutputInformation() {};

    void GenerateData();

    int m_InterpolationIterations;
  };

}

#endif
