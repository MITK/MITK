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

#ifndef MITKUSDiPhASDeviceCustomControls_H_HEADER_INCLUDED_
#define MITKUSDiPhASDeviceCustomControls_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"
#include "mitkUSImageVideoSource.h"
#include "mitkUSDevice.h"

#include <itkObjectFactory.h>

namespace mitk {
/**
  * \brief Custom controls for mitk::USDiPhASDevice.
  */
class MITKUS_EXPORT USDiPhASDeviceCustomControls : public USAbstractControlInterface
{
public:
  mitkClassMacro(USDiPhASDeviceCustomControls, USAbstractControlInterface);
  mitkNewMacro1Param(Self, itk::SmartPointer<USDevice>);

  /**
    * Activate or deactivate the custom controls. This is just for handling
    * widget visibility in a GUI for example.
    */
  virtual void SetIsActive( bool isActive ) override;

  /**
    * \return if this custom controls are currently activated
    */
  virtual bool GetIsActive( ) override;

  virtual void SetSoundOfSpeed(int speed);

protected:
  /**
    * Class needs an mitk::USDevice object for beeing constructed.
    */
  USDiPhASDeviceCustomControls( itk::SmartPointer<USDevice> device );
  virtual ~USDiPhASDeviceCustomControls( );

  bool                          m_IsActive;
  USImageVideoSource::Pointer   m_ImageSource;

  
  virtual void OnSetSoundOfSpeed(int speed);

};
} // namespace mitk

#endif // MITKUSDiPhASDeviceCustomControls_H_HEADER_INCLUDED_