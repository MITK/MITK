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

#ifndef QMITKTRACKINGDEVICECONFIGURATIONWIDGET_H
#define QMITKTRACKINGDEVICECONFIGURATIONWIDGET_H

#include <QWidget>
#include <QThread>
#include "MitkIGTUIExports.h"
#include "ui_QmitkTrackingDeviceConfigurationWidgetControls.h"
#include "mitkTrackingDevice.h"
#include <mitkIPersistenceService.h>

class QmitkTrackingDeviceConfigurationWidgetConnectionWorker;
class QmitkTrackingDeviceConfigurationWidgetScanPortsWorker;

//itk headers

 /** Documentation:
  *   \brief An object of this class offers an UI to configurate
  *          a tracking device. If the user finished the configuration process and
  *          a fully configurated tracking device is availiabe the object emits a
  *          signal "TrackingDeviceConfigurationFinished()". You can then get the
  *          tracking device by calling the method GetTrackingDevice().
  *
  *          Once the tracking device is configurated there are two ways to reset
  *          the UI to allow the user for configuring a new device. The method Reset()
  *          can be called and there is also a button "reset" which can be pressed by
  *          the user. In both cases a signal "TrackingDeviceConfigurationReseted()"
  *          is emitted and you may wait for a new configurated tracking device.
  *
  *          The possibility to reset the configuration by the user can also be switched
  *          of by calling the method EnableUserReset(boolean enable).
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkTrackingDeviceConfigurationWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkTrackingDeviceConfigurationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkTrackingDeviceConfigurationWidget();

    /* @return Returns the current configurated tracking device. If the user didn't finished the
     *         configuration process or if there is an error during configuration NULL is returned.
     */
    mitk::TrackingDevice::Pointer GetTrackingDevice();

    enum Style
      {
      SIMPLE,
      ADVANCED,
      };

    /* @brief Resets the UI to allow the user for configurating a new tracking device.
     */
    void Reset();

    /** @brief External call to disable this widget when configuration is finished. This is also called by the "finished" button,
      *        but if you disable the advanced user control you might want to call this when the configuration is finished.
      *        If you want to configure a new device call the Reset() funktion later.
      */
    void ConfigurationFinished();

    /* @brief Sets our unsets the possibility to reset the UI and start
     *        a new configuration by the user. Concretely this means the
     *        button "reset" is shown or not.
     */
    void EnableUserReset(bool enable);

    /** @return Returns true if the tracking device is completely configured (you can get it by calling GetTrackingDevice() in this case).
     *          Returns false if configuration is not finished.
     */
    bool GetTrackingDeviceConfigured();

    /** @brief Sets the style of this widget. Default is ADVANCED. Caution: The style can only be set once at startup! */
    void SetGUIStyle(Style style);

    /** @brief Enables/disables the advanced user controls which means the reset and finished button. When disabled you'll get NO
      *        signals from this widget and you've to check by yourself if the configuration is finished. Default value is false.
      *        Advanced user control is only availiable when style is ADVANCED.
      */
    void EnableAdvancedUserControl(bool enable);


  signals:

    /* @brief This signal is sent if the user has finished the configuration of the tracking device.
     *        The device is now availiable if the method GetTrackingDevice() is called. The tracking
     *        device you'll get is completly configurated but no tools are added yet.
     */
    void TrackingDeviceConfigurationFinished();

    /* @brief This signal is sent if the UI was reseted and the user is required to configurate
     *        a new tracking device.
     */
    void TrackingDeviceConfigurationReseted();

    /* @brief This signal is sent if the tracking device was changed. */
    void TrackingDeviceSelectionChanged();

    /* @brief This signal is sent if an internal thread (caused by an action of the user) was started. The widget is blocked until the progress ends.*/
    void ProgressStarted();

    /* @brief This signal is sent if a progress which was started before ends.*/
    void ProgressFinished();

  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkTrackingDeviceConfigurationWidgetControls* m_Controls;

    std::stringstream m_output;

    mitk::TrackingDevice::Pointer m_TrackingDevice;

    std::string m_MTCalibrationFile;

    std::string m_OptitrackCalibrationFile;

    bool m_TrackingDeviceConfigurated;

    bool m_AdvancedUserControl;

    // key is port name (e.g. "COM1", "/dev/ttyS0"), value will be filled with the type of tracking device at this port
    typedef QMap<QString, mitk::TrackingDeviceType> PortDeviceMap;

    QmitkTrackingDeviceConfigurationWidgetScanPortsWorker* m_ScanPortsWorker;
    QmitkTrackingDeviceConfigurationWidgetConnectionWorker* m_TestConnectionWorker;
    QThread* m_ScanPortsWorkerThread;
    QThread* m_TestConnectionWorkerThread;

    //######################### internal help methods #######################################
    void ResetOutput();
    void AddOutput(std::string s);
    mitk::TrackingDevice::Pointer ConstructTrackingDevice();

    void StoreUISettings();
    void LoadUISettings();


  protected slots:
    /* @brief This method is called when the user changes the selection of the trackingdevice (m_trackingDeviceChooser).
              It then sets the correct widget for the selected tracking device.*/
    void TrackingDeviceChanged();

    /* @brief This method is called when the user presses the button "test connection". The method will then create a temporary tracking device,
     *        try to open a connection and start tracking. The user can see the result of the connection test on the small output window.
     */
    void TestConnection();

    void TestConnectionFinished(bool connected, QString output);

    /* @brief This method is called when the user presses the button "finished". A new tracking device will be created in this case and will then
     *        then be availiable by calling GetTrackingDevice(). Also a signal TrackingDeviceConfigurationFinished() will be emitted. After this the
     *        UI will be disablet until the widget is reseted to configure a new tracking device.
     */
    void Finished();

    /* @brief This method is called when the user presses the button "reset". He can configure a new tracking device then. The signal
     *        TrackingDeviceConfigurationReseted() will be emitted if this method is called. The method GetTrackingDevice() will return
     *        NULL until a new tracking device is configured.
     */
    void ResetByUser();

    /* @return Returns a configured NDI 5D tracking device. Unfortunately the NDI 5D tracking device is not yet in the open source part
     *        so this method only returns NULL at the moment.
     */
    virtual mitk::TrackingDevice::Pointer ConfigureNDI5DTrackingDevice();

    /* @return Returns a configured NDI 6D tracking device.
     *         The type (which means Aurora/Polaris) will not be set in the returnvalue. You have to this later.
     */
    mitk::TrackingDevice::Pointer ConfigureNDI6DTrackingDevice();

    /* @return Returns a configured Optitrack tracking device.
     *         The type (which means Aurora/Polaris/Optitrack) will not be set in the returnvalue. You have to this later.
     */
    mitk::TrackingDevice::Pointer ConfigureOptitrackTrackingDevice();

    /* @brief Scans the serial ports automatically for a connected tracking device. If the method finds a device
     *        it selects the right type and sets the corresponding port in the widget.
     */
    void AutoScanPorts();

    /** This slot is called when the port scanning is finished. */
    void AutoScanPortsFinished(int PolarisPort, int AuroraPort, QString result, int PortTypePolaris, int PortTypeAurora);

    /* @brief Opens a file dialog. The users sets the calibration file which location is then stored in the member m_MTCalibrationFile.*/
    void SetMTCalibrationFileClicked();

    /* @brief Opens a file dialog. The users sets the calibration file which location is then stored in the member m_OptitrackCalibrationFile.*/
    void SetOptitrackCalibrationFileClicked();

  private:
    PERSISTENCE_GET_SERVICE_METHOD_MACRO
};

//###################################################################################################
//############ PRIVATE WORKER CLASSES FOR THREADS ###################################################
//###################################################################################################


/**
 * Worker thread class for test connection.
 */
class QmitkTrackingDeviceConfigurationWidgetConnectionWorker : public QObject
{
  Q_OBJECT

  public:

  void SetTrackingDevice(mitk::TrackingDevice::Pointer t);

  public slots:

    void TestConnectionThreadFunc();
  signals:

    void ConnectionTested(bool connected, QString output);

  protected:
    mitk::TrackingDevice::Pointer m_TrackingDevice;
};

/**
 * Worker thread class for scan ports.
 */
class QmitkTrackingDeviceConfigurationWidgetScanPortsWorker : public QObject
{
  Q_OBJECT

  public slots:
    void ScanPortsThreadFunc();

  signals:

   /**
     * @param PolarisPort Returns the port, returns -1 if no device was found.
     * @param AuroraPort Returns the port, returns -1 if no device was found.
     * @param PortTypePolaris Returns the port type (0=usb,1=tty), returns -1 if the port type is not specified, e.g, in case of Windows.
     * @param PortTypeAurora Returns the port type (0=usb,1=tty), returns -1 if the port type is not specified, e.g, in case of Windows.
     */
    void PortsScanned(int PolarisPort, int AuroraPort, QString result, int PortTypePolaris, int PortTypeAurora);

  protected:

    /** @brief   Scans the given port for a NDI tracking device.
      * @return  Returns the type of the device if one was found. Returns TrackingSystemInvalid if none was found.
      */
    mitk::TrackingDeviceType ScanPort(QString port);
};
#endif
