/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegmentationInterpolationController_h
#define mitkSegmentationInterpolationController_h

#include <mitkCommon.h>
#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkPlaneGeometry.h>
#include <MitkSegmentationExports.h>
#include <mitkShapeBasedInterpolationAlgorithm.h>

#include <itkObjectFactory.h>

#include <array>
#include <cstddef>
#include <functional>
#include <optional>
#include <vector>

namespace mitk
{
  /**
    \brief Generates interpolations of 2D slices.

    \sa QmitkSlicesInterpolator

    \ingroup ToolManagerEtAl

    Interpolates one label of a segmentation in slices that do not contain it, from the nearest slices below and
    above that do. To find those slices, the controller counts the voxels of the label in every slice along each
    image axis. The counts are recomputed on demand when the modification time of the segmentation changed, so the
    controller needs no notification about edits, but the first interpolation after an edit scans the whole time
    step.
  */
  class MITKSEGMENTATION_EXPORT SegmentationInterpolationController : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SegmentationInterpolationController, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
      \brief Sets the segmentation to interpolate in and the pixel value of the label to interpolate.

      The segmentation must be a 3D or 3D+t image with the pixel type of MultiLabelSegmentation group images. All
      other pixel values count as background. Setting the current segmentation and label again keeps everything
      computed so far. Pass nullptr to release the segmentation.

      \throw mitk::Exception if the segmentation is neither 3D nor 3D+t or has another pixel type.
    */
    void SetSegmentationVolume(const Image *segmentation, Label::PixelType labelValue);

    /**
      \brief Interpolates the label in a slice from the nearest slices below and above that contain it.

      \param sliceDimension Number of the dimension which is constant for all pixels of the meant slice.
      \param sliceIndex Which slice to take, in the direction specified by sliceDimension. Count starts from 0.
      \param currentPlane The plane of the slice. The result is sampled on it.
      \param timeStep Which time step to use.
      \return A 2D image with 1 inside the interpolated shape and 0 elsewhere, or nullptr if there is nothing to
              interpolate, i.e. the slice contains the label or no slice on one of its sides does.

      \throw SegmentationInterpolationException if the enclosing slices cannot be combined.
    */
    Image::Pointer Interpolate(unsigned int sliceDimension,
                               unsigned int sliceIndex,
                               const PlaneGeometry *currentPlane,
                               unsigned int timeStep);

    /**
      \brief Interpolates every slice along an image axis that has something to interpolate.

      Gives the results Interpolate() gives for each slice, computed gap by gap from the segmentation as it is when the
      call starts. The consumer may therefore write each result into its slice of the segmentation, for instance to
      accept all interpolations.

      \param sliceDimension Number of the dimension which is constant for all pixels of the meant slices.
      \param plane The plane of any slice along that axis. Each result is sampled on it, moved to its slice.
      \param timeStep Which time step to use.
      \param consumer Called in ascending slice order with the index of a slice and its interpolation.

      \throw SegmentationInterpolationException if the enclosing slices of a gap cannot be combined.
    */
    void InterpolateAll(unsigned int sliceDimension,
                        const PlaneGeometry *plane,
                        unsigned int timeStep,
                        const std::function<void(unsigned int sliceIndex, const Image *interpolation)> &consumer);

  protected:
    SegmentationInterpolationController();
    ~SegmentationInterpolationController() override;

  private:
    /** Number of label voxels in each slice, per image axis. */
    using SliceCountsType = std::array<std::vector<std::size_t>, 3>;

    struct TimeStepSliceCounts
    {
      std::optional<itk::ModifiedTimeType> SegmentationMTime;
      SliceCountsType Counts;
    };

    /**
      The two slices enclosing a gap, cropped to the bounding box of the label in both and binarized. Algorithm caches
      their distance maps for all slices of the gap.
    */
    struct EnclosingSlices
    {
      unsigned int TimeStep = 0;
      unsigned int SliceDimension = 0;
      unsigned int LowerIndex = 0;
      unsigned int UpperIndex = 0;
      itk::ModifiedTimeType SegmentationMTime = 0;
      PlaneGeometry::ConstPointer LowerPlane;

      std::array<std::size_t, 2> SliceSize = {};
      std::array<std::size_t, 2> CropBegin = {};
      std::array<std::size_t, 2> CropSize = {};

      /** Both null if neither slice contains the label on the plane. */
      Image::Pointer LowerCrop;
      Image::Pointer UpperCrop;

      ShapeBasedInterpolationAlgorithm::Pointer Algorithm;
    };

    const SliceCountsType &GetSliceCounts(unsigned int timeStep);

    EnclosingSlices CreateEnclosingSlices(unsigned int sliceDimension,
                                          unsigned int lowerIndex,
                                          unsigned int upperIndex,
                                          const PlaneGeometry *plane,
                                          unsigned int timeStep) const;

    /** Like CreateEnclosingSlices(), but reuses the enclosing slices of the previous call while they are still valid. */
    const EnclosingSlices &GetEnclosingSlices(unsigned int sliceDimension,
                                              unsigned int lowerIndex,
                                              unsigned int upperIndex,
                                              const PlaneGeometry *currentPlane,
                                              unsigned int timeStep);

    Image::Pointer InterpolateBetween(const EnclosingSlices &enclosingSlices,
                                      unsigned int sliceIndex,
                                      const PlaneGeometry *slicePlane,
                                      unsigned int timeStep) const;

    Image::ConstPointer m_Segmentation;
    Label::PixelType m_LabelValue;
    std::vector<TimeStepSliceCounts> m_SliceCounts;

    /** The enclosing slices of the gap Interpolate() was last called for. */
    std::optional<EnclosingSlices> m_EnclosingSlices;
  };

} // namespace

#endif
