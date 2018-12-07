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

#ifndef MITKUSVideoDevice_H_HEADER_INCLUDED_
#define MITKUSVideoDevice_H_HEADER_INCLUDED_

#include <MitkUSExports.h>
#include <mitkCommon.h>
#include "mitkUSDevice.h"
#include "mitkUSImageVideoSource.h"
#include "mitkUSProbe.h"
#include <itkObjectFactory.h>

namespace itk {
  template<class T> class SmartPointer;
}

namespace mitk {
  class USVideoDeviceCustomControls;
  class USAbstractControlInterface;

  /**
  * \brief A mitk::USVideoDevice is the common class for video only devices.
  * They capture video input either from a file or from a device and
  * transform the output into an mitk::USImage with attached metadata.
  * This simple implementation does only capture and display 2d images without
  * registration for example.
  *
  * \ingroup US
  */
  class MITKUS_EXPORT USVideoDevice : public mitk::USDevice
  {
  public:
    mitkClassMacro(USVideoDevice, mitk::USDevice);
    // To open a device (DeviceID, Manufacturer, Model)
    mitkNewMacro3Param(Self, int, std::string, std::string);
    // To open A VideoFile (Path, Manufacturer, Model)
    mitkNewMacro3Param(Self, std::string, std::string, std::string);
    // To open a device (DeviceID, Metadata)
    mitkNewMacro2Param(Self, int, mitk::USImageMetadata::Pointer);
    // To open A VideoFile (Path, Metadata)
    mitkNewMacro2Param(Self, std::string, mitk::USImageMetadata::Pointer);

    /**
    * \return the qualified name of this class (as returned by GetDeviceClassStatic())
    */
    std::string GetDeviceClass() override;

    /**
     * This methode is necessary instead of a static member attribute to avoid
     * "static initialization order fiasco" when an instance of this class is
     * used in a module activator.
     *
     * \return the qualified name of this class
     */
    static std::string GetDeviceClassStatic();

    /**
    * Getter for the custom control interface which was created during the
    * construction process of mitk::USVideoDevice.
    *
    * \return custom control interface of the video device
    */
    itk::SmartPointer<USAbstractControlInterface> GetControlInterfaceCustom() override;

    /**
    * \brief Remove this device from the micro service.
    * This method is public for mitk::USVideoDevice, because this devices
    * can be completly removed. This is not possible for API devices, which
    * should be available while their sub module is loaded.
    */
    void UnregisterOnService();

    /**
    * \return mitk::USImageSource connected to this device
    */
    USImageSource::Pointer GetUSImageSource() override;

    /**
    * \brief Return all probes for this USVideoDevice or an empty vector it no probes were set
    * Returns a std::vector of all probes that exist for this USVideoDevice if there were probes set while creating or modifying this USVideoDevice.
    * Otherwise it returns an empty vector. Therefore always check if vector is filled, before using it!
    */
    virtual std::vector<mitk::USProbe::Pointer> GetAllProbes() override;

    /**
    * \brief Cleans the std::vector containing all configured probes.
    */
    virtual void DeleteAllProbes() override;

    /**
    * \brief Return current active probe for this USVideoDevice
    * Returns a pointer to the probe that is currently in use. If there were probes set while creating or modifying this USVideoDevice.
    * Returns null otherwise
    */
    virtual mitk::USProbe::Pointer GetCurrentProbe() override;

    /**
    \brief adds a new probe to the device
    */
    virtual void AddNewProbe(mitk::USProbe::Pointer probe) override;

    /**
    * \brief get the probe by its name
    * Returns a  pointer to the probe identified by the given name. If no probe of given name exists for this Device 0 is returned.
    */
    virtual mitk::USProbe::Pointer GetProbeByName(std::string name) override;

    /**
    * \brief Removes the Probe with the given name
    */
    virtual void RemoveProbeByName(std::string name) override;

    /**
    \brief True, if this Device plays back a file, false if it recieves data from a device
    */
    bool GetIsSourceFile();

    /**
    * \brief Sets the first existing probe or the default probe of the video device
    * as the current probe of it.
    */
    virtual  void SetDefaultProbeAsCurrentProbe() override;

    /**
    * \brief Sets the probe with the given name as current probe if the named probe exists.
    */
    virtual void SetCurrentProbe( std::string probename ) override;

    /**
    * \brief Sets the given spacing of the current depth of the current probe.
    */
    void SetSpacing( double xSpacing, double ySpacing ) override;

    itkGetMacro(ImageVector, std::vector<mitk::Image::Pointer>);
    itkGetMacro(DeviceID, int);
    itkGetMacro(FilePath, std::string);
  protected:

    /**
    * \brief Creates a new device that will deliver USImages taken from a video device.
    *  under windows, try -1 for device number, which will grab the first available one
    * (Open CV functionality)
    */
    USVideoDevice(int videoDeviceNumber, std::string manufacturer, std::string model);
    /**
    * \brief Creates a new device that will deliver USImages taken from a video file.
    */
    USVideoDevice(std::string videoFilePath, std::string manufacturer, std::string model);
    /**
    * \brief Creates a new device that will deliver USImages taken from a video device.
    *  under windows, try -1 for device number, which will grab the first available one
    * (Open CV functionality)
    */
    USVideoDevice(int videoDeviceNumber, mitk::USImageMetadata::Pointer metadata);
    /**
    * \brief Creates a new device that will deliver USImages taken from a video file.
    */
    USVideoDevice(std::string videoFilePath, mitk::USImageMetadata::Pointer metadata);

    ~USVideoDevice() override;

    /**
    * \brief Initializes common properties for all constructors.
    */
    void Init();

    /**
    * \brief Is called during the initialization process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    bool OnInitialization() override;

    /**
    * \brief Is called during the connection process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    bool OnConnection() override;

    /**
    * \brief Is called during the disconnection process.
    *  Returns true if successful and false if unsuccessful. Additionally, you may throw an exception to clarify what went wrong.
    */
    bool OnDisconnection() override;

    /**
    * \brief Is called during the activation process. After this method is finsihed, the device should be generating images
    */
    bool OnActivation() override;

    /**
    * \brief Is called during the deactivation process. After a call to this method the device should still be connected, but not producing images anymore.
    */
    bool OnDeactivation() override;

    /**
    * \brief Grabs the next frame from the Video input.
    * This method is called internally, whenever Update() is invoked by an Output.
    */
    virtual void GenerateData() override;

    /**
    * \brief The image source that we use to aquire data
    */
    mitk::USImageVideoSource::Pointer m_Source;

    /**
    * \brief True, if this source plays back a file, false if it recieves data from a device
    */
    bool m_SourceIsFile;

    /**
    * \brief The device id to connect to. Undefined, if m_SourceIsFile == true;
    */
    int m_DeviceID;

    /**
    * \brief The Filepath id to connect to. Undefined, if m_SourceIsFile == false;
    */
    std::string m_FilePath;

    /**
    * \brief custom control interface for us video device
    */
    itk::SmartPointer<USVideoDeviceCustomControls> m_ControlInterfaceCustom;

    /**
    * \brief probes for this USVideoDevice
    */
    std::vector < mitk::USProbe::Pointer > m_Probes;

    /**
    \brief probe that is currently in use
    */
    mitk::USProbe::Pointer m_CurrentProbe;
  };
} // namespace mitk

#endif // MITKUSVideoDevice_H_HEADER_INCLUDED_
