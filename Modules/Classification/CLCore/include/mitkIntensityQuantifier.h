/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkIntensityQuantifier_h
#define mitkIntensityQuantifier_h

#include <MitkCLCoreExports.h>

#include <mitkBaseData.h>
#include <mitkImage.h>

namespace mitk
{
/**
 * \brief Maps continuous image intensity values to discrete histogram bin indices.
 *
 * IntensityQuantifier provides various initialization strategies for setting up
 * an intensity-to-bin-index mapping. Initialization can be done by explicitly
 * providing minimum, maximum, bin count, and/or bin size, or by computing these
 * from an image (optionally restricted to a masked region).
 *
 * After initialization, IntensityToIndex() maps a continuous intensity value to
 * a zero-based bin index, and the IndexTo*Intensity() methods perform the reverse
 * mapping.
 *
 * \sa AbstractGlobalImageFeature
 */
class MITKCLCORE_EXPORT IntensityQuantifier : public BaseData
{
public:
  mitkClassMacro(IntensityQuantifier, BaseData);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  IntensityQuantifier();

  /**
   * \brief Initialize with explicit minimum, maximum, and number of bins.
   *
   * The bin size is computed as (maximum - minimum) / bins.
   *
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] maximum The upper bound of the intensity range.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByMinimumMaximum(double minimum, double maximum, unsigned int bins);

  /**
   * \brief Initialize with explicit minimum, number of bins, and bin size.
   *
   * Maximum is computed as minimum + bins * binsize.
   *
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] bins Number of histogram bins.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByBinsizeAndBins(double minimum, unsigned int bins, double binsize);

  /**
   * \brief Initialize with explicit minimum, maximum, and bin size.
   *
   * The number of bins is computed as ceil((maximum - minimum) / binsize).
   * The actual maximum may be adjusted to be an exact multiple of binsize.
   *
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] maximum The upper bound of the intensity range.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByBinsizeAndMaximum(double minimum, double maximum, double binsize);

  /**
   * \brief Initialize from the full intensity range of an image.
   *
   * \param[in] image The image from which to determine minimum and maximum intensities.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByImage(const Image* image, unsigned int bins);

  /**
   * \brief Initialize from an image with a specified maximum; minimum is computed from the image.
   *
   * \param[in] image The image from which to determine the minimum intensity.
   * \param[in] maximum The upper bound of the intensity range.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByImageAndMaximum(const Image* image, double maximum, unsigned int bins);

  /**
   * \brief Initialize from an image with a specified minimum; maximum is computed from the image.
   *
   * \param[in] image The image from which to determine the maximum intensity.
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByImageAndMinimum(const Image* image, double minimum, unsigned int bins);

  /**
   * \brief Initialize from the intensity range of masked voxels in an image.
   *
   * \param[in] image The image from which to determine intensities.
   * \param[in] mask Binary mask; only voxels with mask value > 0 are considered.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByImageRegion(const Image* image, const Image* mask, unsigned int bins);

  /**
   * \brief Initialize from masked image region with a specified minimum; maximum is computed.
   *
   * \param[in] image The image from which to determine intensities.
   * \param[in] mask Binary mask; only voxels with mask value > 0 are considered.
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByImageRegionAndMinimum(const Image* image, const Image* mask, double minimum, unsigned int bins);

  /**
   * \brief Initialize from masked image region with a specified maximum; minimum is computed.
   *
   * \param[in] image The image from which to determine intensities.
   * \param[in] mask Binary mask; only voxels with mask value > 0 are considered.
   * \param[in] maximum The upper bound of the intensity range.
   * \param[in] bins Number of histogram bins.
   */
  void InitializeByImageRegionAndMaximum(const Image* image, const Image* mask, double maximum, unsigned int bins);

  /**
   * \brief Initialize from the full image intensity range using a specified bin size.
   *
   * \param[in] image The image from which to determine minimum and maximum intensities.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByImageAndBinsize(const Image* image, double binsize);

  /**
   * \brief Initialize from an image with specified minimum and bin size; maximum is computed.
   *
   * \param[in] image The image from which to determine the maximum intensity.
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByImageAndBinsizeAndMinimum(const Image* image, double minimum, double binsize);

  /**
   * \brief Initialize from an image with specified maximum and bin size; minimum is computed.
   *
   * \param[in] image The image from which to determine the minimum intensity.
   * \param[in] maximum The upper bound of the intensity range.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByImageAndBinsizeAndMaximum(const Image* image, double maximum, double binsize);

  /**
   * \brief Initialize from masked image region using a specified bin size.
   *
   * \param[in] image The image from which to determine intensities.
   * \param[in] mask Binary mask; only voxels with mask value > 0 are considered.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByImageRegionAndBinsize(const Image* image, const Image* mask, double binsize);

  /**
   * \brief Initialize from masked image region with specified minimum and bin size; maximum is computed.
   *
   * \param[in] image The image from which to determine intensities.
   * \param[in] mask Binary mask; only voxels with mask value > 0 are considered.
   * \param[in] minimum The lower bound of the intensity range.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByImageRegionAndBinsizeAndMinimum(const Image* image, const Image* mask, double minimum, double binsize);

  /**
   * \brief Initialize from masked image region with specified maximum and bin size; minimum is computed.
   *
   * \param[in] image The image from which to determine intensities.
   * \param[in] mask Binary mask; only voxels with mask value > 0 are considered.
   * \param[in] maximum The upper bound of the intensity range.
   * \param[in] binsize Width of each bin.
   */
  void InitializeByImageRegionAndBinsizeAndMaximum(const Image* image, const Image* mask, double maximum, double binsize);

  /**
   * \brief Map a continuous intensity value to a histogram bin index.
   *
   * The index is clamped to [0, Bins-1].
   *
   * \param[in] intensity The intensity value to quantize.
   * \return Zero-based bin index.
   * \pre The quantifier must be initialized.
   */
  unsigned int IntensityToIndex(double intensity);

  /**
   * \brief Get the lower bound intensity of a histogram bin.
   *
   * \param[in] index Zero-based bin index.
   * \return The minimum intensity value for the given bin.
   */
  double IndexToMinimumIntensity(unsigned int index);

  /**
   * \brief Get the center intensity of a histogram bin.
   *
   * \param[in] index Zero-based bin index.
   * \return The mean intensity value for the given bin.
   */
  double IndexToMeanIntensity(unsigned int index);

  /**
   * \brief Get the upper bound intensity of a histogram bin.
   *
   * \param[in] index Zero-based bin index.
   * \return The maximum intensity value for the given bin.
   */
  double IndexToMaximumIntensity(unsigned int index);

  /** \brief Get whether the quantifier has been initialized. */
  itkGetConstMacro(Initialized, bool);
  /** \brief Get the number of histogram bins. */
  itkGetConstMacro(Bins, unsigned int);
  /** \brief Get the bin size (width of each bin). */
  itkGetConstMacro(Binsize, double);
  /** \brief Get the minimum intensity of the histogram range. */
  itkGetConstMacro(Minimum, double);
  /** \brief Get the maximum intensity of the histogram range. */
  itkGetConstMacro(Maximum, double);

public:

//#ifndef DOXYGEN_SKIP

  void SetRequestedRegionToLargestPossibleRegion() override {};
  bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return true; };
  bool VerifyRequestedRegion() override { return false; };
  void SetRequestedRegion (const itk::DataObject * /*data*/) override {};

  // Override
  bool IsEmpty() const override
  {
    if(IsInitialized() == false)
      return true;
    const TimeGeometry* timeGeometry = const_cast<IntensityQuantifier*>(this)->GetUpdatedTimeGeometry();
    if(timeGeometry == nullptr)
      return true;
    return false;
  }


private:
  bool m_Initialized;
  unsigned int m_Bins;
  double m_Binsize;
  double m_Minimum;
  double m_Maximum;

};
}

#endif
