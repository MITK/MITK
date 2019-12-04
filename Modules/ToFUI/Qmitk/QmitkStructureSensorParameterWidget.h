/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkStructureSensorParameterWidget_h
#define QmitkStructureSensorParameterWidget_h

#include "MitkToFUIExports.h"
#include "ui_QmitkStructureSensorParameterWidgetControls.h"

#include <mitkToFImageGrabber.h>

#include <QWidget>

/**
* @brief Widget for configuring the Structure Sensor device (Occipital, Inc.)
*
* @note This device is currently not available open-source, because the required WiFi
* protocol is part of the commercial mbits source code (http://mbits.info/). If you
* want to use the device please contact mitk-users@dkfz-heidelberg.de.
*
* @ingroup ToFUI
*/
class MITKTOFUI_EXPORT QmitkStructureSensorParameterWidget :public QWidget
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkStructureSensorParameterWidget(QWidget* p = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkStructureSensorParameterWidget() override;

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*!
    \brief returns the ToFImageGrabber
    \return ToFImageGrabber currently used by the widget
    */
    mitk::ToFImageGrabber* GetToFImageGrabber();

    /*!
    \brief sets the ToFImageGrabber
    */
    void SetToFImageGrabber(mitk::ToFImageGrabber* aToFImageGrabber);

    /*!
    \brief activate camera settings according to the parameters from GUI
    */
    void ActivateAllParameters();

    /**
     * @brief GetSelectedResolution getter for 640/320 resolution.
     * @return 320: 320x240, 640: 640x480 else -1 and a warning.
     */
    int GetSelectedResolution();

  protected slots:
    /**
     * @brief OnResolutionChanged called when the resolution combobox is changed.
     */
    void OnResolutionChanged();

  protected:

    Ui::QmitkStructureSensorParameterWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    mitk::ToFImageGrabber::Pointer m_ToFImageGrabber; ///< image grabber object to be configured by the widget
};

#endif // QmitkStructureSensorParameterWidget_h
