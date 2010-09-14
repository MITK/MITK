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