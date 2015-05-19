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

#ifndef MITKUSAbstractControlInterface_H_HEADER_INCLUDED_
#define MITKUSAbstractControlInterface_H_HEADER_INCLUDED_

#include <mitkCommon.h>
#include <MitkUSExports.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USDevice;

/**
  * \brief Superclass for all ultrasound device control interfaces.
  * Defines an interface for activating and deactivating a control interface.
  */
class MITKUS_EXPORT USAbstractControlInterface : public itk::Object
{
public:
  mitkClassMacroItkParent(USAbstractControlInterface, itk::Object);

  /**
    * Method must be implemented in every control interface to handle all
    * activities which should be done on activating and deactivating the
    * concrete control interface.
    * \param isActive whether the control interface should be activated or deactivated
    */
  virtual void SetIsActive( bool isActive ) = 0;

  /**
    * Method must be implemented in every control interface to tell the caller
    * if the interface is currently activated or deactivated.
    * \return whether the control interface is active or not
    */
  virtual bool GetIsActive( ) = 0;

protected:
  USAbstractControlInterface( itk::SmartPointer<USDevice> device );
  virtual ~USAbstractControlInterface( );

  itk::SmartPointer<USDevice> m_Device;
};
} // namespace mitk

#endif // MITKUSAbstractControlInterface_H_HEADER_INCLUDED_
