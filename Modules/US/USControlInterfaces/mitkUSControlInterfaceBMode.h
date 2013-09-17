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
  * There is a getter and a setter defined for each kind of value. Additionaly
  * there are methods for getting the range of the possible values. For
  * uniformly distributed values there are three methods Get<VALUE>Min(),
  * Get<VALUE>Max() and Get<VALUE>Tick(). For non-uniformly distributed values
  * there is one method Get<VALUE>Values() which shoule return a list of all
  * possible values.
  *
  * All of the methods of this interface must be implemented in a concrete
  * subclass. There are some pure virtual methods in the superclass
  * mitk::USAbstractControlInterface which must be implemente, too.
  *
  */
class MitkUS_EXPORT USControlInterfaceBMode : public USAbstractControlInterface
{
public:
  mitkClassMacro(USControlInterfaceBMode, USAbstractControlInterface);

  /**
    * \return current depth value
    */
  virtual double GetScanningDepth( ) = 0;

  /**
    * \param depth new depth value
    */
  virtual void SetScanningDepth( double depth ) = 0;

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
    */
  virtual void SetScanningGain( double gain ) = 0;

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
    */
  virtual void SetScanningRejection( double rejection ) = 0;

  /**
    * \return minimum rejection value for the ultrasound device
    */
  virtual double GetScanningRejectionMin( ) = 0;

  /**
    * \return maximum rejection value for the ultrasound device
    */
  virtual double GetScanningRejectionMax( ) = 0;

  /**
    * \return interval between to rejection values for the ultrasound device
    */
  virtual double GetScanningRejectionTick( ) = 0;

protected:
  USControlInterfaceBMode( );
  virtual ~USControlInterfaceBMode( );
};

} // namespace mitk

#endif // MITKUSControlInterfaceBMode_H_HEADER_INCLUDED_
