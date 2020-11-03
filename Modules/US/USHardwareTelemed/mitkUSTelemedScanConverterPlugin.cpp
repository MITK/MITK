/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkUSTelemedScanConverterPlugin.h"
#include "mitkImageWriteAccessor.h"

USTelemedScanConverterPlugin::USTelemedScanConverterPlugin( )
  : m_Plugin(nullptr), m_OutputImage(nullptr), m_OutputImageMutex(nullptr)
{
}

USTelemedScanConverterPlugin::~USTelemedScanConverterPlugin( )
{
  ReleasePlugin();
}

// -- internal Telemed API function
HRESULT __stdcall USTelemedScanConverterPlugin::QueryInterface(const IID& iid, void** ppv)
{
  reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
  return S_OK ;
}

// -- internal Telemed API function
ULONG __stdcall USTelemedScanConverterPlugin::AddRef()
{
  return InterlockedIncrement(&m_cRef) ;
}

// -- internal Telemed API function
ULONG __stdcall USTelemedScanConverterPlugin::Release()
{
  if (InterlockedDecrement(&m_cRef) == 0)
  {
    delete this ;
    return 0 ;
  }
  return m_cRef ;
}

STDMETHODIMP USTelemedScanConverterPlugin::InterimOutBufferCB (
  PBYTE pBufferInterim,
  int nInterimBufferLen,
  PBYTE pBufferOut,
  int nOutBufferLen,
  int nOutX1,
  int nOutY1,
  int nOutX2,
  int nOutY2
  )
{
  if ( m_OutputImage.IsNull() ) { return S_FALSE; };

  if ( m_OutputImageMutex.IsNotNull() ) { m_OutputImageMutex->Lock(); }

  // initialize mitk::Image with given image size on the first time
  if ( ! m_OutputImage->IsInitialized() )
  {
    unsigned int dim[]={static_cast<unsigned int>(abs(nOutX2 - nOutX1)), static_cast<unsigned int>(abs(nOutY2 - nOutY1))}; // image dimensions

    m_OutputImage->Initialize(mitk::MakeScalarPixelType<BYTE>(), 2, dim);
  }

  // lock the image for writing an copy the given buffer into the image then
  m_OutputImage->SetSlice(pBufferOut);

  if ( m_OutputImageMutex.IsNotNull() ) { m_OutputImageMutex->Unlock(); }

  return S_OK;
}

void USTelemedScanConverterPlugin::ReleasePlugin()
{
  if (m_Plugin != nullptr)
  {
    // remove this callback from Telemed API plugin
    m_Plugin->SetCallback(nullptr,USPC_BUFFER_INTERIM_OUTPUT);
  }
}

void USTelemedScanConverterPlugin::SetOutputImage(mitk::Image::Pointer outputImage, itk::FastMutexLock::Pointer outputImageMutex)
{
  m_OutputImage = outputImage;
  m_OutputImageMutex = outputImageMutex;
}

STDMETHODIMP USTelemedScanConverterPlugin::SetScanConverterPlugin(IDispatch* plugin)
{
  // make sure that there is no scan converter plugin registered already
  this->ReleasePlugin();

  HRESULT hr;

  // it is ok to call this method with a nullptr plugin to remove
  // a previous callback
  if (plugin == nullptr)
  {
    MITK_INFO("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "nullptr plugin set to the scan converter. The callback for the previous plugin is removed now.";
    return S_OK;
  }

  // get Telemed API plugin from the COM library
  Usgfw2Lib::IUsgScanConverterPlugin* tmp_plugin;
  hr = plugin->QueryInterface(__uuidof(Usgfw2Lib::IUsgScanConverterPlugin), (void**)&tmp_plugin);

  if (FAILED(hr))
  {
    MITK_WARN("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Could not query com interface for IUsgScanConverterPlugin (" << hr << ").";
    return hr;
  }

  // get the converter for scan lines from the COM library and
  // save it as a member attribute
  hr = tmp_plugin->get_ScanConverter((IUnknown**)&m_Plugin);

  if (FAILED(hr))
  {
    MITK_WARN("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Could not get ScanConverter from plugin (" << hr << ").";
    return hr;
  }

  SAFE_RELEASE(tmp_plugin);

  // now the callback can be set -> interface functions of this
  // object will be called from now on when new image data is
  // available
  hr = m_Plugin->SetCallback(this,USPC_BUFFER_INTERIM_OUTPUT);

  if (FAILED(hr))
  {
    MITK_WARN("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Could not set callback for plugin (" << hr << ").";
    return hr;
  }

  return S_OK;
}
