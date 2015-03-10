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
#ifndef __mitkToFImageSource_h
#define __mitkToFImageSource_h

#include <MitkToFHardwareExports.h>
#include <mitkImageSource.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

//MicroServices
#include <usServiceInterface.h>
#include <usServiceRegistration.h>

namespace mitk
{
  /**Documentation
  * \brief Image source providing ToF images. Interface for filters provided in ToFProcessing module
  *
  * This class internally holds a ToFCameraDevice to access the images acquired by a ToF camera.
  *
  * Provided images include: distance image (output 0), amplitude image (output 1), intensity image (output 2) and color image (output 3)
  *
  * \ingroup ToFHardware
  */
  class MITKTOFHARDWARE_EXPORT ToFImageSource : public mitk::ImageSource
  {
  public:

    mitkClassMacro( ToFImageSource, ImageSource );

    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

  protected:

    ToFImageSource();

    ~ToFImageSource();

    us::ServiceRegistration<Self> m_ServiceRegistration;
  private:

  };
} //END mitk namespace

/**
ToFCameraDevice is declared a MicroService interface. See
MicroService documenation for more details.
*/
US_DECLARE_SERVICE_INTERFACE(mitk::ToFImageSource, "org.mitk.services.ToFImageSource")
#endif
