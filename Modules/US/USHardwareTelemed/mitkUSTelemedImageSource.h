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


#ifndef MITKUSTelemedImageSource_H_HEADER_INCLUDED_
#define MITKUSTelemedImageSource_H_HEADER_INCLUDED_

#include "mitkUSImageSource.h"
#include "mitkUSTelemedSDKHeader.h"
#include "mitkUSTelemedScanConverterPlugin.h"

namespace mitk {

class USTelemedImageSource : public USImageSource
{
public:
  mitkClassMacro(USTelemedImageSource, USImageSource);
  itkNewMacro(Self);

  virtual void GetNextRawImage( mitk::Image::Pointer& );

  bool CreateAndConnectConverterPlugin( IUsgDataView*, tagScanMode );

protected:
  USTelemedImageSource( );
  virtual ~USTelemedImageSource( );

  IUsgScanConverterPlugin*        m_Plugin;
  USTelemedScanConverterPlugin*   m_PluginCallback;

  mitk::Image::Pointer            m_Image;
};

} // namespace mitk

#endif // MITKUSTelemedImageSource_H
