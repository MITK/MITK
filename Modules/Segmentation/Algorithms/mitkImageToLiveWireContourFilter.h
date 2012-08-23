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
  class Segmentation_EXPORT mitkImageToLiveWireContourFilter : public ImageToContourModelFilter
  {

  public:

    mitkClassMacro(mitkImageToLiveWireContourFilter, ImageToContourModelFilter);
    itkNewMacro(Self);


  protected:
    mitkImageToLiveWireContourFilter();

    virtual ~mitkImageToLiveWireContourFilter();

    void GenerateData();

    mitk::Point3D m_StartPoint;
    mitk::Point3D m_Endpoint;

  };

}

#endif