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

#ifndef MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_
#define MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_

#include "mitkUSAbstractControlInterface.h"
#include "mitkUSProbe.h"

namespace mitk {
/**
  * \brief Interface defining methods for probe selection of ultrasound devices.
  * It consists of methods for getting all available probes and selecting one of
  * them.
  *
  * All of the methods of this interface must be implemented in a concrete
  * subclass. There are some pure virtual methods in the superclass
  * mitk::USAbstractControlInterface which must be implemente, too.
  *
  */
class MitkUS_EXPORT USControlInterfaceProbes : public USAbstractControlInterface
{
public:
  mitkClassMacro(USControlInterfaceProbes, USAbstractControlInterface);

  /**
    * \return vector of all probes currently available at the ultrasound device
    */
  virtual std::vector<USProbe::Pointer> GetProbeSet( ) = 0;

  /**
    * Select and activate probe by given index. The index corresponds to the
    * the index of the probe in the vector returned by
    * mitk::USControlInterfaceProbes::GetProbeSet().
    *
    * \param index index of the probe to be selected
    */
  virtual void SelectProbe( unsigned int index ) = 0;

  /**
    * Select and activate probe by given mitk::USProbe object. This object can
    * be one of these returned by mitk::USControlInterfaceProbes::GetProbeSet().
    *
    * \param probe pointer to probe to be selected
    */
  virtual void SelectProbe( USProbe::Pointer probe ) = 0;

  /**
    * \return number of probes currently available at the ultrasound device
    */
  virtual unsigned int GetProbesCount( ) const = 0;

protected:
  USControlInterfaceProbes( itk::SmartPointer<USDevice> device );
  virtual ~USControlInterfaceProbes( );
};
} // namespace mitk

#endif // MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_
