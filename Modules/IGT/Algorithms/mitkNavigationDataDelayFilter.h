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

#ifndef MITKNavigationDataDelayFilter_H
#define MITKNavigationDataDelayFilter_H

// MITK Header
#include "MitkIGTExports.h"
#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkNavigationData.h"
#include <mitkCommon.h>

//ITK header
#include <itkTimeStamp.h>

#include <queue>

namespace mitk {
  /**Documentation
  * \brief NavigationDataDelayFilter
  *
  * This Filter is used to delay Navigationdata by a certain amount of time.
  * It is used to synchronize TRacking data with other sources.
  *
  * \ingroup IGT
  */
  class MITKIGT_EXPORT NavigationDataDelayFilter : public NavigationDataToNavigationDataFilter
  {
    //     |pairs of| timestamp and |one navigation Data for each input
    typedef std::pair<itk::ModifiedTimeType, std::vector<mitk::NavigationData::Pointer> > BufferType;

  public:

    mitkClassMacro(NavigationDataDelayFilter, NavigationDataToNavigationDataFilter);
    mitkNewMacro1Param(NavigationDataDelayFilter, unsigned int);

    itkSetMacro(Delay, unsigned int);

  protected:

    NavigationDataDelayFilter(unsigned int delay);
    virtual ~NavigationDataDelayFilter();

    virtual void GenerateData() override;

    /**
    * \brief This field containes the buffered navigation datas. It is a queue of (pair of (time and vector of (several navigation datas from one point in time))
    * In more clarity: The top level queue contains (one Navigation Data for each inout and the time these NDs have been recorded at).
    */
    std::queue<BufferType> m_Buffer;

    /**
    * \brief The amount of time by which the Navigationdatas are delayed in milliseconds
    */
    unsigned int m_Delay;
    unsigned int m_Tolerance;
  };
} // namespace mitk

#endif // MITKNavigationDataDelayFilter_H
