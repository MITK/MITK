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
#ifndef __mitkToFCameraMESASR4000Device_h
#define __mitkToFCameraMESASR4000Device_h

#include "mitkMESASR4000ModuleExports.h"
#include "mitkCommon.h"
#include "mitkToFCameraDevice.h"
#include "mitkToFCameraMESADevice.h"
#include "mitkToFCameraMESASR4000Controller.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"


namespace mitk
{
  /**
  * @brief Device class representing a MESA CamBoard camera
  *
  * @ingroup ToFHardware
  */
  class MITK_MESASR4000MODULE_EXPORT ToFCameraMESASR4000Device : public ToFCameraMESADevice
  {
  public:

    mitkClassMacro( ToFCameraMESASR4000Device , ToFCameraMESADevice );

    itkNewMacro( Self );

    /*!
    \brief set a BaseProperty
    */
    virtual void SetProperty( const char *propertyKey, BaseProperty* propertyValue );
    /*!
    \brief Setting the region of interest, the camera is configured to only output a certain area of the image.
    \param leftUpperCornerX x value of left upper corner of region
    \param leftUpperCornerX y value of left upper corner of region
    \param width width of region
    \param height height of region
    */
    void SetRegionOfInterest( unsigned int leftUpperCornerX, unsigned int leftUpperCornerY, unsigned int width, unsigned int height );

  protected:

    ToFCameraMESASR4000Device();

    ~ToFCameraMESASR4000Device();


  private:

  };
} //END mitk namespace
#endif
