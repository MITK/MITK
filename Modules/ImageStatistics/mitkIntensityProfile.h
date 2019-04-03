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

#ifndef mitkIntensityProfile_h
#define mitkIntensityProfile_h

#include <itkListSample.h>
#include <mitkImage.h>
#include <mitkPlanarLine.h>
#include <mitkImageStatisticsCalculator.h>
#include <MitkImageStatisticsExports.h>

namespace mitk
{
  typedef itk::Statistics::ListSample<itk::Statistics::MeasurementVectorPixelTraits<ScalarType>::MeasurementVectorType> IntensityProfile;

  /** \brief Compute intensity profile of an image for each pixel along the first PolyLine of a given planar figure.
    *
    * \param[in] image A two or three-dimensional image which consists of single component pixels.
    * \param[in] planarFigure A planar figure from which the first PolyLine is used to evaluate the intensity profile.
    *
    * \return The computed intensity profile.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::Pointer ComputeIntensityProfile(Image::Pointer image, PlanarFigure::Pointer planarFigure);

  namespace InterpolateImageFunction
  {
    enum Enum
    {
      NearestNeighbor,
      Linear,
      WindowedSinc_Blackman_3,
      WindowedSinc_Blackman_4,
      WindowedSinc_Blackman_5,
      WindowedSinc_Cosine_3,
      WindowedSinc_Cosine_4,
      WindowedSinc_Cosine_5,
      WindowedSinc_Hamming_3,
      WindowedSinc_Hamming_4,
      WindowedSinc_Hamming_5,
      WindowedSinc_Lanczos_3,
      WindowedSinc_Lanczos_4,
      WindowedSinc_Lanczos_5,
      WindowedSinc_Welch_3,
      WindowedSinc_Welch_4,
      WindowedSinc_Welch_5
    };
  }

  /** \brief Compute intensity profile of an image for each sample along a planar line.
    *
    * \param[in] image A three-dimensional image which consists of single component pixels.
    * \param[in] planarLine A planar line along which the intensity profile will be evaluated.
    * \param[in] numSamples Number of samples along the planar line (must be at least 2).
    * \param[in] interpolator Image interpolation function which is used to read each sample.
    * \throw if image is 4D
    *
    * \return The computed intensity profile.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::Pointer ComputeIntensityProfile(Image::Pointer image, PlanarLine::Pointer planarLine, unsigned int numSamples, InterpolateImageFunction::Enum interpolator = InterpolateImageFunction::NearestNeighbor);

  /** \brief Compute intensity profile of an image for each sample between two points.
    *
    * \param[in] image A three-dimensional image which consists of single component pixels.
    * \param[in] startPoint A point at which the first sample is to be read.
    * \param[in] endPoint A point at which the last sample is to be read.
    * \param[in] numSamples Number of samples between startPoint and endPoint (must be at least 2).
    * \param[in] interpolator Image interpolation function which is used to read each sample.
   * \throw if image is 4D
    *
    * \return The computed intensity profile.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::Pointer ComputeIntensityProfile(Image::Pointer image, const Point3D& startPoint, const Point3D& endPoint, unsigned int numSamples, InterpolateImageFunction::Enum interpolator = InterpolateImageFunction::NearestNeighbor);

  /** \brief Compute global maximum of an intensity profile.
    *
    * \param[in] intensityProfile An intensity profile.
    *
    * \return Index of the global maximum.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::InstanceIdentifier ComputeGlobalMaximum(IntensityProfile::ConstPointer intensityProfile, IntensityProfile::MeasurementType &max);

  /** \brief Compute global minimum of an intensity profile.
    *
    * \param[in] intensityProfile An intensity profile.
    *
    * \return Index of the global minimum.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::InstanceIdentifier ComputeGlobalMinimum(IntensityProfile::ConstPointer intensityProfile, IntensityProfile::MeasurementType &min);

  /** \brief Compute statistics of an intensity profile.
    *
    * \param[in] intensityProfile An intensity profile.
    *
    * \param[in] stats An ImageStatisticsCalculator::Statistics object to hold the calculated statistics.
    *
    */
  MITKIMAGESTATISTICS_EXPORT void ComputeIntensityProfileStatistics(IntensityProfile::ConstPointer intensityProfile, ImageStatisticsContainer::ImageStatisticsObject& stats);

  /** \brief Compute center of maximum area under the curve of an intensity profile.
    *
    * \param[in] intensityProfile An intensity profile.
    * \param[in] radius Radius of the area (width of area equals 1 + 2 * radius).
    *
    * \return Index of the maximum area center.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::InstanceIdentifier ComputeCenterOfMaximumArea(IntensityProfile::ConstPointer intensityProfile, IntensityProfile::InstanceIdentifier radius);

  /** \brief Convert an intensity profile to a standard library vector.
    *
    * \param[in] intensityProfile An intensity profile.
    *
    * \return Standard library vector which contains the input intensity profile measurements.
    */
  MITKIMAGESTATISTICS_EXPORT std::vector<IntensityProfile::MeasurementType> CreateVectorFromIntensityProfile(IntensityProfile::ConstPointer intensityProfile);

  /** \brief Convert a standard library vector to an intensity profile.
    *
    * \param[in] vector An standard library vector which contains intensity profile measurements.
    *
    * \return An intensity profile.
    */
  MITKIMAGESTATISTICS_EXPORT IntensityProfile::Pointer CreateIntensityProfileFromVector(const std::vector<IntensityProfile::MeasurementType>& vector);
}

#endif
