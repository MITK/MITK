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


#ifndef MITKUSDeviceService_H_HEADER_INCLUDED_
#define MITKUSDeviceService_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <vector>
#include "mitkUSDevice.h"

namespace mitk {

    /**Documentation
    * \brief TODO
    *
    * \ingroup US
    */

  class MitkUS_EXPORT USDeviceService : public itk::Object
    {
    public:
      mitkClassMacro(USDeviceService,itk::Object);
      itkNewMacro(Self);




      //## getter and setter ##

      /**
      * \brief Sets that given device as active in the service (i.e. ready to produce images)
      */
      void ActivateDevice (mitk::USDevice::Pointer device);
      /**
      * \brief Removes the device with index i from the List of currently active devices
      */
      void DeactivateDevice (int index);

      std::vector<mitk::USDevice::Pointer> GetActiveDevices();


    protected:
      USDeviceService();
      virtual ~USDeviceService();

      // This Vector contains all devices that are connected and active (i.e. ready to produce images)
      std::vector<mitk::USDevice::Pointer> m_ActiveDevices;

      // This Vector contains all known devices. Might later function as a catalogue. Not yet implemented.
      //std::vector<mitk::USDevice::Pointer> m_AllDevices;








    };
} // namespace mitk
#endif