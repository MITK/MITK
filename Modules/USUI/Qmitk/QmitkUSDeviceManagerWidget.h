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

#ifndef _QmitkUSDeviceManagerWidget_H_INCLUDED
#define _QmitkUSDeviceManagerWidget_H_INCLUDED

#include "mitkUSUIExports.h"
#include "ui_QmitkUSDeviceManagerWidgetControls.h"

//QT headers
#include <QWidget>

//mitk header

/**
* @brief TODO
*
* @ingroup USUI
*/
class MitkUSUI_EXPORT QmitkUSDeviceManagerWidget :public QWidget
{

  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkUSDeviceManagerWidget(QWidget* p = 0, Qt::WindowFlags f1 = 0);
    virtual ~QmitkUSDeviceManagerWidget();

    /* @brief This method is part of the widget an needs not to be called seperately. */
    virtual void CreateQtPartControl(QWidget *parent);
    /* @brief This method is part of the widget an needs not to be called seperately. (Creation of the connections of main and control widget.)*/
    virtual void CreateConnections();
  
  signals:

    /*!
    \brief This signal is sent if the user has connected the TOF camera.
     *        The ToFImageGrabber is now availiable if the method GetToFImageGrabber() is called.
    */
    // void ToFCameraConnected();


  protected slots:
  
    /*!
    \brief slot called when the "Connect Camera" button was pressed
    * According to the selection in the camera combo box, the widget provides
    * the desired instance of the ToFImageGrabber
    */
    //void OnConnectCamera();
    

  protected:

    Ui::QmitkUSDeviceManagerWidgetControls* m_Controls; ///< member holding the UI elements of this widget

 

  private:

};

#endif // _QmitkUSDeviceManagerWidget_H_INCLUDED
