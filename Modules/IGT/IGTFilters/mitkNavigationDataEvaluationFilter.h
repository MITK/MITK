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

    /** @return returns the number of analysed navigation datas for the specified input. */
    int GetNumberOfAnalysedNavigationData(int input);


    double GetPositionMean(int input);
    double GetPositionStandardDerivation(int input);

    double GetQuaternionMean(int input);
    double GetQuaternionStandardDerivation(int input);
    

    /** @return Returns the mean distance to the mean postion. */
    double GetPositionErrorMean(int input);
    double GetPositionErrorStandardDerication(int input);
    double GetPositionErrorRMS(int input);
    double GetPositionErrorMedian(int input);
    
    double GetQuaternionErrorMean(int input);
    double GetQuaternionErrorStandardDerication(int input);
    double GetQuaternionErrorRMS(int input);
    double GetQuaternionErrorMedian(int input);
    
    
  
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
  };
} // namespace mitk

#endif /* MITKNavigationDataEvaluationFilter_H_HEADER_INCLUDED_ */