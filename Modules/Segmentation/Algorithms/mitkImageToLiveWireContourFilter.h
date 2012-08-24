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

#ifndef _mitkmitkImageToLiveWireContourFilter_h__
#define _mitkmitkImageToLiveWireContourFilter_h__

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkContourModel.h"
#include "mitkImageToContourModelFilter.h"



namespace mitk {

  /**
  *
  * \brief 
  *
  * \ingroup ContourModelFilters
  * \ingroup Process
  */
  class Segmentation_EXPORT ImageToLiveWireContourFilter : public ImageToContourModelFilter
  {

  public:

    mitkClassMacro(ImageToLiveWireContourFilter, ImageToContourModelFilter);
    itkNewMacro(Self);

    itkSetMacro(StartPoint, mitk::Point3D);
    itkGetMacro(StartPoint, mitk::Point3D);

    itkSetMacro(EndPoint, mitk::Point3D);
    itkGetMacro(EndPoint, mitk::Point3D);


  protected:
    ImageToLiveWireContourFilter();

    virtual ~ImageToLiveWireContourFilter();

    void GenerateData();

    template<typename TPixel, unsigned int VImageDimension>
    void ItkProcessImage (itk::Image<TPixel, VImageDimension>* inputImage);

    mitk::Point3D m_StartPoint;
    mitk::Point3D m_EndPoint;

    ImageToContourModelFilter::IndexType m_StartPointInIndex;
    ImageToContourModelFilter::IndexType m_EndPointInIndex;

  };

}

#endif