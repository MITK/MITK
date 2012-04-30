/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKUSDevice_H_HEADER_INCLUDED_
#define MITKUSDevice_H_HEADER_INCLUDED_

#include <vector>
#include "mitkUSProbe.h"
#include "mitkUSImageMetadata.h"
#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <mitkImageSource.h>
#include <itkObjectFactory.h>


namespace mitk {

    /**Documentation
    * \brief A device holds information about it's model, make and the connected probes. It is the
    * common superclass for all Devices and acts as an image source for mitk Images as well as openCV Images.
    * if one of these functionalities is not supported by a subclass, it will throw an unsupported Operation Exception.
    * \ingroup US
    */
  class MitkUS_EXPORT USDevice : public itk::Object
    {
    public:
      mitkClassMacro(USDevice, mitk::ImageSource);
      mitkNewMacro3Param(Self, std::string, std::string, bool);

      /**
      * \brief Add a probe to the device without connecting to it.
      *  This should usually be done before connecting to the probe
      */
      virtual void AddProbe(mitk::USProbe::Pointer probe);

      /**
      * \brief Connect to a probe and activate it. The probe should be added first.
      *  Usually, a VideoDevice will just add a probe it want's to connect to,
      *  but an SDK Device might require adding a probe first.
      */
      virtual void ActivateProbe(mitk::USProbe::Pointer probe);

      /**
      * \brief Deactivates the currently active probe.
      */
      virtual void DeactivateProbe();

      /**
      * \brief Removes a probe from the ist of currently added probes.
      */
      //virtual void removeProbe(mitk::USProbe::Pointer probe);
    
      std::vector<mitk::USProbe::Pointer> GetConnectedProbes();

      //########### GETTER & SETTER ##################//

      itkGetMacro(ActiveProbe, mitk::USProbe::Pointer);
      std::string GetDeviceManufacturer();
      std::string GetDeviceModel();
      std::string GetDeviceComment();
      bool GetIsVideoOnly();

    protected:
      mitk::USProbe::Pointer m_ActiveProbe;
      std::vector<mitk::USProbe::Pointer> m_ConnectedProbes; 
      mitk::USImageMetadata::Pointer m_Metadata;

      
      /**
      * \brief Enforces minimal Metadata to be set. The isVideoOnly flag indicates that this class 
      *        only handles a videostream and does not recieve Metadata from the physical device itself.
      */
      USDevice(std::string manufacturer, std::string model, bool isVideoOnly);
      virtual ~USDevice();


    };
} // namespace mitk
#endif