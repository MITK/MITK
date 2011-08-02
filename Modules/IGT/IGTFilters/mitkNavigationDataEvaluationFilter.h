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

    
    std::map<int,std::vector<mitk::Point3D>> m_LoggedPositions; //a map here, to have one list for every navigation data
    std::map<int,std::vector<mitk::Quaternion>> m_LoggedQuaternions;
    std::map<int,int> m_InavildSamples;


    /** @return returns a list with the distances to the mean of the list */
    std::vector<double> GetErrorList(std::vector<mitk::Point3D> list);

    mitk::Point3D GetMean(std::vector<mitk::Point3D> list);

    mitk::Quaternion GetMean(std::vector<mitk::Quaternion> list);


    double GetMean(std::vector<double> list);

    double GetMedian(std::vector<double> list);

    double GetMax(std::vector<double> list);

    double GetMin(std::vector<double> list);

    /** @return returns the standard derivation of the list */
    double GetStabw(std::vector<double> list);

    /** @return returns the sample standard derivation of the list */
    double GetSampleStabw(std::vector<double> list);
  };
} // namespace mitk

#endif /* MITKNavigationDataEvaluationFilter_H_HEADER_INCLUDED_ */