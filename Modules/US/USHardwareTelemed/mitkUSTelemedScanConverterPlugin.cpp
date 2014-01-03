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

#include "mitkUSTelemedScanConverterPlugin.h"
#include "mitkImageWriteAccessor.h"

USTelemedScanConverterPlugin::USTelemedScanConverterPlugin( )
  : m_Plugin(NULL), m_OutputImage(NULL)
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
  if ( ! m_OutputImage ) { return S_FALSE; };

  // initialize mitk::Image with given image size on the first time
  if ( ! m_OutputImage->IsInitialized() )
  {
    unsigned int dim[]={(nOutX2 - nOutX1), (nOutY2 - nOutY1)}; // image dimensions

    m_OutputImage->Initialize(mitk::MakeScalarPixelType<BYTE>(), 2, dim);
  }

  // lock the image for writing an copy the given buffer
  // into the image then
  mitk::ImageWriteAccessor imageWriteAccessor(m_OutputImage);
  m_OutputImage->SetSlice(pBufferOut);

  return S_OK;
}

void USTelemedScanConverterPlugin::ReleasePlugin()
{
  if (m_Plugin != NULL)
  {
    // remove this callback from Telemed API plugin
    m_Plugin->SetCallback(NULL,USPC_BUFFER_INTERIM_OUTPUT);
  }
  SAFE_RELEASE(m_Plugin);
}

void USTelemedScanConverterPlugin::SetOutputImage(mitk::Image::Pointer outputImage)
{
  m_OutputImage = outputImage;
}

STDMETHODIMP USTelemedScanConverterPlugin::SetScanConverterPlugin(IDispatch* plugin)
{
  // make sure that there is no scan converter plugin registered already
  ReleasePlugin();

  HRESULT hr;

  if (plugin == NULL)
  {
    MITK_WARN("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Plugin must not be NULL when calling SetScanConverterPlugin.";
    return S_FALSE;
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