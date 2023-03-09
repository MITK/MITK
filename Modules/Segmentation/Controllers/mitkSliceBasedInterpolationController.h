/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSliceBasedInterpolationController_h
#define mitkSliceBasedInterpolationController_h

#include "mitkLabelSetImage.h"
#include <MitkSegmentationExports.h>

#include <itkImage.h>
#include <itkObjectFactory.h>

#include <map>
#include <vector>

namespace mitk
{
  class Image;

  /**
    \brief Generates interpolations of 2D slices.

    \sa QmitkSlicesInterpolator
    \sa QmitkInteractiveSegmentation

    \ingroup ToolManagerEtAl

    This class keeps track of the contents of a 3D segmentation image.
    \attention mitk::SliceBasedInterpolationController assumes that the image contains pixel values of 0 and 1.

    After you set the segmentation image using SetSegmentationVolume(), the whole image is scanned for pixels other than
    0.
    SliceBasedInterpolationController registers as an observer to the segmentation image, and repeats the scan whenvever
    the
    image is modified.

    You can prevent this (time consuming) scan if you do the changes slice-wise and send difference images to
    SliceBasedInterpolationController.
    For this purpose SetChangedSlice() should be used. mitk::OverwriteImageFilter already does this every time it
    changes a
    slice of an image. There is a static method InterpolatorForImage(), which can be used to find out if there already
    is an interpolator
    instance for a specified image. OverwriteImageFilter uses this to get to know its interpolator.

    SliceBasedInterpolationController needs to maintain some information about the image slices (in every dimension).
    This information is stored internally in m_SegmentationCountInSlice, which is basically three std::vectors (one for
    each dimension).
    Each item describes one image dimension, each vector item holds the count of pixels in "its" slice. This is perhaps
    better to understand
    from the following picture (where red items just mean to symbolize "there is some segmentation" - in reality there
    is an integer count).

    $Author$
  */
  class MITKSEGMENTATION_EXPORT SliceBasedInterpolationController : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SliceBasedInterpolationController, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
        \brief Find interpolator for a given image.
        \return nullptr if there is no interpolator yet.

        This method is useful if several "clients" modify the same image and want to access the interpolations.
        Then they can share the same object.
       */
      static SliceBasedInterpolationController *InterpolatorForImage(const Image *);

    /**
      \brief Initialize with a whole volume.

      Will scan the volume for segmentation pixels (values other than 0) and fill some internal data structures.
      You don't have to call this method every time something changes, but only
      when several slices at once change.

      When you change a single slice, call SetChangedSlice() instead.
    */
    void SetWorkingImage(LabelSetImage *image);

    /**
      \brief Set a reference image (original patient image) - optional.

      If this image is set (must exactly match the dimensions of the segmentation),
      the interpolation algorithm may consider image content to improve the interpolated
      (estimated) segmentation.
     */
    void SetReferenceImage(Image *image);

    /**
      \brief Update after changing a single slice in the working image.

      \param image is a 2D image with the difference image of the slice determined by sliceDimension and sliceIndex.
             The difference is (pixel value in the new slice minus pixel value in the old slice).

      \param sliceDimension Number of the dimension which is constant for all pixels of the meant slice.

      \param sliceIndex Which slice to take, in the direction specified by sliceDimension. Count starts from 0.

      \param timeStep Which time step is changed
    */
    void SetChangedSlice(const Image *image,
                         unsigned int sliceDimension,
                         unsigned int sliceIndex,
                         unsigned int timeStep);

    /**
      \brief Generates an interpolated image for the given slice.

      \param sliceDimension Number of the dimension which is constant for all pixels of the meant slice.

      \param sliceIndex Which slice to take, in the direction specified by sliceDimension. Count starts from 0.

      \param currentPlane

      \param timeStep Which time step to use
    */
    Image::Pointer Interpolate(unsigned int sliceDimension,
                               unsigned int sliceIndex,
                               const mitk::PlaneGeometry *currentPlane,
                               unsigned int timeStep);

    /**
      \brief Initializes the internal container with the number of voxels per label.
    */
    void ResetLabelCount();

  protected:
    /**
      \brief Protected class of mitk::SliceBasedInterpolationController. Don't use (you shouldn't be able to do so)!
    */
    class MITKSEGMENTATION_EXPORT SetChangedSliceOptions
    {
    public:
      SetChangedSliceOptions(unsigned int sd, unsigned int si, unsigned int d0, unsigned int d1, unsigned int t)
        : sliceDimension(sd), sliceIndex(si), dim0(d0), dim1(d1), timeStep(t)
      {
      }

      unsigned int sliceDimension;
      unsigned int sliceIndex;
      unsigned int dim0;
      unsigned int dim1;
      unsigned int timeStep;
      //        void* pixelData;
    };

    typedef std::vector<unsigned int> LabelCounterVectorType;
    typedef std::vector<LabelCounterVectorType> LabelCounterSliceVectorType;
    typedef std::vector<std::vector<LabelCounterSliceVectorType>> LabelCounterSliceTimeVectorType;
    typedef std::map<const Image *, SliceBasedInterpolationController *> InterpolatorMapType;

    SliceBasedInterpolationController(); // purposely hidden
    ~SliceBasedInterpolationController() override;

    /// internal scan of a single slice
    template <typename PixelType>
    void ScanSliceITKProcessing(const itk::Image<PixelType, 2> *, const SetChangedSliceOptions &options);

    /// internal scan of the whole image
    template <typename TPixel, unsigned int VImageDimension>
    void ScanImageITKProcessing(itk::Image<TPixel, VImageDimension> *, unsigned int timeStep);

    /**
      An array that of flags. One for each dimension of the image. A flag is set, when a slice in a certain dimension
      has at least one pixel that is not 0 (which would mean that it has to be considered by the interpolation
      algorithm).
      E.g. flags for axial slices are stored in m_SegmentationCountInSlice[0][index].
      Enhanced with time steps it is now m_SegmentationCountInSlice[timeStep][0][index]
    */
    LabelCounterSliceTimeVectorType m_LabelCountInSlice;

    static InterpolatorMapType s_InterpolatorForImage;

    LabelSetImage::Pointer m_WorkingImage;
    Image::Pointer m_ReferenceImage;
  };
} // namespace

#endif
