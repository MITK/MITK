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

#ifndef _mitkImageToLiveWireContourFilter_h__
#define _mitkImageToLiveWireContourFilter_h__

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkContourModel.h"
#include "mitkContourModelSource.h"

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>




namespace mitk {

  /**
  *
  * \brief
  *
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class MitkSegmentation_EXPORT ImageToLiveWireContourFilter : public ContourModelSource
  {

  public:

    mitkClassMacro(ImageToLiveWireContourFilter, ContourModelSource);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    typedef ContourModel OutputType;
    typedef OutputType::Pointer OutputTypePointer;
    typedef mitk::Image InputType;

    itkSetMacro(StartPoint, mitk::Point3D);
    itkGetMacro(StartPoint, mitk::Point3D);

    itkSetMacro(EndPoint, mitk::Point3D);
    itkGetMacro(EndPoint, mitk::Point3D);


    virtual void SetInput( const InputType *input);

    virtual void SetInput( unsigned int idx, const InputType * input);

    const InputType* GetInput(void);

    const InputType* GetInput(unsigned int idx);

  protected:
    ImageToLiveWireContourFilter();

    virtual ~ImageToLiveWireContourFilter();

    void GenerateData();

    void GenerateOutputInformation() {};

    mitk::Point3D m_StartPoint;
    mitk::Point3D m_EndPoint;

    mitk::Point3D m_StartPointInIndex;
    mitk::Point3D m_EndPointInIndex;

  private:

    template<typename TPixel, unsigned int VImageDimension>
    void ItkProcessImage (const itk::Image<TPixel, VImageDimension>* inputImage);


  };

}
#endif
