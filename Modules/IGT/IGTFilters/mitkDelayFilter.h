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

#ifndef MITKDELAYFILTER_H
#define MITKDELAYFILTER_H

// MITK Header
#include "MitkIGTExports.h"
#include "mitkNavigationDataToNavigationDataFilter.h"
#include "mitkNavigationData.h"

//ITK header
#include <itkTimeStamp.h>

namespace mitk {
  class MitkIGT_EXPORT DelayFilter : public mitk::NavigationDataToNavigationDataFilter
  {
    /**
    * @sa itk::ProcessObject::MakeOutput(DataObjectPointerArraySizeType)
    */
    //    virtual DataObjectPointer MakeOutput ( DataObjectPointerArraySizeType idx );

    /**
    * @sa itk::ProcessObject::MakeOutput(const DataObjectIdentifierType&)
    */
    //    virtual DataObjectPointer MakeOutput(const DataObjectIdentifierType& name);

  protected:
    virtual void GenerateData();

    /**
    * This field containes the buffered navigation datas. It is a vector of (pair of (time and vector of (several navigation datas from one point in time))
    * In more clarity: The top level vector contains (one Navigation Data for each inout and the time these NDs have been recorded at).
    * The last line of this comment is meant to be viewed in the header file directly:
    |list of    |pairs of| timestamp and |one navigation Data for each input       |*/
    std::vector<std::pair<itk::TimeStamp, std::vector<mitk::NavigationData::Pointer>>> m_Buffer;

    long m_DeltaT;
  };
} // namespace mitk

#endif // MITKDELAYFILTER_H