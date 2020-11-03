/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
class MITKUS_EXPORT USControlInterfaceProbes : public USAbstractControlInterface
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
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceProbes::OnSelectProbe() instead.
    *
    * \param index index of the probe to be selected
    */
  void SelectProbe( unsigned int index );

  /**
    * Select and activate probe by given mitk::USProbe object. This object can
    * be one of these returned by mitk::USControlInterfaceProbes::GetProbeSet().
    *
    * Do not override this method in a subclass. Implement
    * mitk::USControlInterfaceProbes::OnSelectProbe() instead.
    *
    * \param probe pointer to probe to be selected
    */
  void SelectProbe( USProbe::Pointer probe );

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceProbes::SelectProbe().
    * Implement this method to handle the actual selecting of the probe at the
    * device api.
    *
    * \param index index of the probe to be selected
    */
  virtual void OnSelectProbe( unsigned int index ) = 0;

  /**
    * \brief Virtual method which is called inside mitk::USControlInterfaceProbes::SelectProbe().
    * Implement this method to handle the actual selecting of the probe at the
    * device api.
    *
    * \param probe pointer to probe to be selected
    */
  virtual void OnSelectProbe( USProbe::Pointer probe ) = 0;

  /**
    * \return Ultrasound probe which is currently selected as the active probe.
    */
  virtual USProbe::Pointer GetSelectedProbe() = 0;

  /**
    * \return number of probes currently available at the ultrasound device
    */
  virtual unsigned int GetProbesCount( ) const = 0;

protected:
  USControlInterfaceProbes( itk::SmartPointer<USDevice> device );
  ~USControlInterfaceProbes( ) override;
};
} // namespace mitk

#endif // MITKUSControlInterfaceProbes_H_HEADER_INCLUDED_
