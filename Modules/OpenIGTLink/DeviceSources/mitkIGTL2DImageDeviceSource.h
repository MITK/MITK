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

#ifndef IGTL2DIMAGEDEVICESOURCE_H_HEADER_INCLUDED_
#define IGTL2DIMAGEDEVICESOURCE_H_HEADER_INCLUDED_

#include "mitkIGTLDeviceSource.h"

namespace mitk {
  /**
  * \brief Connects a mitk::IGTLDevice to a
  *        MITK-OpenIGTLink-Message-Filter-Pipeline
  *
  * This class is the source of most OpenIGTLink pipelines. It encapsulates a
  * mitk::IGTLDevice and provides the information/messages of the connected
  * OpenIGTLink devices as igtl::MessageBase objects. Note, that there is just
  * one single output.
  *
  */
  class MITKOPENIGTLINK_EXPORT IGTL2DImageDeviceSource : public IGTLDeviceSource
  {
  public:
    mitkClassMacro(IGTL2DImageDeviceSource, IGTLDeviceSource);
    itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

  protected:
    IGTL2DImageDeviceSource();
    virtual ~IGTL2DImageDeviceSource();

    /**
    * \brief filter execute method
    *
    * queries the OpenIGTLink device for new messages and updates its output
    * igtl::MessageBase objects with it.
    * \warning Will raise a std::out_of_range exception, if tools were added to
    * the OpenIGTLink device after it was set as input for this filter
    */
    virtual void GenerateData() override;
  };
} // namespace mitk
#endif /* MITKIGTLDeviceSource_H_HEADER_INCLUDED_ */
