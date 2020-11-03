/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef _mitkImageToContourModelFilter_h__
#define _mitkImageToContourModelFilter_h__

#include "mitkCommon.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"
#include <MitkContourModelExports.h>
#include <mitkImage.h>

namespace mitk
{
  /**
  *
  * \brief Base class for all filters with mitk::Image as input and mitk::ContourModel
  *
  * @ingroup MitkContourModelModule
  */
  class MITKCONTOURMODEL_EXPORT ImageToContourModelFilter : public ContourModelSource
  {
  public:
    mitkClassMacro(ImageToContourModelFilter, ContourModelSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef mitk::Image InputType;

    using Superclass::SetInput;

    virtual void SetInput(const InputType *input);

    virtual void SetInput(unsigned int idx, const InputType *input);

    const InputType *GetInput(void);

    const InputType *GetInput(unsigned int idx);

    void SetContourValue(float contourValue);

    float GetContourValue();

  protected:
    ImageToContourModelFilter();

    ~ImageToContourModelFilter() override;

    void GenerateData() override;

    template <typename TPixel, unsigned int VImageDimension>
    void Itk2DContourExtraction(const itk::Image<TPixel, VImageDimension> *sliceImage);

  private:
    const BaseGeometry *m_SliceGeometry;
    float m_ContourValue;
  };
}

#endif
