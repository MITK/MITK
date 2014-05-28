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

#ifndef MITKUSIMAGEMETADATA_H_HEADER_INCLUDED_
#define MITKUSIMAGEMETADATA_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include <itkObject.h>
#include <itkObjectFactory.h>

namespace mitk {
  /**Documentation
  * \brief This class encapsulates all necessary metadata to describe a US Image.
  * \ingroup US
  *
  * \deprecated Deprecated since 2014-06
  */
  class MitkUS_EXPORT USImageMetadata : public itk::Object
  {
  public:
    mitkClassMacro(USImageMetadata, itk::Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    //## getter and setter ##
    itkGetMacro(DeviceManufacturer, std::string);
    itkSetMacro(DeviceManufacturer, std::string);
    itkGetMacro(DeviceModel, std::string);
    itkSetMacro(DeviceModel, std::string);
    itkGetMacro(DeviceComment, std::string);
    itkSetMacro(DeviceComment, std::string);
    itkGetMacro(ProbeName,  std::string);
    itkSetMacro(ProbeName,  std::string);
    itkGetMacro(ProbeFrequency,  std::string);
    itkSetMacro(ProbeFrequency,  std::string);
    itkGetMacro(Zoom,  std::string);
    itkSetMacro(Zoom,  std::string);
    itkGetMacro(DeviceIsVideoOnly, bool);
    itkSetMacro(DeviceIsVideoOnly, bool);
    itkGetMacro(DeviceIsCalibrated, bool);
    itkSetMacro(DeviceIsCalibrated, bool);
    itkGetMacro(DeviceClass, std::string);
    itkSetMacro(DeviceClass, std::string);

    // The following constants define how metadata is written to and read from an mitk image
    // when defining new properties, add them here, define them in the cpp, and add them to
    // USImage's getMetadata and setMetadata methods as well
    static const char* PROP_DEV_MANUFACTURER;
    static const char* PROP_DEV_MODEL;
    static const char* PROP_DEV_COMMENT;
    static const char* PROP_DEV_ISVIDEOONLY;
    static const char* PROP_DEV_ISCALIBRATED;
    static const char* PROP_PROBE_NAME;
    static const char* PROP_PROBE_FREQUENCY;
    static const char* PROP_ZOOM;
    static const char* PROP_DEVICE_CLASS;

  protected:
    /**
    * \brief Creates a new metadata object with all fields set to default values.
    */
    USImageMetadata();
    virtual ~USImageMetadata();

    std::string m_DeviceManufacturer;
    std::string m_DeviceModel;
    std::string m_DeviceComment;
    std::string m_ProbeName;
    std::string m_ProbeFrequency;
    std::string m_Zoom;
    std::string m_DeviceClass;
    bool m_DeviceIsVideoOnly;
    bool m_DeviceIsCalibrated;
  };
} // namespace mitk
#endif
