/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAndMaskGenerator_h
#define mitkAndMaskGenerator_h

#include <MitkImageStatisticsExports.h>
#include <mitkMaskGenerator.h>
#include <mitkLabel.h>

#include <optional>

namespace mitk
{
  /**
   * \brief Restricts the masks of a primary MaskGenerator to the region selected by a secondary MaskGenerator.
   *
   * For every mask of the primary generator, the result is a copy in which a voxel keeps its
   * label value if the secondary mask carries the selected label value at the same world
   * position, and is set to 0 (background) otherwise. This is a pixel-wise logical AND of
   * "primary voxel is labeled" and "secondary voxel has the selected label", with the label
   * values of the primary mask preserved so that per-label statistics stay possible.
   *
   * The result has the dimension and geometry of the primary mask. Both masks are matched by
   * world coordinates, so they may differ in extent, origin and dimension (for example a 2D
   * planar figure mask against a 3D mask); voxels outside the secondary mask count as not
   * selected. The voxel grids have to be aligned, i.e. same orientation, spacings that are
   * integer multiples of each other, and no sub-voxel offset. Otherwise GetMask() throws.
   * Masks of a pixel type other than unsigned short are converted; pixel types that cannot
   * be converted (e.g. RGB) are rejected with an exception.
   *
   * Primary generator, secondary generator and the selected label value have to be set. The
   * time point is forwarded to both generators on every GetMask() call, the input image is
   * not: configure the chained generators directly. GetNumberOfMasks() and
   * GetReferenceImage() are delegated to the primary generator. Results are not cached.
   *
   * Typical use is any region of interest generator as primary and an IgnorePixelMaskGenerator
   * with selected label value 1 as secondary, to exclude e.g. zero-valued voxels.
   *
   * \sa MaskGenerator
   * \sa IgnorePixelMaskGenerator
   * \sa ImageStatisticsCalculator
   */
  class MITKIMAGESTATISTICS_EXPORT AndMaskGenerator : public MaskGenerator
  {
  public:
    mitkClassMacro(AndMaskGenerator, MaskGenerator);
    itkNewMacro(Self);

    using LabelValueType = Label::PixelType;

    /** \brief Set the generator whose masks are restricted. Their label values are preserved. */
    itkSetObjectMacro(PrimaryMaskGenerator, MaskGenerator);

    /** \brief Set the generator whose mask selects the voxels to keep. */
    itkSetObjectMacro(SecondaryMaskGenerator, MaskGenerator);

    /** \brief Set which mask of the secondary generator is used for the selection. Default is 0. */
    itkSetMacro(SecondaryMaskID, unsigned int);

    /** \brief Set the label value of the secondary mask that selects voxels. Has to be set before GetMask(). */
    void SetSecondaryLabelValue(LabelValueType labelValue);

    /**
     * \brief Number of masks of the primary generator.
     * \pre The primary generator has to be set.
     */
    unsigned int GetNumberOfMasks() const override;

    /**
     * \brief Reference image of the primary generator.
     * \pre The primary generator has to be set.
     */
    Image::ConstPointer GetReferenceImage() override;

    /** \brief Latest modification time of this generator and the two chained generators. */
    itk::ModifiedTimeType GetMTime() const override;

  protected:
    AndMaskGenerator() = default;
    ~AndMaskGenerator() override = default;

    Image::ConstPointer DoGetMask(unsigned int maskID) override;

  private:
    MaskGenerator::Pointer m_PrimaryMaskGenerator;
    MaskGenerator::Pointer m_SecondaryMaskGenerator;
    unsigned int m_SecondaryMaskID = 0;
    std::optional<LabelValueType> m_SecondaryLabelValue;
  };
}

#endif
