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

#ifndef _MITK_CONTOURMODEL_TO_Surface_FILTER_H_
#define _MITK_CONTOURMODEL_TO_Surface_FILTER_H_


#include "mitkCommon.h"
#include "SegmentationExports.h"


#include "mitkContourModel.h"
#include "mitkContourModelSource.h"

#include <mitkSurfaceSource.h>


namespace mitk {

  class Segmentation_EXPORT ContourModelToSurfaceFilter : public SurfaceSource
  {

  public:
    /** Standard class typedefs. */
    mitkClassMacro( ContourModelToSurfaceFilter,BaseProcess );

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    typedef mitk::Surface OutputType;

    typedef mitk::ContourModel InputType;



    /** Set/Get the image input of this process object.  */
    virtual void SetInput( const InputType *input);
    virtual void SetInput( unsigned int idx, const InputType * input);
    const InputType * GetInput(void);
    const InputType * GetInput(unsigned int idx);

  protected:
    ContourModelToSurfaceFilter();
    ~ContourModelToSurfaceFilter();

    virtual void GenerateData();

  };
}
#endif
