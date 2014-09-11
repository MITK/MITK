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

#ifndef MITKUSControlInterfaceBMode_H_HEADER_INCLUDED_
#define MITKUSControlInterfaceBMode_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"

namespace mitk {
/**
  * \brief Interface defining methods for scanning mode b of ultrasound devices.
  * It consists of methods for scanning depth, scanning gaing and scanning
  * rejection.
  *
  * Scanning Depth: Defines the clipping of the ultrasound image. Small depth
  * for looking at structures just below the skin. Great depth for seeing deeper
  * structures.
  *
  * Scanning Gain: Defines amplification of ultrasound echo.
  *
  * Scanning Rejection: Defines range of the received values from the ultrasound
  * signal.
  *
  * Scanning Dynamic Range: Ability of displaying strong and weak signals at the same
  * time. High dynamic range means that strong and weak signals can be distinguished.
  * Lower dynamic range can lead to more contrast.
  *
  * There is a getter and a setter defined for each kind of value. Additionaly
  * there are methods for getting the range of the possible values. For
  * uniformly distributed values there are three methods Get<VALUE>Min(),
  * Get<VALUE>Max() and Get<VALUE>Tick(). For non-uniformly distributed values
  * there is one method Get<VALUE>Values() which shoule return a list of all
  * possible values.
  *
  * All getters of this interface must be implemented in a concrete
  * subclass. Additionally all OnSet* methods must be implemented. They handle
  * the acutal setting of the values at the device api. The Set* methods are
  * already implemented in this class and should not be overwritten. There are
  * some pure virtual methods in the superclass
  * mitk::USAbstractControlInterface which must be implemented, too.
  *
  */
class MitkUS_EXPORT USControlInterfaceBMode : public USAbstractControlInterface
{
public:
  mitkClassMacro(USControlInterfaceBMode, USAbstractControlInterface);

  /**
    * \brief Initializes the microservice properties with the current values from the device.
    * This method must be called before one would like to get the
    * b mode values (e.g. frequency, power, ...) from the microservice
    * and after the device is ready to deliver the values by the methods
    * of this control interface.
    */
  void Initialize();

  /**
    * \return current frequency value
    */
  virtual double GetScanningFrequency( ) = 0;

  /**
    * \param depth new frequency value
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceBMode::OnSetScanningFrequency instead.
    */
  void SetScanningFrequency( double frequency );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceBMode::SetScanningFrequency.
    */
  virtual void OnSetScanningFrequency( double frequency ) = 0;

  /**
    * \return vector of all possible frequency values for the utrasound device
    */
  virtual std::vector<double> GetScanningFrequencyValues( ) = 0;

    /**
    * \return current power value
    */
  virtual double GetScanningPower( ) = 0;

  /**
    * \param depth new power value
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceBMode::OnSetScanningPower instead.
    */
  void SetScanningPower( double power );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceBMode::SetScanningPower.
    * Implement this method to handle the actual setting of the
    * value at the device api.
    */
  virtual void OnSetScanningPower( double power ) = 0;

  /**
    * \return minimum power value for the ultrasound device
    */
  virtual double GetScanningPowerMin( ) = 0;

  /**
    * \return maximum power value for the ultrasound device
    */
  virtual double GetScanningPowerMax( ) = 0;

  /**
    * \return interval between two power values for the ultrasound device
    */
  virtual double GetScanningPowerTick( ) = 0;

  /**
    * \return current depth value
    */
  virtual double GetScanningDepth( ) = 0;

  /**
    * \param depth new depth value
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceBMode::OnSetScanningDepth instead.
    */
  void SetScanningDepth( double depth );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceBMode::SetScanningDepth.
    * Implement this method to handle the actual setting of the
    * value at the device api.
    */
  virtual void OnSetScanningDepth( double depth ) = 0;

  /**
    * \return vector of all possible depth values for the utrasound device
    */
  virtual std::vector<double> GetScanningDepthValues( ) = 0;

  /**
    * \return current scanning gain
    */
  virtual double GetScanningGain( ) = 0;

  /**
    * \param gain new gain value
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceBMode::OnSetScanningGain instead.
    */
  void SetScanningGain( double gain );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceBMode::SetScanningGain.
    * Implement this method to handle the actual setting of the
    * value at the device api.
    */
  virtual void OnSetScanningGain( double gain ) = 0;

  /**
    * \return minimum gain value for the ultrasound device
    */
  virtual double GetScanningGainMin( ) = 0;

  /**
    * \return maximum gain value for the ultrasound device
    */
  virtual double GetScanningGainMax( ) = 0;

  /**
    * \return interval between two gain values for the ultrasound device
    */
  virtual double GetScanningGainTick( ) = 0;

  /**
    * \return current scanning rejection
    */
  virtual double GetScanningRejection( ) = 0;

  /**
    * \param rejection new rejection value
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceBMode::OnSetScanningRejection instead.
    */
  void SetScanningRejection( double rejection );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceBMode::SetScanningRejection.
    * Implement this method to handle the actual setting of the
    * value at the device api.
    */
  virtual void OnSetScanningRejection( double rejection ) = 0;

  /**
    * \return minimum rejection value for the ultrasound device
    */
  virtual double GetScanningRejectionMin( ) = 0;

  /**
    * \return maximum rejection value for the ultrasound device
    */
  virtual double GetScanningRejectionMax( ) = 0;

  /**
    * \return interval between two rejection values for the ultrasound device
    */
  virtual double GetScanningRejectionTick( ) = 0;

  /**
    * \return current scanning dynamic range
    */
  virtual double GetScanningDynamicRange( ) = 0;

  /**
    * \param rejection new dynamic range value
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceBMode::OnSetScanningDynamicRange instead.
    */
  virtual void SetScanningDynamicRange( double dynamicRange );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceBMode::SetScanningDynamicRange.
    * Implement this method to handle the actual setting of the
    * value at the device api.
    */
  virtual void OnSetScanningDynamicRange( double dynamicRange ) = 0;

  /**
  * \return minimum dynamic range value for the ultrasound device
  */
  virtual double GetScanningDynamicRangeMin( ) = 0;

  /**
  * \return maximum dynamic range value for the ultrasound device
  */
  virtual double GetScanningDynamicRangeMax( ) = 0;

  /**
  * \return interval between two dynamic range values for the ultrasound device
  */
  virtual double GetScanningDynamicRangeTick( ) = 0;

protected:
  USControlInterfaceBMode( itk::SmartPointer<USDevice> device );
  virtual ~USControlInterfaceBMode( );
};
} // namespace mitk

#endif // MITKUSControlInterfaceBMode_H_HEADER_INCLUDED_