/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNDIConfigurationWidget_h
#define QmitkNDIConfigurationWidget_h

#include "ui_QmitkNDIConfigurationWidget.h"
#include "mitkNDITrackingDevice.h"
#include "mitkTrackingDeviceSource.h"
#include "QStringList"
#include "MitkIGTUIExports.h"
#include "mitkNodePredicateBase.h"

#include "mitkNavigationTool.h"

class QmitkNDIToolDelegate;
namespace mitk
{
  class DataStorage;
};

/**@deprecated This widget is deprecated. The features (1) connection to NDI tracking devices and
  *            (2) handling of navigation tools are available in the pluging org.mitk.gui.qt.igttracking
  *            in a current version. The new concept to access the tracking devices is to use microservices.
  *            This can be achieved very simple by using the QmitkNavigationDataSourceSelectionWidget. You
  *            can find an example in the IGT tutorial step 2 / org.mitk.gui.qt.igtexamples (view TrackingLab).
  *
  *\ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkNDIConfigurationWidget : public QWidget
{
  Q_OBJECT // this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
public:
  QmitkNDIConfigurationWidget(QWidget* parent);
  ~QmitkNDIConfigurationWidget() override;

  DEPRECATED(std::string GetDeviceName() const);
  DEPRECATED(mitk::NDITrackingDevice* GetTracker() const);
  DEPRECATED(mitk::DataStorage* GetDataStorage() const);
  DEPRECATED(mitk::NodePredicateBase* GetPredicate() const);
  DEPRECATED(const QStringList& GetToolTypes() const);
  DEPRECATED(void SetToolTypes(const QStringList& types));       ///< set types list for type editor combobox
  DEPRECATED(void SetDataStorage(mitk::DataStorage* ds));    ///< set datastorage for organ node editor
  DEPRECATED(void SetPredicate(mitk::NodePredicateBase::Pointer p)); ///< set predicate for organ node editor
  DEPRECATED(void SetTagPropertyName(const std::string& name));      ///< set name of the property that is used to tag selected nodes
  DEPRECATED(void SetTagProperty(mitk::BaseProperty::Pointer prop));   ///< set the property that is used to tag selected nodes

  DEPRECATED(const QString GetToolType(unsigned int index) const);
  DEPRECATED(const QString GetToolName(unsigned int index) const);
  QMap<QString, unsigned int> GetToolAndTypes() const;
  DEPRECATED(QList<unsigned int> GetToolsByToolType(QString toolType) const);
  DEPRECATED(mitk::DataNode* GetNode(unsigned int index) const);


  signals:
    void ToolsAdded(QStringList tools);
    void ToolsChanged();
    void Connected();
    void Disconnected();
    void RepresentationChanged( int row , mitk::Surface::Pointer surface ); // returns the row number of the clicked tableitem for changing tool representation
    void SignalToolNameChanged(int id, QString name);
    void SignalSavedTool(int id, QString surfaceFilename);
    void SignalLoadTool(int id, mitk::DataNode::Pointer dn);

  public slots:
    void SetDeviceName(const char* dev);  ///< set the device name (e.g. "COM1", "/dev/ttyS0") that will be used to connect to the tracking device
    void ShowToolRepresentationColumn(); ///< show or hide the tooltable column "Tool Representation". This SLOT should be called after SIGNAL "Connected" is emitted
    void EnableAddToolsButton(bool enable); ///< enables or disables the Add Tools button
    void EnableDiscoverNewToolsButton(bool enable); ; ///< enables or disables the Discover new Tools button

  protected slots:
    void OnConnect();
    void OnDisconnect();
    void OnDiscoverTools();
    void OnDiscoverDevices();
    void OnAddPassiveTool();
    void UpdateTrackerFromToolTable(const QModelIndex & topLeft, const QModelIndex & /*bottomRight*/);
    void OnTableItemClicked(const QModelIndex & topLeft); ///< for clicking on tooltable items
    void OnDisoverDevicesBtnInfo();
    void OnTableCellChanged(int row, int column);
    void OnSaveTool();
    void OnLoadTool();


protected:
  typedef QMap<QString, mitk::TrackingDeviceType> PortDeviceMap;  // key is port name (e.g. "COM1", "/dev/ttyS0"), value will be filled with the type of tracking device at this port
  /**Documentation
  * \brief scans the ports provided as key in the portsAndDevices and fills the respective value of portsAndDevices with the tracking device type at that port
  *
  *
  * \param[in,out] portsAndDevices keys are used to query serial ports, values of the existing keys will be filled with the tracking device type
  */
  void ScanPortsForNDITrackingDevices(PortDeviceMap& portsAndDevices);
  mitk::TrackingDeviceType ScanPort(QString port);
  mitk::NavigationTool::Pointer GenerateNavigationTool(mitk::TrackingTool* tool);

  QStringList GetToolNamesList(); ///< returns a string list with the names of all tools of the current tracking device

  void CreateTracker();  ///< creates new NDITrackingDevice object
  void SetupTracker();   ///< sets the parameters from the gui to the tracking device object
  QString GetStatusText(); ///< construct a status text depending on the current state of the tracking device object

  void UpdateWidgets();
  void UpdateToolTable(); ///< read all tools from the tracking device object and display them in the gui
  virtual void CreateQtPartControl(QWidget *parent);
  virtual void CreateConnections(); ///< \brief Creation of the connections of main and control widget
  void HidePolarisOptionsGroupbox( bool on ); ///< show or hide polaris options in the UI
  void HideAuroraOptionsGroupbox( bool on ); ///< show or hide aurora options in the UI

  Ui::QmitkNDIConfigurationWidget* m_Controls;  ///< gui widgets
  mitk::NDITrackingDevice::Pointer m_Tracker;   ///< tracking device object
  mitk::TrackingDeviceSource::Pointer m_Source;
  QmitkNDIToolDelegate* m_Delegate;

  QString m_SROMCellDefaultText;
  QString m_RepresentatonCellDefaultText;

  mitk::Surface::Pointer LoadSurfaceFromSTLFile(QString surfaceFilename);

};
#endif
