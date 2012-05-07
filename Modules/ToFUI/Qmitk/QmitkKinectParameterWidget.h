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

#ifndef _QMITKKINECTPARAMETERWIDGET_H_INCLUDED
#define _QMITKKINECTPARAMETERWIDGET_H_INCLUDED

#include "mitkTOFUIExports.h"
#include "ui_QmitkKinectParameterWidgetControls.h"

//mitk headers
#include <mitkToFImageGrabber.h>

#include <QWidget>

/**
* @brief Widget for configuring the Kinect device
*
* @ingroup ToFUI
*/
class mitkTOFUI_EXPORT QmitkKinectParameterWidget :public QWidget
{
  //this is needed for all Qt objects that should have a MOC object (everything that derives from QObject)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkKinectParameterWidget(QWidget* p = 0, Qt::WindowFlags f = 0);
    virtual ~QmitkKinectParameterWidget();

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

  protected slots:
    void OnAcquisitionModeChanged();

  protected:

    Ui::QmitkKinectParameterWidgetControls* m_Controls; ///< member holding the UI elements of this widget

    mitk::ToFImageGrabber::Pointer m_ToFImageGrabber; ///< image grabber object to be configured by the widget

  private:
};

#endif // _QmitkKinectParameterWIDGET_H_INCLUDED
