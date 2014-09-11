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
#include <MitkContourModelExports.h>
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
  class MitkContourModel_EXPORT ImageToContourModelFilter : public ContourModelSource
  {

  public:

    mitkClassMacro(ImageToContourModelFilter, ContourModelSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef mitk::Image InputType;


    using Superclass::SetInput;

    virtual void SetInput( const InputType *input);

    virtual void SetInput( unsigned int idx, const InputType * input);

    const InputType* GetInput(void);

    const InputType* GetInput(unsigned int idx);

  protected:
    ImageToContourModelFilter();

    virtual ~ImageToContourModelFilter();

    void GenerateData();

    template<typename TPixel, unsigned int VImageDimension>
    void Itk2DContourExtraction (const itk::Image<TPixel, VImageDimension>* sliceImage);

  private:
    const BaseGeometry* m_SliceGeometry;

  };

}

#endif
