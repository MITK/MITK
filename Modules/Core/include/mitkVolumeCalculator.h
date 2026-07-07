/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkVolumeCalculator_h
#define mitkVolumeCalculator_h

#include <itkImage.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

#include <mitkImage.h>
#include <mitkImageTimeSelector.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Calculates the volume of a mitk::Image.
   *
   * The volume is given in milliliters (cubic cm for 3D, square cm for 2D)
   * or as a voxel count. Voxels are counted if their gray value is equal to
   * or greater than a threshold (see SetThreshold). The default threshold is 0.
   *
   * The filter works for 2D, 3D, and 3D+t images. In the 3D+t case, a vector
   * of volumes is provided (see GetVolumes()).
   */
  class MITKCORE_EXPORT VolumeCalculator : public itk::Object
  {
  public:
    mitkClassMacroItkParent(VolumeCalculator, itk::Object);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

    itkSetObjectMacro(Image, Image);

    /**
     * \brief Sets threshold; all voxels with a value equal to or greater than this are accepted.
     */
    itkSetMacro(Threshold, int);

    /**
     * \brief Return the calculated volume.
     *
     * For 2D images the result is in square cm, for 3D images in cubic cm.
     * For 4D data, use GetVolumes() instead.
     *
     * \sa GetVolumes()
     */
    itkGetMacro(Volume, float);

    /**
     * \brief Return the number of accepted voxels (those at or above the threshold).
     */
    itkGetMacro(VoxelCount, unsigned long int);

    /**
     * \brief Return the volumes for 4D images.
     *
     * Each entry in the returned vector represents the volume for one time step
     * in cubic cm.
     *
     * \note For 2D+t images the result is also returned in cubic cm.
     *
     * \return A vector of volumes, one per time step.
     */
    std::vector<float> GetVolumes();

    /**
     * \brief Compute the volume by iterating over all voxels above the threshold.
     *
     * For 4D images, the per-time-step volumes are stored and can be retrieved
     * via GetVolumes().
     */
    void ComputeVolume();

    /**
     * \brief Compute the volume using image statistics (count of max-valued voxels).
     *
     * This is a faster alternative to ComputeVolume() when only the maximum-valued
     * voxels are of interest.
     */
    void ComputeVolumeFromImageStatistics();

    /**
     * \brief Compute a volume from spacing and voxel count.
     *
     * \param[in] spacing The voxel spacing in each dimension.
     * \param[in] voxelCount The number of voxels.
     * \return The computed volume in cubic cm.
     */
    static float ComputeVolume(Vector3D spacing, unsigned int voxelCount);

  protected:
    VolumeCalculator();
    ~VolumeCalculator() override;

    template <typename TPixel, unsigned int VImageDimension>
    void InternalCompute(const itk::Image<TPixel, VImageDimension> *itkImage);

    Image::ConstPointer m_Image;
    int m_Threshold;
    float m_Volume;
    unsigned long int m_VoxelCount;
    std::vector<float> m_Volumes;
    ImageTimeSelector::Pointer m_TimeSelector;
  };

} // namespace mitk

#endif
