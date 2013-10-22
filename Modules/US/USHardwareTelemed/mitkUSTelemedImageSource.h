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
/**
  * \brief Implementation of mitk::USImageSource for Telemed API devices.
  * The method mitk::USImageSource::GetNextRawImage() is implemented for
  * getting images from the Telemed API.
  *
  * A method for connecting this ImageSource to the Telemed API is
  * implemented (mitk::USTelemedImageSource::CreateAndConnectConverterPlugin()).
  * This method is available for being used by mitk::USTelemedDevice.
  */
class USTelemedImageSource : public USImageSource
{
public:
  mitkClassMacro(USTelemedImageSource, USImageSource);
  itkNewMacro(Self);

  /**
    * Implementation of the superclass method. Returns the pointer
    * to the mitk::Image filled by Telemed API callback.
    */
  virtual void GetNextRawImage( mitk::Image::Pointer& );

  /**
    * \brief Connect this object to the Telemed API.
    * This method is for being used by mitk::USTelemedDevice.
    */
  bool CreateAndConnectConverterPlugin( Usgfw2Lib::IUsgDataView*, Usgfw2Lib::tagScanMode );

protected:
  USTelemedImageSource( );
  virtual ~USTelemedImageSource( );

  Usgfw2Lib::IUsgScanConverterPlugin*         m_Plugin;
  USTelemedScanConverterPlugin*               m_PluginCallback;

  mitk::Image::Pointer                        m_Image;
};
} // namespace mitk

#endif // MITKUSTelemedImageSource_H