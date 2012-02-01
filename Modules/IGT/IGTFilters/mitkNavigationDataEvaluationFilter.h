/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 16011 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKNavigationDataEvaluationFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataEvaluationFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include <mitkPointSet.h>
#include <itkQuaternionRigidTransform.h>
#include <itkTransform.h>


namespace mitk {

  /**Documentation
  * \brief NavigationDataEvaluationFilter calculates statistical data (mean value, mean error, etc.) on the input navigation data.
  * Input navigation data are set 1:1 on output navigation data.
  *
  * \ingroup IGT
  */
  class MitkIGT_EXPORT NavigationDataEvaluationFilter : public NavigationDataToNavigationDataFilter
  {

  public:

    mitkClassMacro(NavigationDataEvaluationFilter, NavigationDataToNavigationDataFilter);
    itkNewMacro(Self);

    /** @brief Resets all statistics and starts again. */
    void ResetStatistic();

    /** @return Returns the number of analysed navigation datas for the specified input (without invalid samples). */
    int GetNumberOfAnalysedNavigationData(int input);
    /** @return Returns the number of invalid samples for the specified input. Invalid samples are ignored for the statistical calculation.*/
    int GetNumberOfInvalidSamples(int input);
    /** @return Returns the percentage of invalid samples in relation to all samples for the specified input.*/
    double GetPercentageOfInvalidSamples(int input);
    /** @return Returns the mean position of the specified input since the start of the statistic (last call of ResetStatistic()) */
    mitk::Point3D GetPositionMean(int input);
    /** @return Returns the standard derivation of each component (x, y and z) of the specified input since the start of the statistic (last call of ResetStatistic()) */
    mitk::Vector3D GetPositionStandardDeviation(int input);
    /** @return Returns the sample standard derivation of each component (x, y and z) of the specified input since the start of the statistic (last call of ResetStatistic()) */
    mitk::Vector3D GetPositionSampleStandardDeviation(int input);

    /** @return Returns the mean quaternion of the specified input since the start of the statistic (last call of ResetStatistic()) */
    mitk::Quaternion GetQuaternionMean(int input);
    /** @return Returns the standard derivation of each component of the specified input since the start of the statistic (last call of ResetStatistic()) */
    mitk::Quaternion GetQuaternionStandardDeviation(int input);
    /** @return Returns the mean euler angles (theta_x, theta_y, theta_z) of the specified input since the start of the statistic (last call of ResetStatistic()) */
    mitk::Vector3D GetEulerAnglesMean(int input);
    /** @return Returns the RMS of the error of the euler angles (theta_x, theta_y, theta_z) in radians of the specified input since the start of the statistic (last call of ResetStatistic()) */
    double GetEulerAnglesRMS(int input);
    /** @return Returns the RMS of the error of the euler angles (theta_x, theta_y, theta_z) in degree of the specified input since the start of the statistic (last call of ResetStatistic()) */
    double GetEulerAnglesRMSDegree(int input);
       
    /** @return Returns the mean distance to the mean postion (=mean error) to the specified input. */
    double GetPositionErrorMean(int input);
    /** @return Returns the standard derivation of the errors of all positions to the specified input. */
    double GetPositionErrorStandardDeviation(int input);
    /** @return Returns the sample standard derivation of the errors of all positions to the specified input. */
    double GetPositionErrorSampleStandardDeviation(int input);
    /** @return Returns the RMS of the errors of all positions to the specified input. */
    double GetPositionErrorRMS(int input);
    /** @return Returns the median of the errors of all positions to the specified input. */
    double GetPositionErrorMedian(int input);
    /** @return Returns the maximum of the errors of all positions to the specified input. */
    double GetPositionErrorMax(int input);
    /** @return Returns the minimum of the errors of all positions to the specified input. */
    double GetPositionErrorMin(int input);

    /** @return Returns a logged point on position i of the specified input. If there is no point on position i the method returns [0,0,0] */
    mitk::Point3D GetLoggedPosition(int i, int input);

    /** @return Returns a logged orientation on position i of the specified input. If there is no orientation on position i the method returns [0,0,0,0] */
    mitk::Quaternion GetLoggedOrientation(int i, int input);
      
  protected:

    NavigationDataEvaluationFilter();
    virtual ~NavigationDataEvaluationFilter();

    /**Documentation
    * \brief filter execute method
    *
    * transforms navigation data
    */
    virtual void GenerateData();

    /** @brief Creates the member variables which store all the statistical data for every input. */
    void CreateMembersForAllInputs();

    
    std::map<int,std::vector<mitk::Point3D> > m_LoggedPositions; //a map here, to have one list for every navigation data
    std::map<int,std::vector<mitk::Quaternion> > m_LoggedQuaternions;
    std::map<int,int> m_InavildSamples;

    mitk::Quaternion GetMean(std::vector<mitk::Quaternion> list);

    mitk::PointSet::Pointer VectorToPointSet(std::vector<mitk::Point3D> pSet);

    mitk::PointSet::Pointer VectorToPointSet(std::vector<mitk::Vector3D> pSet);

    /** @brief Converts a list of quaterions to a list of euler angles (theta_x, theta_y, theta_z) */
    std::vector<mitk::Vector3D> QuaternionsToEulerAngles(std::vector<mitk::Quaternion> quaterions); //in radians
    std::vector<mitk::Vector3D> QuaternionsToEulerAnglesGrad(std::vector<mitk::Quaternion> quaterions); //in degree
    
  };
} // namespace mitk

#endif /* MITKNavigationDataEvaluationFilter_H_HEADER_INCLUDED_ */