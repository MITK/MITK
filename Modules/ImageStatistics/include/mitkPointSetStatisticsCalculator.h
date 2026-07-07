/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkPointSetStatisticsCalculator_h
#define mitkPointSetStatisticsCalculator_h

#include <itkObject.h>
#include <MitkImageStatisticsExports.h>
#include <mitkPointSet.h>

namespace mitk
{

/**
 * \brief Class for calculating statistics (like standard derivation, RMS, mean, etc.) for a PointSet.
 */
class MITKIMAGESTATISTICS_EXPORT PointSetStatisticsCalculator : public itk::Object
{
public:

  mitkClassMacroItkParent( PointSetStatisticsCalculator, itk::Object );
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  mitkNewMacro1Param(PointSetStatisticsCalculator,mitk::PointSet::Pointer);

  /** \brief Set the point set to be analysed. */
  void SetPointSet(mitk::PointSet::Pointer pSet);

  /** \brief Get the mean position of the analysed point set (only valid navigation data).
   * \return The mean position, or [0;0;0] if there is no valid navigation data.
   */
  mitk::Point3D GetPositionMean();

  /** \brief Get the standard deviation of each component (x, y, z) of the analysed point set.
   * \return The standard deviation vector, or [0;0;0] if there is no valid navigation data.
   */
  mitk::Vector3D GetPositionStandardDeviation();

  /** \brief Get the sample standard deviation of each component (x, y, z) of the analysed point set.
   * \return The sample standard deviation vector, or [0;0;0] if there is no valid navigation data.
   */
  mitk::Vector3D GetPositionSampleStandardDeviation();

  /** \brief Get the mean distance to the mean position (mean error) of the analysed point set.
   * \return The mean error, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorMean();

  /** \brief Get the standard deviation of position errors of the analysed point set.
   * \return The standard deviation of errors, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorStandardDeviation();

  /** \brief Get the sample standard deviation of position errors of the analysed point set.
   * \return The sample standard deviation of errors, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorSampleStandardDeviation();

  /** \brief Get the RMS of position errors of the analysed point set.
   * \return The RMS error, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorRMS();

  /** \brief Get the median of position errors of the analysed point set.
   * \return The median error, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorMedian();

  /** \brief Get the maximum of position errors of the analysed point set.
   * \return The maximum error, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorMax();

  /** \brief Get the minimum of position errors of the analysed point set.
   * \return The minimum error, or 0 if there is no valid navigation data.
   */
  double GetPositionErrorMin();

  //#####################################################################################################

  //this both methods are used by another class an so they are public... perhaps we want to move them
  //out of this class because they have nothing to do with point sets.

  /** \brief Compute the standard deviation of the given list (not of the point set).
   * \return The standard deviation.
   */
  double GetStabw(std::vector<double> list);

  /** \brief Compute the sample standard deviation of the given list (not of the point set).
   * \return The sample standard deviation.
   */
  double GetSampleStabw(std::vector<double> list);

  //#####################################################################################################


protected:

  PointSetStatisticsCalculator();
  explicit PointSetStatisticsCalculator(mitk::PointSet::Pointer);
  ~PointSetStatisticsCalculator() override;

  // TODO: Remove the std::vector<mitk::Point3D> data structure and use mitk::PointSet everywhere

  /** \brief Get a list of distances to the mean of the given point list.
   * \return A vector of error distances.
   */
  std::vector<double> GetErrorList(std::vector<mitk::Point3D> list);

  /** \brief Get the mean of the given point list.
   * \return The mean position, or [0;0;0] if the list is empty.
   */
  mitk::Point3D GetMean(std::vector<mitk::Point3D> list);

  /** \brief Convert a point set to a vector of Point3D. */
  std::vector<mitk::Point3D> PointSetToVector(mitk::PointSet::Pointer pSet);

  /** \brief Check whether all positions in the point set are equal.
   * \return True if all positions are identical, false otherwise.
   */
  bool CheckIfAllPositionsAreEqual();

  mitk::PointSet::Pointer m_PointSet;

  double GetMean(std::vector<double> list);

  double GetMedian(std::vector<double> list);

  double GetMax(std::vector<double> list);

  double GetMin(std::vector<double> list);

};

}

#endif
