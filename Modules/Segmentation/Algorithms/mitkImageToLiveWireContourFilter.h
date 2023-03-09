/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkImageToLiveWireContourFilter_h
#define mitkImageToLiveWireContourFilter_h

#include "mitkCommon.h"
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"
#include <MitkSegmentationExports.h>

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

namespace mitk
{
  /**
  *
  * \brief
  *
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class MITKSEGMENTATION_EXPORT ImageToLiveWireContourFilter : public ContourModelSource
  {
  public:
    mitkClassMacro(ImageToLiveWireContourFilter, ContourModelSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      typedef ContourModel OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::Image InputType;

    itkSetMacro(StartPoint, mitk::Point3D);
    itkGetMacro(StartPoint, mitk::Point3D);

    itkSetMacro(EndPoint, mitk::Point3D);
    itkGetMacro(EndPoint, mitk::Point3D);

    virtual void SetInput(const InputType *input);

    using Superclass::SetInput;
    virtual void SetInput(unsigned int idx, const InputType *input);

    const InputType *GetInput(void);

    const InputType *GetInput(unsigned int idx);

  protected:
    ImageToLiveWireContourFilter();

    ~ImageToLiveWireContourFilter() override;

    void GenerateData() override;

    void GenerateOutputInformation() override{};

    mitk::Point3D m_StartPoint;
    mitk::Point3D m_EndPoint;

    mitk::Point3D m_StartPointInIndex;
    mitk::Point3D m_EndPointInIndex;

  private:
    template <typename TPixel, unsigned int VImageDimension>
    void ItkProcessImage(const itk::Image<TPixel, VImageDimension> *inputImage);
  };
}
#endif
