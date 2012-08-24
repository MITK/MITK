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

#ifndef _mitkImageToContourModelFilter_h__
#define _mitkImageToContourModelFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"
#include <mitkImage.h>


namespace mitk {

  /**
  *
  * \brief Base class for all filters with mitk::Image as input and mitk::ContourModel
  *
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class Segmentation_EXPORT ImageToContourModelFilter : public ContourModelSource
  {

  public:

    mitkClassMacro(ImageToContourModelFilter, ContourModelSource);
    itkNewMacro(Self);

    typedef mitk::Image InputType;


    virtual void SetInput( const InputType *input);

    virtual void SetInput( unsigned int idx, const InputType * input);

    const InputType* GetInput(void);

    const InputType* GetInput(unsigned int idx);

  protected:
    ImageToContourModelFilter();

    virtual ~ImageToContourModelFilter();

    void GenerateData();

  };

}

#endif