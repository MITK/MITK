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

#ifndef MITKUSTelemedDevice_H_HEADER_INCLUDED_
#define MITKUSTelemedDevice_H_HEADER_INCLUDED_

#include "mitkUSDevice.h"

namespace mitk {

class USTelemedDevice : public USDevice
{
  mitk::USTelemedDevice::USTelemedDevice(std::string manufacturer, std::string model);

  /**
  * \brief Returns the Class of the Device. This Method must be reimplemented by every Inheriting Class.
  */
  virtual std::string GetDeviceClass();

  /**
  * \brief Is called during the connection process. Override this method in your subclass to handle the actual connection.
  *  Return true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
  */
  virtual bool OnConnection();

  /**
  * \brief Is called during the disconnection process. Override this method in your subclass to handle the actual disconnection.
  *  Return true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
  */
  virtual bool OnDisconnection();

  /**
  * \brief Is called during the activation process. After this method is finished, the device should be generating images
  */
  virtual bool OnActivation();


  /**
  * \brief Is called during the deactivation process. After a call to this method the device should still be connected, but not producing images anymore.
  */
  virtual void OnDeactivation();

  /**
  *  \brief Grabs the next frame from the Video input. This method is called internally, whenever Update() is invoked by an Output.
  */
   void GenerateData();
};

}

#endif // MITKUSTelemedDevice_H_HEADER_INCLUDED_
