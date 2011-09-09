/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision:  $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkToFCameraPMDRawDataCamBoardDevice_h
#define __mitkToFCameraPMDRawDataCamBoardDevice_h

#include "mitkToFHardwareExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraPMDRawDataDevice.h"

namespace mitk
{
  /**
  * @brief Device class representing a PMD CamCube camera
  *
  *
  * @ingroup ToFHardwareMBI
  */
  class MITK_TOFHARDWARE_EXPORT ToFCameraPMDRawDataCamBoardDevice : public ToFCameraPMDRawDataDevice
  {
  public: 

    mitkClassMacro( ToFCameraPMDRawDataCamBoardDevice , ToFCameraPMDRawDataDevice );

    itkNewMacro( Self );
    
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );

    virtual void GetChannelSourceData(short* sourceData, vtkShortArray* vtkChannelArray );


  protected:

    ToFCameraPMDRawDataCamBoardDevice();

    ~ToFCameraPMDRawDataCamBoardDevice();
  private:

  };
} //END mitk namespace
#endif // __mitkToFCameraPMDRawDataCamBoardDevice_h
