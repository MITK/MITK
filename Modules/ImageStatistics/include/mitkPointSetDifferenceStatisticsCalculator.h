/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkPointSetDifferenceStatisticsCalculator_h
#define mitkPointSetDifferenceStatisticsCalculator_h

#include <itkObject.h>
#include <MitkImageStatisticsExports.h>
#include <mitkImageStatisticsCalculator.h>
#include <mitkPointSet.h>

namespace mitk
{

/**
 * \brief Class for calculating the difference between two corresponding point sets.
 * The user can access the single distances between corresponding points as well as a complete statistic (mean, sd, rms, median, max, min)
 * The point sets must be of equal size!
 */
class MITKIMAGESTATISTICS_EXPORT PointSetDifferenceStatisticsCalculator : public itk::Object
{
public:

  mitkClassMacroItkParent( PointSetDifferenceStatisticsCalculator, itk::Object );
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  mitkNewMacro2Param(PointSetDifferenceStatisticsCalculator,mitk::PointSet::Pointer,mitk::PointSet::Pointer);

  /**
   * \brief Set the two point sets to be compared.
   * \param[in] pSet1 First point set.
   * \param[in] pSet2 Second point set. Must have the same size as pSet1.
   */
  void SetPointSets(mitk::PointSet::Pointer pSet1, mitk::PointSet::Pointer pSet2);

  /**
   * \brief Get the Euclidean distances between corresponding points.
   * \return A vector holding the distance for each pair of corresponding points.
   */
  std::vector<double> GetDifferences();

  /**
   * \brief Get the squared Euclidean distances between corresponding points.
   * \return A vector holding the squared distance for each pair of corresponding points.
   */
  std::vector<double> GetSquaredDifferences();

  /**
   * \brief Get the mean distance of all corresponding point pairs.
   * \return The mean Euclidean distance.
   */
  double GetMean();

  /**
   * \brief Get the standard deviation of the distances between all corresponding point pairs.
   * \return The standard deviation of distances.
   */
  double GetSD();

  /**
   * \brief Get the variance of the distances between all corresponding point pairs.
   * \return The variance of distances.
   */
  double GetVariance();

  /**
   * \brief Get the root mean squared distance of all corresponding point pairs.
   * \return The RMS distance.
   */
  double GetRMS();

  /**
   * \brief Get the median distance of all corresponding point pairs.
   * \return The median distance.
   */
  double GetMedian();

  /**
   * \brief Get the maximum distance of all corresponding point pairs.
   * \return The maximum distance.
   */
  double GetMax();

  /**
   * \brief Get the minimum distance of all corresponding point pairs.
   * \return The minimum distance.
   */
  double GetMin();

  /**
   * \brief Get the total number of corresponding point pairs.
   * \return The number of points.
   */
  double GetNumberOfPoints();

protected:

  PointSetDifferenceStatisticsCalculator();
  PointSetDifferenceStatisticsCalculator(mitk::PointSet::Pointer,mitk::PointSet::Pointer);
  ~PointSetDifferenceStatisticsCalculator() override;

  /**
   * \brief Compute the complete statistics of the distances between the given point sets.
   */
  void ComputeStatistics();

  mitk::ImageStatisticsContainer::ImageStatisticsObject m_Statistics; ///< struct holding the statistics
  std::vector<double> m_DifferencesVector; ///< vector holding the differences between the corresponding points
  std::vector<double> m_SquaredDifferencesVector; ///< vector holding the squared differences between the corresponding points
  mitk::PointSet::Pointer m_PointSet1; ///< first point set used for comparison
  mitk::PointSet::Pointer m_PointSet2; ///< second point set used for comparison
  bool m_StatisticsCalculated; ///< flag indicating whether statistics are already calculated or not.
};

}

#endif
