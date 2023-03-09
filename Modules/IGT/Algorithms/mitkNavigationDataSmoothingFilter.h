/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkNavigationDataSmoothingFilter_h
#define mitkNavigationDataSmoothingFilter_h

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
    ~NavigationDataSmoothingFilter() override;

    void GenerateData() override;

    std::map< int, std::map< int , mitk::Point3D> > m_LastValuesList;

    int m_NumerOfValues;

    void InitializeLastValuesList();

    void AddValue(int outputID, mitk::Point3D value);

    mitk::Point3D GetMean(int outputID);

  };
} // namespace mitk

#endif
