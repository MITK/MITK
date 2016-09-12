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

#ifndef MITKUSDiPhASCustomControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASCustomControls_H_HEADER_INCLUDED_

#include "mitkUSDevice.h"
#include "mitkUSDiPhASDeviceCustomControls.h"
#include "mitkUSDiPhASDevice.h"

#include <itkObjectFactory.h>

namespace mitk {
/**
  * \brief Custom controls for mitk::USDiPhASDevice.
  */
class USDiPhASDevice;
class USDiPhASCustomControls : public USDiPhASDeviceCustomControls
{
public:
  mitkClassMacro(USDiPhASCustomControls, USAbstractControlInterface);
  mitkNewMacro1Param(Self, mitk::USDiPhASDevice*);

  /**
    * Activate or deactivate the custom controls. This is just for handling
    * widget visibility in a GUI for example.
    */
  virtual void SetIsActive( bool isActive ) override;

  /**
    * \return if this custom controls are currently activated
    */
  virtual bool GetIsActive( ) override;

protected:
  /**
    * Class needs an mitk::USDiPhASDevice object for beeing constructed.
    * This object's ScanMode will be manipulated by the custom controls methods.
    */
  USDiPhASCustomControls(USDiPhASDevice* device);
  virtual ~USDiPhASCustomControls( );

  bool                          m_IsActive;
  USImageVideoSource::Pointer   m_ImageSource;
  USDiPhASDevice*               m_device;

  virtual void OnSetSoundOfSpeed(int speed) override;
};
} // namespace mitk

#endif // MITKUSDiPhASCustomControls_H_HEADER_INCLUDED_