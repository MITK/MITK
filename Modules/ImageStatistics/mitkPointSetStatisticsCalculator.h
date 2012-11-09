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


#ifndef _MITK_PointSetSTATISTICSCALCULATOR_H
#define _MITK_PointSetSTATISTICSCALCULATOR_H

#include <itkObject.h>
#include "ImageStatisticsExports.h"
#include <mitkPointSet.h>

namespace mitk
{

/**
 * \brief Class for calculating statistics (like standard derivation, RMS, mean, etc.) for a PointSet.
 */
class ImageStatistics_EXPORT PointSetStatisticsCalculator : public itk::Object
{
public:

  mitkClassMacro( PointSetStatisticsCalculator, itk::Object );
  itkNewMacro( PointSetStatisticsCalculator );

  mitkNewMacro1Param(PointSetStatisticsCalculator,mitk::PointSet::Pointer)

  /** @brief Sets the point set which will be analysed. */
  void SetPointSet(mitk::PointSet::Pointer pSet);

  /** @return Returns the mean position of the analysed point set (only valid navigation data). Returns [0;0;0] if there is no valid navigation data.*/
  mitk::Point3D GetPositionMean();

  /** @return Returns the standard derivation of each component (x, y and z) of the analysed point set (only valid navigation data). Returns [0;0;0] if there is no valid navigation data.*/
  mitk::Vector3D GetPositionStandardDeviation();

  /** @return Returns the sample standard derivation of each component (x, y and z) of the analysed point set (only valid navigation data). Returns [0;0;0] if there is no valid navigation data.*/
  mitk::Vector3D GetPositionSampleStandardDeviation();

  /** @return Returns the mean distance to the mean postion (=mean error) of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorMean();

  /** @return Returns the standard derivation of the errors of all positions of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorStandardDeviation();

  /** @return Returns the sample standard derivation of the errors of all positions of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorSampleStandardDeviation();

  /** @return Returns the RMS of the errors of all positions of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorRMS();

  /** @return Returns the median of the errors of all positions of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorMedian();

  /** @return Returns the maximum of the errors of all positions of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorMax();

  /** @return Returns the minimum of the errors of all positions of the analysed point set (only valid navigation data). Returns 0 if there is no valid navigation data. */
  double GetPositionErrorMin();

  //#####################################################################################################

  //this both methods are used by another class an so they are public... perhaps we want to move them
  //out of this class because they have nothing to do with point sets.

  /** @return returns the standard derivation of the given list (NOT of the point set).*/
  double GetStabw(std::vector<double> list);

  /** @return returns the sample standard derivation of the given list (NOT of the point set).*/
  double GetSampleStabw(std::vector<double> list);

  //#####################################################################################################


protected:

  PointSetStatisticsCalculator();
  PointSetStatisticsCalculator(mitk::PointSet::Pointer);
  virtual ~PointSetStatisticsCalculator();

  // TODO: Remove the std::vector<mitk::Point3D> data structure and use mitk::PointSet everywhere

  /** @return Returns a list with the distances to the mean of the list */
  std::vector<double> GetErrorList(std::vector<mitk::Point3D> list);

  /** @return Returns the mean of the point list. Returns [0;0;0] if the list is empty. */
  mitk::Point3D GetMean(std::vector<mitk::Point3D> list);

  /** @brief Converts a point set to a vector of Point3D. */
  std::vector<mitk::Point3D> PointSetToVector(mitk::PointSet::Pointer pSet);

  /** @return Returns true if all positions in the evaluated points set are equal. False if not. */
  bool CheckIfAllPositionsAreEqual();

  mitk::PointSet::Pointer m_PointSet;

  double GetMean(std::vector<double> list);

  double GetMedian(std::vector<double> list);

  double GetMax(std::vector<double> list);

  double GetMin(std::vector<double> list);

};

}

#endif // #define _MITK_PointSetSTATISTICSCALCULATOR_H
