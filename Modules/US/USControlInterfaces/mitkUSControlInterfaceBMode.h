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

  class MitkUS_EXPORT USControlInterfaceBMode : public USAbstractControlInterface
  {
  public:
    mitkClassMacro(USControlInterfaceBMode, USAbstractControlInterface);

    virtual double GetScanningDepth( ) = 0;
    virtual void SetScanningDepth( double ) = 0;
    virtual std::vector<double> GetScanningDepthValues( ) = 0;

    virtual double GetScanningGain( ) = 0;
    virtual void SetScanningGain( double ) = 0;
    virtual double GetScanningGainMin( ) = 0;
    virtual double GetScanningGainMax( ) = 0;
    virtual double GetScanningGainTick( ) = 0;

    virtual double GetScanningRejection( ) = 0;
    virtual void SetScanningRejection( double ) = 0;
    virtual double GetScanningRejectionMin( ) = 0;
    virtual double GetScanningRejectionMax( ) = 0;
    virtual double GetScanningRejectionTick( ) = 0;

  protected:
    USControlInterfaceBMode();
    virtual ~USControlInterfaceBMode();
  };

} // namespace mitk

#endif // MITKUSControlInterfaceBMode_H_HEADER_INCLUDED_
