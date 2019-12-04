/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef NAVIGATIONDATAVISUALIZATIONFILTER_H_INCLUDED
#define NAVIGATIONDATAVISUALIZATIONFILTER_H_INCLUDED

#include <itkProcessObject.h>

#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkNavigationData.h"
#include "mitkNavigationDataToNavigationDataFilter.h"

namespace mitk {
  /**Documentation
  * \brief NavigationDataVisualizationFilter represents the superclass of all IGT Filters that visualize NavigationData
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NavigationDataVisualizationFilter : public mitk::NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacroItkParent(NavigationDataVisualizationFilter,itk::ProcessObject);

    /**Documentation
    * \brief Set the input of this filter
    */
    virtual void SetInput( const NavigationData* nd);

    /**Documentation
    * \brief Set input with id idx of this filter
    */
    virtual void SetInput( unsigned int idx, const NavigationData* nd);

    /**Documentation
    * \brief Get the input of this filter
    */
    const NavigationData* GetInput(void);

    /**Documentation
    * \brief Get the input with id idx of this filter
    */
    const NavigationData* GetInput(unsigned int idx);

  protected:
    NavigationDataVisualizationFilter();
    virtual ~NavigationDataVisualizationFilter();

    virtual void GenerateData()=0;

  };
} // namespace mitk


#endif
