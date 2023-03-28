/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkIGTL3DImageDeviceSource_h
#define mitkIGTL3DImageDeviceSource_h

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
  class MITKOPENIGTLINK_EXPORT IGTL3DImageDeviceSource : public IGTLDeviceSource
  {
  public:
    mitkClassMacro(IGTL3DImageDeviceSource, IGTLDeviceSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

  protected:
    IGTL3DImageDeviceSource();
    ~IGTL3DImageDeviceSource() override;

    /**
    * \brief filter execute method
    *
    * queries the OpenIGTLink device for new messages and updates its output
    * igtl::MessageBase objects with it.
    * \warning Will raise a std::out_of_range exception, if tools were added to
    * the OpenIGTLink device after it was set as input for this filter
    */
    void GenerateData() override;
  };
} // namespace mitk
#endif
