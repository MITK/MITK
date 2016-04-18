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

#ifndef USNavigation_h
#define USNavigation_h

// Blueberry, QMITK
#include <berryISelectionListener.h>
#include <QmitkAbstractView.h>
#include "ui_USNavigationControls.h"

// QT
#include <QTimer>
#include <QProgressBar>

// MITK
#include <mitkNavigationDataSource.h>
#include <mitkNavigationToolStorage.h>
#include <mitkUSCombinedModality.h>
#include <mitkNodeDisplacementFilter.h>
#include <mitkNeedleProjectionFilter.h>
#include <mitkNavigationDataSmoothingFilter.h>
#include <mitkCameraVisualization.h>

/*!
  \brief USNavigation

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class USNavigation : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    USNavigation();
    virtual void CreateQtPartControl(QWidget *parent);

  protected slots:

    /**
    * \brief Triggered, when the user selects a device from either the list of USDevices or Tracking Devices
    */
    //void OnClickDevices();
    void OnDeviceSelected();
    void OnDeviceDeselected();

    /**
    * \brief Triggered, when the user has clicked "select Devices".
    */
    void OnSelectDevices();

    /**
    * \brief Triggered, when the user has clicked "Load Calibration".
    */
    //void OnLoadCalibration();

    /**
    * \brief
    */
    void OnStartIntervention();

    /**
    * \brief
    */
    void OnFreeze();

    /**
    * \brief
    */
    void OnReset();

    /**
    * \brief
    */
    void OnNeedleViewToggle();

    void OnZoneAdded();

    /**
    * \brief
    */
    void Update();

    void UpdateMeters();

  protected:

    mitk::NavigationToolStorage::Pointer m_ToolStorage;
    std::vector<mitk::DataNode::Pointer> m_Zones;
    mitk::DataNode::Pointer m_USStream;

    bool m_Freeze;

    QString m_RangeMeterStyle;

    /**
    * \brief The CombinedModality used for imaging.
    */
    mitk::USCombinedModality::Pointer m_USDevice;

    /**
    * \brief Temporary saving point for loaded calibration data.
    */
    std::string m_LoadedCalibration;

    /**
    * \brief NavigationDataSource used for tracking data.
    */
    mitk::NavigationDataSource::Pointer m_Tracker;

    mitk::NodeDisplacementFilter::Pointer m_ZoneFilter;

    mitk::NavigationDataSmoothingFilter::Pointer m_SmoothingFilter;

    mitk::CameraVisualization::Pointer m_CameraVis;

    /**
    * \brief Creates a Pointset that projects the needle's path
    */
    mitk::NeedleProjectionFilter::Pointer m_NeedleProjectionFilter;

    /**
    * \brief Colors the given node according to the color selected in the "Edit Zone" section, names it correctly and adjusts it's size.
    */
    void FormatZoneNode(mitk::DataNode::Pointer node, mitk::Point3D center);

    /**
    * \brief Sets up the RangeMeter Section of the Navigation by creating and initializing all necessary Progress Bars.
    */
    void SetupProximityView();

    QProgressBar* CreateRangeMeter(int i);
    void ResetRangeMeters();

    std::vector< QProgressBar* > m_RangeMeters;

    virtual void SetFocus();

    QTimer *m_Timer;
    QTimer *m_RangeMeterTimer;

    bool m_UpdateImage;

    bool m_IsNeedleViewActive;

    static const int MAXRANGE = 60;
    static const int WARNRANGE = 25;

    Ui::USNavigationControls m_Controls;

    bool m_ImageAlreadySetToNode;
};

#endif // USNavigation_h
