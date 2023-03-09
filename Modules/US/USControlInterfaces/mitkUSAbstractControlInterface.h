/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUSAbstractControlInterface_h
#define mitkUSAbstractControlInterface_h

#include <mitkCommon.h>
#include <MitkUSExports.h>

#include <itkWeakPointer.h>

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
  ~USAbstractControlInterface( ) override;

  itk::WeakPointer<USDevice> m_Device;
};
} // namespace mitk

#endif
