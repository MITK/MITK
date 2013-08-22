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

#ifndef MITKUSTelemedProbesControls_H_HEADER_INCLUDED_
#define MITKUSTelemedProbesControls_H_HEADER_INCLUDED_

#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSTelemedProbe.h"
#include "mitkUSControlInterfaceProbes.h"

#include <itkObjectFactory.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USTelemedDevice;

  class USTelemedProbesControls : public USControlInterfaceProbes
  {
  public:
    mitkClassMacro(USTelemedProbesControls, USControlInterfaceProbes);
    itkNewMacro(Self);

    virtual void SetIsActive(bool);
    virtual bool GetIsActive();

    virtual std::vector<USProbe::Pointer> GetProbeSet();
    virtual void SelectProbe(unsigned int index);
    virtual void SelectProbe(USProbe::Pointer probe);

    void SetTelemedDevice(itk::SmartPointer<USTelemedDevice> telemedDevice);

  protected:
    USTelemedProbesControls();
    virtual ~USTelemedProbesControls();

    bool CreateProbesCollection();
    void CreateProbesSet();

    bool                                m_IsActive;
    std::vector<USTelemedProbe::Pointer>       m_ProbesSet;
    itk::SmartPointer<USTelemedDevice>  m_TelemedDevice;

    IUsgCollection*                 m_ProbesCollection;
    IProbe*                         m_Probe;
  };
}

#endif // MITKUSTelemedProbesControls_H_HEADER_INCLUDED_