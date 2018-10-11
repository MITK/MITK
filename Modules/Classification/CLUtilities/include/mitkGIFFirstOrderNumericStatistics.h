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

#ifndef mitkGIFFirstNumericOrderStatistics_h
#define mitkGIFFirstNumericOrderStatistics_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  class MITKCLUTILITIES_EXPORT GIFFirstOrderNumericStatistics : public AbstractGlobalImageFeature
  {
  public:
    /**
    * \brief Calculates first order statistics of the given image.
    *
    * The first order statistics for the intensity distribution within a given Region of Interest (ROI)
    * is caluclated. The ROI is defined using a mask.
    *
    * The features are calculated on a quantified image. If the bin-size is too big, the obtained values
    * can be errornous and missleading. It is therefore important to use enough bins. The binned approach is
    * used in order to avoid floating-point related errors.
    *
    * This feature calculator is activated by the option <b>-first-order</b> or <b>-fo</b>.
    *
    * The connected areas are based on the binned image, the binning parameters can be set via the default
    * parameters as described in AbstractGlobalImageFeature. It is also possible to determine the
    * dimensionality of the neighbourhood using direction-related commands as described in AbstractGlobalImageFeature.
    * No other options are possible beside these two options.
    *
    * The features are calculated based on a mask. It is assumed that the mask is
    * of the type of an unsigned short image. All voxels with the value 1 are treated as masked.
    *
    * The following features are then defined using the (binned) voxel intensity \f$ x_i \f$ of each voxel, the probability
    * an intensity \f$ p_x \f$, and the overall number of voxels within the mask \f$ N_v \f$:
    * - <b>First Order::Mean</b>: The mean intensity within the ROI
    * \f[ \textup{Mean}= \mu = \frac{1}{N_v} \sum x_i \f]
    * - <b>First Order::Unbiased Variance</b>: An unbiased estimation of the variance:
    * \f[ \textup{Unbiased Variance} = \frac{1}{N_v - 1} \sum \left( x_i - \mu \right)^2 \f]
    * - <b>First Order::Biased Variance</b>: An biased estimation of the variance. If not specified otherwise, this is
    * used as the variance:
    * \f[ \textup{Biased Variance} = \sigma^2 = \frac{1}{N_v} \sum \left( x_i - \mu \right)^2 \f]
    * - <b>First Order::Unbiased Standard Deviation</b>: Estimation of diversity within the intensity values
    * \f[ \textup{Unbiased Standard Deviation} =  \sqrt{\frac{1}{N_v-1} \sum \left( x_i - \mu \right)^2} \f]
    * - <b>First Order::Biased Standard Deviation</b>: Estimation of diversity within the intensity values
    * \f[ \textup{Biased Standard Deviation} = \sigma = \sqrt{\frac{1}{N_v} \sum \left( x_i - \mu \right)^2} \f]
    * - <b>First Order::Skewness</b>:
    * \f[ \textup{Skewness} = \frac{\frac{1}{N_v} \sum \left( x_i - \mu \right)^3}{\sigma^3} \f]
    * - <b>First Order::Kurtosis</b>: The kurtosis is a measurement of the peakness of the given
    * distirbution:
    * \f[ \textup{Kurtosis} = \frac{\frac{1}{N_v} \sum \left( x_i - \mu \right)^4}{\sigma^4} \f]
    * - <b>First Order::Excess Kurtosis</b>: The kurtosis is a measurement of the peakness of the given
    * distirbution. The excess kurtosis is similar to the kurtosis, but is corrected by a fisher correction,
    * ensuring that a gaussian distribution has an excess kurtosis of 0.
    * \f[ \textup{Excess Kurtosis} = \frac{\frac{1}{N_v} \sum \left( x_i - \mu \right)^4}{\sigma^4} - 3 \f]
    * - <b>First Order::Median</b>: The median is defined as the median of the all intensities in the ROI.
    * - <b>First Order::Minimum</b>: The minimum is defined as the minimum of the all intensities in the ROI.
    * - <b>First Order::05th Percentile</b>: \f$ P_{5\%} \f$ The 5% percentile. 5% of all voxel do have this or a lower intensity.
    * - <b>First Order::10th Percentile</b>: \f$ P_{10\%} \f$ The 10% percentile. 10% of all voxel do have this or a lower intensity.
    * - <b>First Order::15th Percentile</b>: \f$ P_{15\%} \f$ The 15% percentile. 15% of all voxel do have this or a lower intensity.
    * - <b>First Order::20th Percentile</b>: \f$ P_{20\%} \f$ The 20% percentile. 20% of all voxel do have this or a lower intensity.
    * - <b>First Order::25th Percentile</b>: \f$ P_{25\%} \f$ The 25% percentile. 25% of all voxel do have this or a lower intensity.
    * - <b>First Order::30th Percentile</b>: \f$ P_{30\%} \f$ The 30% percentile. 30% of all voxel do have this or a lower intensity.
    * - <b>First Order::35th Percentile</b>: \f$ P_{35\%} \f$ The 35% percentile. 35% of all voxel do have this or a lower intensity.
    * - <b>First Order::40th Percentile</b>: \f$ P_{40\%} \f$ The 40% percentile. 40% of all voxel do have this or a lower intensity.
    * - <b>First Order::45th Percentile</b>: \f$ P_{45\%} \f$ The 45% percentile. 45% of all voxel do have this or a lower intensity.
    * - <b>First Order::50th Percentile</b>: \f$ P_{50\%} \f$ The 50% percentile. 50% of all voxel do have this or a lower intensity.
    * - <b>First Order::55th Percentile</b>: \f$ P_{55\%} \f$ The 55% percentile. 55% of all voxel do have this or a lower intensity.
    * - <b>First Order::60th Percentile</b>: \f$ P_{60\%} \f$ The 60% percentile. 60% of all voxel do have this or a lower intensity.
    * - <b>First Order::65th Percentile</b>: \f$ P_{65\%} \f$ The 65% percentile. 65% of all voxel do have this or a lower intensity.
    * - <b>First Order::70th Percentile</b>: \f$ P_{70\%} \f$ The 70% percentile. 70% of all voxel do have this or a lower intensity.
    * - <b>First Order::75th Percentile</b>: \f$ P_{75\%} \f$ The 75% percentile. 75% of all voxel do have this or a lower intensity.
    * - <b>First Order::80th Percentile</b>: \f$ P_{80\%} \f$ The 80% percentile. 80% of all voxel do have this or a lower intensity.
    * - <b>First Order::85th Percentile</b>: \f$ P_{85\%} \f$ The 85% percentile. 85% of all voxel do have this or a lower intensity.
    * - <b>First Order::90th Percentile</b>: \f$ P_{90\%} \f$ The 90% percentile. 90% of all voxel do have this or a lower intensity.
    * - <b>First Order::95th Percentile</b>: \f$ P_{95\%} \f$ The 95% percentile. 95% of all voxel do have this or a lower intensity.
    * - <b>First Order::Maximum</b>: The maximum is defined as the minimum of the all intensities in the ROI.
    * - <b>First Order::Range</b>: The range of intensity values is defined as the difference between the maximum
    * and minimum intensity in the ROI.
    * - <b>First Order::Interquartile Range</b>: The difference between the 75% and 25% quantile.
    * - <b>First Order::Mean Absolute Deviation</b>: The mean absolute deviation gives the mean distance of each
    * voxel intensity to the overal mean intensity and is a measure of the dispersion of the intensity form the
    * mean value:
    * \f[ \textup{Mean Absolute Deviation} = \frac{1}{N_v} \sum \left \| x_i - \mu \right \| \f]
    * - <b>First Order::Robust Mean</b>: The mean intensity within the ROI for all voxels between the 10% and 90% quantile:
    * \f[ \textup{Robust Mean}= \mu_R = \frac{1}{N_{vr}} \sum x_i \f]
    * - <b>First Order::Robust Mean Absolute Deviation</b>: The absolute deviation of all intensities within the ROI for
    * all voxels between the 10% and 90% quantilefrom the robust mean intensity:
    * \f[ \textup{Robust Mean Absolute Deviation}= \mu_R = \frac{1}{N_{vr}} \sum \left \| x_i - \mu_R \right \| \f]
    * - <b>First Order::Median Absolute Deviation</b>: Similar to the mean absolute deviation, but uses the median
    * instead of the mean to measure the center of the distribution.
    * - <b>First Order::Coefficient Of Variation</b>: Measures the dispersion of the intensity distribution:
    * \f[ \textup{Coefficient Of Variation} = \frac{sigma}{\mu} \f]
    * - <b>First Order::Quantile Coefficient Of Dispersion</b>: A robust alternative to teh coefficient of variance:
    * \f[ \textup{Quantile Coefficient Of Dispersion} = \frac{P_{75\%} - P_{25\%} }{P_{75\%} + P_{25\%}} \f]
    * - <b>First Order::Energy</b>: The intensity energy:
    * \f[ \textup{Energy} = \sum x_i ^2 \f]
    * - <b>First Order::Root Mean Square</b>: Root mean square is an important measure for the error.
    * \f[ \textup{Root Mean Square} = \sqrt{\frac{\sum x_i ^2}{N_v}} \f]
    * - <b>First Order::Uniformity</b>:
    * \f[ \textup{Uniformity} = \sum p_x^2 \f]
    * - <b>First Order::Entropy</b>:
    * \f[ \textup{Entropy} = - \sum p_x \textup{log}_2(p_x) \f]
    * - <b>First Order::Entropy</b>:
    * \f[ \textup{Entropy} = - \sum p_x \textup{log}_2(p_x) \f]
    * - <b>First Order::Covered Image Intensity Range</b>: Percentage of the image intensity range (maximum - minimum in whole
    * image) that is covered by the ROI.
    * - <b>First Order::Sum</b>: The sum of all intensities. It is correlated to the mean intensity.
    * \f[ \textup{Sum} =  \sum x_i \f]
    * - <b>First Order::Mode</b>: The most common intensity.
    * - <b>First Order::Mode Probability</b>: The likelihood of the most common intensity.
    * - <b>First Order::Number Of Voxels</b>: \f$ N_v \f$ the number of voxels covered by the ROI.
    * - <b>First Order::Image Dimension</b>: The dimensionality of the image (e.g. 2D, 3D, etc.).
    * - <b>First Order::Number Of Voxels</b>: The product of all spacing along all dimensions. In 3D, this is equal to the
    * volume.
    * - <b>First Order::Number Of Voxels</b>: The volume of a single voxel. If the dimensionality is only 2D, this is the
    * surface of an voxel.
    */
    mitkClassMacro(GIFFirstOrderNumericStatistics,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFFirstOrderNumericStatistics();

    /**
    * \brief Calculates the First Order Features based on a binned version of the image.
    */
    FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    FeatureNameListType GetFeatureNames() override;
    virtual std::string GetCurrentFeatureEncoding() override;

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);

  };
}
#endif //mitkGIFFirstNumericOrderStatistics_h
