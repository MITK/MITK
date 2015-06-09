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

#ifndef NAVIGATIONDATAPASSTHROUGHFILTER_H
#define NAVIGATIONDATAPASSTHROUGHFILTER_H

#include "mitkNavigationDataToNavigationDataFilter.h"
#include "MitkUSNavigationExports.h"

namespace mitk {

/**
 * \brief Basis for filters that want to leave the navigation data untouched.
 *
 * Subclasses can call the mitk::NavigationDataToNavigationDataFilter::GenerateData()
 * method in their own GenerateData() implementation to pass through navigation data
 * from all inputs to the outputs.
 */
class MITKUSNAVIGATION_EXPORT NavigationDataPassThroughFilter : public NavigationDataToNavigationDataFilter
{
public:
  mitkClassMacro(NavigationDataPassThroughFilter, NavigationDataToNavigationDataFilter)
  itkNewMacro(Self)

protected:
  NavigationDataPassThroughFilter();
  virtual ~NavigationDataPassThroughFilter();

  /**
   * \brief Passes navigation data from all inputs to all outputs.
   * If a subclass wants to implement its own version of the GenerateData()
   * method it should call this method inside its implementation.
   */
  virtual void GenerateData() override;
};

} // namespace mitk

#endif // NAVIGATIONDATAPASSTHROUGHFILTER_H
