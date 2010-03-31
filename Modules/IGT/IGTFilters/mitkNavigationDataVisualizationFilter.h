/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  class MitkIGT_EXPORT NavigationDataVisualizationFilter : public mitk::NavigationDataToNavigationDataFilter
  {
  public:
    mitkClassMacro(NavigationDataVisualizationFilter,itk::ProcessObject);

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
