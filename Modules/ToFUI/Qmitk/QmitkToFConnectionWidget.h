/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef _QMITKTOFCONNECTIONWIDGET_H_INCLUDED
#define _QMITKTOFCONNECTIONWIDGET_H_INCLUDED

#include "mitkTOFUIExports.h"
#include "ui_QmitkToFConnectionWidgetControls.h"

//QT headers
#include <QWidget>

//mitk headers
#include "mitkToFImageGrabber.h"

/**
* @brief Widget allowing to connect to different ToF / range cameras (located in module ToFProcessing)
*
* The widget basically allows to connect/disconnect to different ToF cameras
*
* @ingroup ToFUI
*/
class mitkTOFUI_EXPORT QmitkToFConnectionWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkToFConnectionWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkToFConnectionWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();

    /*!
    \brief returns the ToFImageGrabber which was provided by the ToFImageGrabberCreator after selecting a camera / player
    \return ToFImageGrabber currently used by the widget
    */
    mitk::ToFImageGrabber* GetToFImageGrabber();
  
  signals:

    /*!
    \brief This signal is sent if the user has connected the TOF camera.
     *        The ToFImageGrabber is now availiable if the method GetToFImageGrabber() is called.
    */
    void ToFCameraConnected();
    /*!
    \brief This signal is sent if the user has disconnect the TOF camera.
     */
    void ToFCameraDisconnected();
    /*!
    \brief signal that is emitted when a ToF camera is selected in the combo box
    */
    void ToFCameraSelected(const QString selectedText);

  protected slots:
  
    /*!
    \brief slot called when the "Connect Camera" button was pressed
    * According to the selection in the camera combo box, the ToFImageGrabberCreator provides
    * the desired instance of the ToFImageGrabber
    */
    void OnConnectCamera();
    /*!
    \brief slot updating the GUI elements after the selection of the camera combo box has changed
    */
    void OnSelectCamera(const QString selectedText);

  protected:

    Ui::QmitkToFConnectionWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    mitk::ToFImageGrabber* m_ToFImageGrabber; ///< member holding the current ToFImageGrabber

    int m_IntegrationTime; ///< member for the current integration time of the ToF device
    int m_ModulationFrequency; ///< member for the current modulation frequency of the ToF device

  private:

    void ShowParameterWidget();

    void ShowPMDParameterWidget();

    void ShowMESAParameterWidget();

    void ShowPlayerParameterWidget();
};

#endif // _QMITKTOFCONNECTIONWIDGET_H_INCLUDED
