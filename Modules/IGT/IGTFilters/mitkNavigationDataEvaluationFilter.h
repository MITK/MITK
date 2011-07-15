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

    /** @brief Resets the number of analysed navigation datas. */
    itkGetMacro(NumberAnalysedNavigationDatas,int);

  
  protected:

    NavigationDataEvaluationFilter();
    virtual ~NavigationDataEvaluationFilter();

    /**Documentation
    * \brief filter execute method
    *
    * transforms navigation data
    */
    virtual void GenerateData();

    mitk::Point3D m_SumPositions; //todo: make a map here, to have one sum for every navigation data
    int m_NumberAnalysedNavigationDatas;

  };
} // namespace mitk

#endif /* MITKNavigationDataEvaluationFilter_H_HEADER_INCLUDED_ */