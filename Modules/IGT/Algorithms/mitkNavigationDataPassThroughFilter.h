/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef NAVIGATIONDATAPASSTHROUGHFILTER_H
#define NAVIGATIONDATAPASSTHROUGHFILTER_H

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "MitkIGTExports.h"

namespace mitk {

/**
 * \brief Basis for filters that want to leave the navigation data untouched.
 *
 * Subclasses can call the mitk::NavigationDataToNavigationDataFilter::GenerateData()
 * method in their own GenerateData() implementation to pass through navigation data
 * from all inputs to the outputs.
 */
class MITKIGT_EXPORT NavigationDataPassThroughFilter : public NavigationDataToNavigationDataFilter
{
public:
  mitkClassMacro(NavigationDataPassThroughFilter, NavigationDataToNavigationDataFilter);
  itkNewMacro(Self);

protected:
  NavigationDataPassThroughFilter();
  ~NavigationDataPassThroughFilter() override;

  /**
   * \brief Passes navigation data from all inputs to all outputs.
   * If a subclass wants to implement its own version of the GenerateData()
   * method it should call this method inside its implementation.
   */
  void GenerateData() override;
};

} // namespace mitk

#endif // NAVIGATIONDATAPASSTHROUGHFILTER_H
