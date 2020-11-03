/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef IGTLTrackingDataDeviceSource_H_HEADER_INCLUDED_
#define IGTLTrackingDataDeviceSource_H_HEADER_INCLUDED_

#include "mitkIGTLDeviceSource.h"

namespace mitk {
  /**
  * \brief Connects a mitk::IGTLDevice to a
  *        MITK-OpenIGTLink-Message-Filter-Pipeline
  *
  * This class is the source of most OpenIGTLink tracking data pipelines.
  * Deriving from IGTLDeviceSource it encapsulates a mitk::IGTLDevice and
  * provides the tracking data messages of the connected
  * OpenIGTLink devices as igtl::MessageBase objects.
  * This means it filters for TDATA, QTDATA and TRANSFORM messages. It can
  * be configured to listen to one of these message types or to all of them.
  *
  * Note, that there is just one single output.
  *
  */
  class MITKOPENIGTLINK_EXPORT IGTLTrackingDataDeviceSource : public IGTLDeviceSource
  {
  public:
    /**OpenIGTLink tracking data message types*/
    enum TrackingMessageType
    {
      TDATA, TRANSFORM, QTDATA, UNKNOWN
    };

    mitkClassMacro(IGTLTrackingDataDeviceSource, IGTLDeviceSource);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    /** Sets the tracking data type the source should listen to. If it is set to unknown (default) it listens to all tracking data message types.*/
    itkSetMacro(trackingDataType, TrackingMessageType);
    itkGetConstMacro(trackingDataType, TrackingMessageType);

  protected:
    IGTLTrackingDataDeviceSource();
    ~IGTLTrackingDataDeviceSource() override;

    TrackingMessageType m_trackingDataType;

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
#endif /* MITKIGTLDeviceSource_H_HEADER_INCLUDED_ */
