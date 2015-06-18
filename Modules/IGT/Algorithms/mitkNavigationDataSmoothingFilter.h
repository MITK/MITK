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


#ifndef MITKNavigationDataSmoothingFilter_H_HEADER_INCLUDED_
#define MITKNavigationDataSmoothingFilter_H_HEADER_INCLUDED_

#include <mitkNavigationDataToNavigationDataFilter.h>
#include "MitkIGTExports.h"


namespace mitk {

  /**Documentation
  * \brief This filter smoothes the navigation data by calculating the mean value
  *        of the last few input values and using this as output.
  *
  * @ingroup Navigation
  */
  class MITKIGT_EXPORT NavigationDataSmoothingFilter : public NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataSmoothingFilter, NavigationDataToNavigationDataFilter);

    itkNewMacro(Self);

    /** @brief Sets the number of values before the current value which will be
     *         used for smoothing.
     */
    itkSetMacro(NumerOfValues,int);

  protected:
    NavigationDataSmoothingFilter();
    virtual ~NavigationDataSmoothingFilter();

    virtual void GenerateData() override;

    std::map< int, std::map< int , mitk::Point3D> > m_LastValuesList;

    int m_NumerOfValues;

    void InitializeLastValuesList();

    void AddValue(int outputID, mitk::Point3D value);

    mitk::Point3D GetMean(int outputID);

  };
} // namespace mitk

#endif /* MITKNavigationDataSmoothingFilter_H_HEADER_INCLUDED_ */