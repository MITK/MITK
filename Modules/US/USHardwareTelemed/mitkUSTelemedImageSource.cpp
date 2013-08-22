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

mitk::USTelemedImageSource::USTelemedImageSource(  )
  : m_Image(mitk::Image::New()), m_Plugin(0), m_PluginCallback(0)
{

}

mitk::USTelemedImageSource::~USTelemedImageSource( )
{
  SAFE_RELEASE(m_Plugin);
  SAFE_RELEASE(m_PluginCallback);
}

void mitk::USTelemedImageSource::GetNextRawImage( mitk::Image::Pointer& image)
{
  if ( m_Image->IsInitialized() )
  {
    image = m_Image;
  }
}

bool mitk::USTelemedImageSource::CreateAndConnectConverterPlugin(IUsgDataView* usgDataView, tagScanMode scanMode)
{
  IUnknown* tmp_obj = NULL;

  //get plugin
  mitk::telemed::CreateUsgControl( usgDataView, IID_IUsgScanConverterPlugin, scanMode, 0, (void**)&tmp_obj );
  if ( ! tmp_obj )
  {
    MITK_ERROR("USImageSource")("USTelemedImageSource") << "Could not create scan converter plugin.";
    return false;
  }

  SAFE_RELEASE(m_Plugin);
  m_Plugin = (IUsgScanConverterPlugin*)tmp_obj;

  SAFE_RELEASE(m_PluginCallback);
  m_PluginCallback = new USTelemedScanConverterPlugin();
  m_PluginCallback->SetScanConverterPlugin(m_Plugin);
  m_PluginCallback->SetOutputImage(m_Image.GetPointer());

  return true;
}