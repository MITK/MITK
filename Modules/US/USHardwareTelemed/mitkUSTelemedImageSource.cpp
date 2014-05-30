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

#include "mitkUSTelemedImageSource.h"
#include "mitkUSTelemedSDKHeader.h"
#include "MITKUSTelemedScanConverterPlugin.h"
#include "mitkImageReadAccessor.h"

mitk::USTelemedImageSource::USTelemedImageSource(  )
  : m_Image(mitk::Image::New()),
    m_ImageMutex(itk::FastMutexLock::New()),
    m_Plugin(0),
    m_PluginCallback(0)
{
}

mitk::USTelemedImageSource::~USTelemedImageSource( )
{
  SAFE_RELEASE(m_PluginCallback);
  SAFE_RELEASE(m_Plugin);
}

void mitk::USTelemedImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  if ( image.IsNull() ) { image = mitk::Image::New(); }
  if ( m_Image->IsInitialized() )
  {
    m_ImageMutex->Lock();

    // copy contents of the given image into the member variable
    image->Initialize(m_Image->GetPixelType(), m_Image->GetDimension(), m_Image->GetDimensions());
    mitk::ImageReadAccessor inputReadAccessor(m_Image, m_Image->GetSliceData(0,0,0));
    image->SetSlice(inputReadAccessor.GetData());

    m_ImageMutex->Unlock();
  }
}

bool mitk::USTelemedImageSource::CreateAndConnectConverterPlugin(Usgfw2Lib::IUsgDataView* usgDataView, Usgfw2Lib::tagScanMode scanMode)
{
  IUnknown* tmp_obj = NULL;

  // create control object from Telemed API
  mitk::telemed::CreateUsgControl( usgDataView, Usgfw2Lib::IID_IUsgScanConverterPlugin, scanMode, 0, (void**)&tmp_obj );
  if ( ! tmp_obj )
  {
    MITK_ERROR("USImageSource")("USTelemedImageSource") << "Could not create scan converter plugin.";
    return false;
  }

  // create the callback object for the scan conversion
  if ( ! m_PluginCallback )
  {
    m_PluginCallback = new USTelemedScanConverterPlugin();

    // current image buffer should be copied to m_Image at every callback
    m_PluginCallback->SetOutputImage(m_Image.GetPointer(), m_ImageMutex);
  }
  else
  {
    // make sure that the scan converter plugin is not set
    // to the plugin callback any longer
    m_PluginCallback->SetScanConverterPlugin(0);
  }

  // now the ScanConverterPlugin can be created and set as plugin
  SAFE_RELEASE(m_Plugin);
  m_Plugin = (Usgfw2Lib::IUsgScanConverterPlugin*)tmp_obj;
  m_PluginCallback->SetScanConverterPlugin(m_Plugin);

  return true;
}