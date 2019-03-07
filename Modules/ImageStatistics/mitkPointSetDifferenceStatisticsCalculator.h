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


#ifndef _MITK_PointSetDifferenceStatisticsCalculator_H
#define _MITK_PointSetDifferenceStatisticsCalculator_H

#include <itkObject.h>
#include <MitkImageStatisticsExports.h>
#include "mitkImageStatisticsCalculator.h"
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
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  mitkNewMacro2Param(PointSetDifferenceStatisticsCalculator,mitk::PointSet::Pointer,mitk::PointSet::Pointer);

  /*!
  \brief set point sets to be compared
  */
  void SetPointSets(mitk::PointSet::Pointer pSet1, mitk::PointSet::Pointer pSet2);
  /*!
  \brief returns a vector holding the differences between the corresponding points of the point sets
  */
  std::vector<double> GetDifferences();
  /*!
  \brief returns a vector holding the squared differences between the corresponding points of the point sets
  */
  std::vector<double> GetSquaredDifferences();
  /*!
  \brief returns the mean distance of all corresponding points of the point sets
  */
  double GetMean();
  /*!
  \brief returns the standard deviation of the distances between all corresponding points of the point sets
  */
  double GetSD();
  /*!
  \brief returns the variance of the distances between all corresponding points of the point sets
  */
  double GetVariance();
  /*!
  \brief returns the root mean squared distance of all corresponding points of the point sets
  */
  double GetRMS();
  /*!
  \brief returns the median distance of all corresponding points of the point sets
  */
  double GetMedian();
  /*!
  \brief returns the maximal distance of all corresponding points of the point sets
  */
  double GetMax();
  /*!
  \brief returns the minimal distance of all corresponding points of the point sets
  */
  double GetMin();
  /*!
  \brief returns the total number of corresponding points of the point sets
  */
  double GetNumberOfPoints();

protected:

  PointSetDifferenceStatisticsCalculator();
  PointSetDifferenceStatisticsCalculator(mitk::PointSet::Pointer,mitk::PointSet::Pointer);
  ~PointSetDifferenceStatisticsCalculator() override;

  /*!
  \brief Method for computing the complete statistics of the differences between the given point sets.
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

#endif // #define _MITK_PointSetDifferenceStatisticsCalculator_H
