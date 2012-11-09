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
#ifndef MITKBOUNDINGOBJECTTOSEGMENTATIONFILTER_H
#define MITKBOUNDINGOBJECTTOSEGMENTATIONFILTER_H

#include <mitkImageToImageFilter.h>
#include <mitkBoundingObjectGroup.h>

namespace mitk{
  class MitkExt_EXPORT BoundingObjectToSegmentationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingObjectToSegmentationFilter, ImageToImageFilter);
    itkNewMacro(BoundingObjectToSegmentationFilter);

    void SetBoundingObject(mitk::BoundingObject::Pointer boundingObject);
  protected:
    BoundingObjectToSegmentationFilter();
    virtual ~BoundingObjectToSegmentationFilter();

    virtual void GenerateData();

    mitk::BoundingObjectGroup::Pointer m_boundingObjectGroup;

  };//class
}//namespace
#endif
