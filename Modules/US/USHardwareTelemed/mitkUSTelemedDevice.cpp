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

#include "mitkUSTelemedDevice.h"

#include "mitkUSTelemedSDKHeader.h"

mitk::USTelemedDevice::USTelemedDevice(std::string manufacturer, std::string model)
: mitk::USDevice(manufacturer, model),
m_ControlsProbes(mitk::USTelemedProbesControls::New(this)),
m_ControlsBMode(mitk::USTelemedBModeControls::New(this)),
m_ControlsDoppler(mitk::USTelemedDopplerControls::New(this)),
m_ImageSource(mitk::USTelemedImageSource::New()), m_UsgMainInterface(0),
m_Probe(0), m_UsgDataView(0), m_ProbesCollection(0)
{
  SetNumberOfOutputs(1);
  SetNthOutput(0, this->MakeOutput(0));
}

mitk::USTelemedDevice::~USTelemedDevice()
{
}

std::string mitk::USTelemedDevice::GetDeviceClass()
{
  return "org.mitk.modules.us.USTelemedDevice";
}

mitk::USControlInterfaceBMode::Pointer mitk::USTelemedDevice::GetControlInterfaceBMode()
{
  return m_ControlsBMode.GetPointer();
}

mitk::USControlInterfaceProbes::Pointer mitk::USTelemedDevice::GetControlInterfaceProbes()
{
  return m_ControlsProbes.GetPointer();
};

mitk::USControlInterfaceDoppler::Pointer mitk::USTelemedDevice::GetControlInterfaceDoppler()
{
  return m_ControlsDoppler.GetPointer();
};

bool mitk::USTelemedDevice::OnInitialization()
{
  CoInitialize(NULL); // initialize COM library

  return true;
}

bool mitk::USTelemedDevice::OnConnection()
{
  // create main Telemed API COM library object
  HRESULT hr;

  hr = CoCreateInstance(Usgfw2Lib::CLSID_Usgfw2, NULL, CLSCTX_INPROC_SERVER, Usgfw2Lib::IID_IUsgfw2,(LPVOID*) &m_UsgMainInterface);
  if (FAILED(hr))
  {
    SAFE_RELEASE(m_UsgMainInterface);
    MITK_ERROR("USDevice")("USTelemedDevice") << "Error at connecting to ultrasound device (" << hr << ").";
    return false;
  }

  this->ConnectDeviceChangeSink();

  return true;
}

bool mitk::USTelemedDevice::OnDisconnection()
{
  // control objects cannot be active anymore
  m_ControlsBMode->SetIsActive(false);
  m_ControlsDoppler->SetIsActive(false);
  m_ControlsProbes->SetIsActive(false);

  ReleaseUsgControls();

  return true;
}

bool mitk::USTelemedDevice::OnActivation()
{
  // probe controls are available now
  m_ControlsProbes->SetIsActive(true);

  if ( m_ControlsProbes->GetProbesCount() < 1 )
  {
    MITK_WARN("USDevice")("USTelemedDevice") << "No probe found.";
    return false;
  }

  // select first probe as a default
  m_ControlsProbes->SelectProbe(0);

  // set scan mode b as default for activation -
  // control interfaces can override this later
  HRESULT hr = m_UsgDataView->put_ScanMode(Usgfw2Lib::SCAN_MODE_B);
  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Could not set scan mode b (" << hr << ").";
    return false;
  }

  // start ultrasound scanning with selected scan mode
  hr = m_UsgDataView->put_ScanState(Usgfw2Lib::SCAN_STATE_RUN);
  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Start scanning failed (" << hr << ").";
    return false;
  }

  m_ControlsBMode->ReinitializeControls();

  return true;
}

bool mitk::USTelemedDevice::OnDeactivation()
{
  this->StopScanning();
  return true;
}

void mitk::USTelemedDevice::OnFreeze(bool freeze)
{
  if ( freeze )
  {
    m_UsgDataView->put_ScanState(Usgfw2Lib::SCAN_STATE_FREEZE);
  }
  else
  {
    m_UsgDataView->put_ScanState(Usgfw2Lib::SCAN_STATE_RUN);
  }
}

mitk::USImageSource::Pointer mitk::USTelemedDevice::GetUSImageSource()
{
  return m_ImageSource.GetPointer();
}

void mitk::USTelemedDevice::ReleaseUsgControls()
{
  if (m_UsgDataView) { this->StopScanning(); };

  SAFE_RELEASE(m_UsgMainInterface);
  SAFE_RELEASE(m_Probe);
  SAFE_RELEASE(m_UsgDataView);
  SAFE_RELEASE(m_ProbesCollection);
}

void mitk::USTelemedDevice::StopScanning()
{
  if ( ! m_UsgDataView )
  {
    MITK_WARN("USDevice")("USTelemedDevice") << "Cannot stop scanning as Telemed Data View is null.";
    return;
  }

  HRESULT hr;
  hr = m_UsgDataView->put_ScanState(Usgfw2Lib::SCAN_STATE_STOP);

  if (FAILED(hr))
  {
    MITK_ERROR("USDevice")("USTelemedDevice") << "Stop scanning failed (" << hr << ").";
    mitkThrow() << "Stop scanning failed (" << hr << ").";
  }
}

Usgfw2Lib::IUsgfw2* mitk::USTelemedDevice::GetUsgMainInterface()
{
  return m_UsgMainInterface;
}

void mitk::USTelemedDevice::SetActiveDataView(Usgfw2Lib::IUsgDataView* usgDataView)
{
  // do nothing if the usg data view hasn't changed
  if ( m_UsgDataView != usgDataView )
  {
    // scan converter plugin is connected to IUsgDataView -> a new plugin
    // must be created when changing IUsgDataView
    m_UsgDataView = usgDataView;
    if ( ! m_ImageSource->CreateAndConnectConverterPlugin(m_UsgDataView, Usgfw2Lib::SCAN_MODE_B)) { return; }

    // b mode control object must know about active data view
    m_ControlsBMode->SetUsgDataView(m_UsgDataView);
  }
}

void mitk::USTelemedDevice::ConnectDeviceChangeSink( )
{
  IConnectionPointContainer* cpc = NULL;
  HRESULT hr = m_UsgMainInterface->QueryInterface(IID_IConnectionPointContainer, (void**)&cpc);
  if (hr != S_OK)
    cpc = NULL;

  if (cpc != NULL)
    hr = cpc->FindConnectionPoint(Usgfw2Lib::IID_IUsgDeviceChangeSink, &m_UsgDeviceChangeCpnt);

  if (hr != S_OK)
  {
    m_UsgDeviceChangeCpnt = NULL;
    m_UsgDeviceChangeCpntCookie = 0;
  }
  SAFE_RELEASE(cpc);

  if (m_UsgDeviceChangeCpnt != NULL)
    hr = m_UsgDeviceChangeCpnt->Advise((IUnknown*)((Usgfw2Lib::IUsgDeviceChangeSink*)this), &m_UsgDeviceChangeCpntCookie);
}

// --- Methods for Telemed API Interfaces

HRESULT __stdcall mitk::USTelemedDevice::raw_OnBeamformerArrive(IUnknown *pUsgBeamformer, ULONG *reserved)
{
  this->Connect();

  return S_OK;
}

HRESULT __stdcall mitk::USTelemedDevice::raw_OnBeamformerRemove(IUnknown *pUsgBeamformer, ULONG *reserved)
{
  if ( this->GetIsActive() ) { this->Deactivate(); }

  this->Disconnect();

  return S_OK;
}

HRESULT __stdcall mitk::USTelemedDevice::raw_OnProbeArrive(IUnknown*, ULONG* probeIndex)
{
  m_ControlsProbes->ProbeAdded(static_cast<unsigned int>(*probeIndex));

  this->Activate();

  return S_OK;
};

HRESULT __stdcall mitk::USTelemedDevice::raw_OnProbeRemove(IUnknown*, ULONG* probeIndex)
{
  m_ControlsProbes->ProbeRemoved(static_cast<unsigned int>(*probeIndex));

  if ( this->GetIsActive() ) { this->Deactivate(); }

  return S_OK;
};

STDMETHODIMP_(ULONG) mitk::USTelemedDevice::AddRef()
{
  ++m_RefCount;
  return m_RefCount;
}

STDMETHODIMP_(ULONG) mitk::USTelemedDevice::Release()
{
  --m_RefCount;
  return m_RefCount;
}

STDMETHODIMP  mitk::USTelemedDevice::QueryInterface(REFIID riid, void** ppv)
{
  if (riid == IID_IUnknown || riid == Usgfw2Lib::IID_IUsgDeviceChangeSink)
  {
    *ppv = (IUsgDeviceChangeSink*)this;
    return S_OK;
  }
  if (riid == IID_IDispatch)
  {
    *ppv = (IDispatch*)this;
    return S_OK;
  }
  return E_NOINTERFACE;
}

HRESULT mitk::USTelemedDevice::GetTypeInfoCount(UINT *pctinfo)
{
  if (pctinfo == NULL) return E_INVALIDARG;
  *pctinfo = 0;
  return S_OK;
}

HRESULT mitk::USTelemedDevice::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
{
  if (pptinfo == NULL) return E_INVALIDARG;
  *pptinfo = NULL;
  if(itinfo != 0) return DISP_E_BADINDEX;
  return S_OK;
}

HRESULT mitk::USTelemedDevice::GetIDsOfNames(const IID &riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)
{
  // this is not used - must use the same fixed dispid's from Usgfw2 idl file
  return S_OK;
}

HRESULT mitk::USTelemedDevice::Invoke(DISPID dispIdMember, const IID &riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
  if ( (dispIdMember >= 1) && (dispIdMember <= 6) )
  {
    if (pDispParams->cArgs != 2) // we need 2 arguments
      return S_OK;

    IUnknown *unkn = NULL;
    ULONG *res = NULL;

    VARIANTARG* p1;
    VARIANTARG* p;
    p1 = pDispParams->rgvarg;

    p = p1;
    if (p->vt == (VT_BYREF|VT_UI4))
      res = p->pulVal;
    p1++;

    p = p1;
    if (p->vt == VT_UNKNOWN)
      unkn = (IUnknown*)(p->punkVal);

    if (dispIdMember == 1)
      OnProbeArrive(unkn, res);
    else if (dispIdMember == 2)
      OnBeamformerArrive(unkn, res);
    else if (dispIdMember == 3)
      OnProbeRemove(unkn, res);
    else if (dispIdMember == 4)
      OnBeamformerRemove(unkn, res);
    else if (dispIdMember == 5)
      OnProbeStateChanged(unkn, res);
    else if (dispIdMember == 6)
      OnBeamformerStateChanged(unkn, res);
  }

  return S_OK;
}