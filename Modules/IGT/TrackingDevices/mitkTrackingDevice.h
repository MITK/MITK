/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef MITKTRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2
#define MITKTRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2

#include <MitkIGTExports.h>
#include "itkObject.h"
#include "mitkCommon.h"
#include "mitkTrackingTypes.h"
#include "itkFastMutexLock.h"
#include "mitkNavigationToolStorage.h"


namespace mitk {
    class TrackingTool; // interface for a tool that can be tracked by the TrackingDevice

    /**Documentation
    * \brief Interface for all Tracking Devices
    *
    * Defines the methods that are common for all tracking devices.
    *
    *  \ingroup IGT
    */
    class MITKIGT_EXPORT TrackingDevice : public itk::Object
    {
    public:
      mitkClassMacroItkParent(TrackingDevice, itk::Object);

      /** Defines the rotation modes of this tracking device which results in different representations
       *  of quaternions.
       *
       *  - Standard: normal representation, rawdata from the device is not changed (DEFAULT)
       *
       *  - Transposed: the rotation is stored transposed, which is (by mistake!) expected by some older MITK classes due
       *                to an ambigious method naming in VNL.
       *
       *  CAUTION: The rotation mode can only be changed for backward compatibility of old WRONG code.
       *           PLEASE DO NOT CHANGE THE ROTATION MODE UNLESS YOU ARE KNOWING EXACTLY WHAT YOU ARE DOING!
       *
       *  use SetRotationMode to change the mode.
       */
      enum RotationMode {RotationStandard, RotationTransposed};

      enum TrackingDeviceState {Setup, Ready, Tracking};   ///< Type for state variable. The trackingdevice is always in one of these states
      /**
       * \brief Opens a connection to the device
       *
       * This may only be called if there is currently no connection to the device.
       * If OpenConnection() is successful, the object will change from Setup state to Ready state
       */
      virtual bool OpenConnection() = 0;

      /**
       * \brief Closes the connection to the device
       *
       * This may only be called if there is currently a connection to the device, but tracking is
       * not running (e.g. object is in Ready state)
       */
      virtual bool CloseConnection() = 0; ///< Closes the connection with the device

      /**
       * \brief start retrieving tracking data from the device.
       *
       * This may only be called after the connection to the device has been established
       * with a call to OpenConnection() (E.g. object is in Ready mode). This will change the
       * object state from Ready to Tracking
       */
      virtual bool StartTracking() = 0;

      /**
       * \brief stop retrieving tracking data from the device.
       * stop retrieving tracking data from the device.
       * This may only be called after StartTracking was called
       * (e.g. the object is in Tracking mode).
       * This will change the object state from Tracking to Ready.
       */
      virtual bool StopTracking();

      /**
       * \brief Return tool with index toolNumber
       *
       * tools are numbered from 0 to GetToolCount() - 1.
       */
      virtual TrackingTool* GetTool(unsigned int toolNumber) const = 0;

      /**
      * \brief Returns the tool with the given tool name
      *
      * Note: subclasses can and should implement optimized versions of this method
      * \return the given tool or nullptr if no tool with that name exists
      */
      virtual mitk::TrackingTool* GetToolByName(std::string name) const;

      /**
       * \brief Returns number of tracking tools
       */
      virtual unsigned int GetToolCount() const = 0;

      /** Sets the rotation mode of this class. See documentation of enum RotationMode for details
       *  on the different modes. This method has to be implemented in a deriving class to become
       *  functional / if different rotation modes should be supported.
       *  CAUTION: The rotation mode can only be changed for backward compatibility of old WRONG code.
       *           PLEASE DO NOT CHANGE THE ROTATION MODE UNLESS YOU ARE KNOWING EXACTLY WHAT YOU ARE DOING!
       */
      virtual void SetRotationMode(RotationMode r);

     /** @return Returns the rotation mode of this class. See documentation of enum
       *         RotationMode for details on the different modes.
       */
      itkGetConstMacro(RotationMode,RotationMode);

      /**
       * \brief return current object state (Setup, Ready or Tracking)
       */
      TrackingDeviceState GetState() const;

      /**
       * \brief Deprecated! Use the more specific getData or GetTrackingDeviceName instead. return device type identifier
       */
    TrackingDeviceType GetType() const;
       /**
       * \brief Deprecated! Use the more specific setDeviceData instead. set device type
       */
    void SetType(TrackingDeviceType type);

    /**
    * \brief Convenient Method to get the Name of the Tracking Device.
    * This is identical with GetData().Line and can be used to compare with TrackingDeviceTypeInformation::GetTrackingDeviceName()
    * to check if you have a specific device.
    */
    std::string GetTrackingDeviceName();

     /**
       * \brief return device data
     */
    TrackingDeviceData GetData() const;
       /**
       * \brief set device type
       */
    void SetData(TrackingDeviceData data);

    /**
     * @return Returns true if the device is installed on this system an can be used.
     *         Installed means activated in MITK, in some cases this means the MITK
     *         installation / build has to know the installation path of the device
     *         libraries on this system. This path is usually given as cmake variable
     *         during the build configuration in devellopers mode. If the device should
     *         be available for end users with an installer the libraries can be included
     *         into the installer or the installer has to be adapted such that it asks
     *         for the path.
     *         Returns fals if the device is not installed. It cannot be used on this build
     *         in this case.
     *
     *         Note that some tracking systems communicate via a standard interface (e.g., serial
     *         port) and don't need any library or installation. These devices are always "installed".
     */
    virtual bool IsDeviceInstalled();

    /** @return Returns true if this device can autodetects its tools. */
    virtual bool AutoDetectToolsAvailable();

    /** @return Returns true if it is possible to add a single tool. Default return is true.*/
    virtual bool AddSingleToolIsAvailable();

    /** Autodetects tools from this device and returns them as a navigation tool storage.
     *  @return Returns the detected tools. Returns an empty storage if no tools are present
     *          or if detection is not possible
     */
    virtual mitk::NavigationToolStorage::Pointer AutoDetectTools();

    private:
      TrackingDeviceState m_State; ///< current object state (Setup, Ready or Tracking)
    protected:

      /**
      * \brief  change object state
      */
      void SetState(TrackingDeviceState state);


      TrackingDevice();
      ~TrackingDevice() override;

    TrackingDeviceData m_Data; ///< current device Data

      bool m_StopTracking;       ///< signal stop to tracking thread
      itk::FastMutexLock::Pointer m_StopTrackingMutex; ///< mutex to control access to m_StopTracking
      itk::FastMutexLock::Pointer m_TrackingFinishedMutex; ///< mutex to manage control flow of StopTracking()
      itk::FastMutexLock::Pointer m_StateMutex; ///< mutex to control access to m_State
      RotationMode m_RotationMode; ///< defines the rotation mode Standard or Transposed, Standard is default
    };
} // namespace mitk

#endif /* MITKTRACKINGDEVICE_H_HEADER_INCLUDED_C1C2FCD2 */
