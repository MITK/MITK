/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKUSTelemedProbesControls_H_HEADER_INCLUDED_
#define MITKUSTelemedProbesControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSTelemedProbe.h"
#include "mitkUSControlInterfaceProbes.h"

#include <itkObjectFactory.h>

namespace mitk {
  class USTelemedDevice;

  /**
    * \brief Implementation of mitk::USControlInterfaceProbes for Telemed ultrasound devices.
    * See documentation of mitk::USControlInterfaceProbes for a description of the interface methods.
    */
  class USTelemedProbesControls : public USControlInterfaceProbes
  {
  public:
    mitkClassMacro(USTelemedProbesControls, USControlInterfaceProbes);
    mitkNewMacro1Param(Self, itk::SmartPointer<USTelemedDevice>);

    /**
      * Probe informations are fetched on activation. On deactivation there is nothing done.
      */
    virtual void SetIsActive(bool);

    virtual bool GetIsActive();

    virtual std::vector<USProbe::Pointer> GetProbeSet();
    virtual void OnSelectProbe(unsigned int index);
    virtual void OnSelectProbe(USProbe::Pointer probe);
    virtual USProbe::Pointer GetSelectedProbe();
    virtual unsigned int GetProbesCount() const;

    void ProbeRemoved(unsigned int index);
    void ProbeAdded(unsigned int index);

  protected:
    /**
      * Constructs an empty object.
      * Telemed device has to be set after constructing by calling
      * mitk::USTelemedProbesControls::SetTelemedDevice before the
      * object can be used.
      */
    USTelemedProbesControls(itk::SmartPointer<USTelemedDevice> device);
    virtual ~USTelemedProbesControls();

    /**
      * Create collection object (Telemed API) for the API device.
      */
    bool CreateProbesCollection();

    /**
      * Create vector of mitk::USTelemedProbe objects from the
      * Telemed API probe collection. Hence
      * mitk::USTelemedProbesControls::CreateProbesCollection has to
      * be called before.
      */
    void CreateProbesSet();

    bool                                  m_IsActive;
    unsigned int                          m_SelectedProbeIndex;
    std::vector<USTelemedProbe::Pointer>  m_ProbesSet;
    itk::SmartPointer<USTelemedDevice>    m_TelemedDevice;

    Usgfw2Lib::IUsgCollection*            m_ProbesCollection;
    Usgfw2Lib::IProbe*                    m_Probe;
  };
}

#endif // MITKUSTelemedProbesControls_H_HEADER_INCLUDED_
