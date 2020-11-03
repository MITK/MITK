/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKBOUNDINGOBJECTTOSEGMENTATIONFILTER_H
#define MITKBOUNDINGOBJECTTOSEGMENTATIONFILTER_H

#include <mitkBoundingObjectGroup.h>
#include <mitkImageToImageFilter.h>

#include <MitkAlgorithmsExtExports.h>

namespace mitk
{
  class MITKALGORITHMSEXT_EXPORT BoundingObjectToSegmentationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingObjectToSegmentationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void SetBoundingObject(mitk::BoundingObject::Pointer boundingObject);

  protected:
    BoundingObjectToSegmentationFilter();
    ~BoundingObjectToSegmentationFilter() override;

    void GenerateData() override;

    mitk::BoundingObjectGroup::Pointer m_boundingObjectGroup;

  }; // class
} // namespace
#endif
