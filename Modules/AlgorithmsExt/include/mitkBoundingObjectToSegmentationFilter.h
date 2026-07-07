/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkBoundingObjectToSegmentationFilter_h
#define mitkBoundingObjectToSegmentationFilter_h

#include <mitkBoundingObjectGroup.h>
#include <mitkImageToImageFilter.h>

#include <MitkAlgorithmsExtExports.h>

namespace mitk
{
  /**
   * \brief Converts a BoundingObject or BoundingObjectGroup into a binary segmentation image.
   *
   * This filter takes an input image and a BoundingObject, and produces an output image of the
   * same size where pixels inside the bounding object are set to 1 and pixels outside remain 0.
   * If a single BoundingObject is provided, it is wrapped into a BoundingObjectGroup internally.
   * If a BoundingObjectGroup is provided directly, all contained bounding objects are rasterized.
   * The positive/negative flag of each BoundingObject controls whether its interior is filled (1)
   * or cleared (0).
   *
   * \sa BoundingObject
   * \sa BoundingObjectGroup
   * \sa BoundingObjectCutter
   * \ingroup Process
   */
  class MITKALGORITHMSEXT_EXPORT BoundingObjectToSegmentationFilter : public ImageToImageFilter
  {
  public:
    mitkClassMacro(BoundingObjectToSegmentationFilter, ImageToImageFilter);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Set the bounding object to be rasterized into the segmentation.
       *
       * If a BoundingObjectGroup is passed, it is used directly. If a single
       * BoundingObject is passed, it is wrapped in a new BoundingObjectGroup.
       *
       * \param[in] boundingObject The bounding object (or group) to rasterize.
       */
      void SetBoundingObject(mitk::BoundingObject::Pointer boundingObject);

  protected:
    BoundingObjectToSegmentationFilter();
    ~BoundingObjectToSegmentationFilter() override;

    void GenerateData() override;

    mitk::BoundingObjectGroup::Pointer m_boundingObjectGroup;

  }; // class
} // namespace
#endif
