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


///////////////////////////////////////////////////////////////////////////
HRESULT __stdcall USTelemedScanConverterPlugin::QueryInterface(const IID& iid, void** ppv)
{
  //The reinterpret_cast operator allows any pointer to be converted into
  //any other pointer type.
  reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
  return S_OK ;
}

//////////////////////////////////////
ULONG __stdcall USTelemedScanConverterPlugin::AddRef()
{
  return InterlockedIncrement(&m_cRef) ;
}

//////////////////////////////////////
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

      if ( ! m_OutputImage->IsInitialized() )
      {
        unsigned int dim[]={(nOutX2 - nOutX1), (nOutY2 - nOutY1)}; // image dimensions

        m_OutputImage->Initialize(mitk::MakeScalarPixelType<BYTE>(), 2, dim);
      }

      /*for (int n = 0; n < nOutX2; n++)
      {
        MITK_INFO << pBufferOut[n];
      }*/

      mitk::ImageWriteAccessor imageWriteAccessor(m_OutputImage);
      m_OutputImage->SetSlice(pBufferOut);

      return S_OK;
}

STDMETHODIMP USTelemedScanConverterPlugin::ParameterCB (
  int nPin
    )
{
      return S_OK;
}

void USTelemedScanConverterPlugin::ReleasePlugin()
{
  if (m_Plugin != NULL)
  {
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
  HRESULT hr;
  ReleasePlugin();

  if (plugin == NULL)
  {
    MITK_ERROR("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Plugin must not be NULL when calling SetScanConverterPlugin.";
    return S_FALSE;
  }

  IUsgScanConverterPlugin* tmp_plugin;
  hr = plugin->QueryInterface(__uuidof(IUsgScanConverterPlugin), (void**)&tmp_plugin);

  if (FAILED(hr))
  {
    MITK_ERROR("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Could not query com interface for IUsgScanConverterPlugin (" << hr << ").";
    return hr;
  }

  hr = tmp_plugin->get_ScanConverter((IUnknown**)&m_Plugin);

  if (FAILED(hr))
  {
    MITK_ERROR("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Could not get ScanConverter from plugin (" << hr << ").";
    return hr;
  }

  SAFE_RELEASE(tmp_plugin);

  hr = m_Plugin->SetCallback(this,USPC_BUFFER_INTERIM_OUTPUT);

  if (FAILED(hr))
  {
    MITK_ERROR("IUsgfwScanConverterPluginCB")("ScanConverterPlugin")
      << "Could not set callback for plugin (" << hr << ").";
    return hr;
  }

  return S_OK;
}
