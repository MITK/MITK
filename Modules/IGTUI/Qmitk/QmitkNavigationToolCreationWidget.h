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

#ifndef QmitkNavigationToolCreationWidget_H
#define QmitkNavigationToolCreationWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include <mitkNavigationTool.h>
#include <mitkNavigationToolStorage.h>
#include <mitkNodePredicateDataType.h>

//ui header
#include "ui_QmitkNavigationToolCreationWidget.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to create new NavigationTools
  *
  *      Be sure to call the Initialize-methode before you start the widget
  *      otherwise some errors might occure.
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkNavigationToolCreationWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    /** @brief Initializes the widget.
      * @param dataStorage  The data storage is needed to offer the possibility to choose surfaces from the data storage for tool visualization.
      * @param supposedIdentifier This Identifier is supposed for the user. It is needed because every identifier in a navigation tool storage must be unique and we don't know the others.
      */
    void Initialize(mitk::DataStorage* dataStorage, std::string supposedIdentifier);

    /** @brief Sets the default tracking device type. You may also define if it is changeable or not.*/
    void SetTrackingDeviceType(mitk::TrackingDeviceType type, bool changeable = true);

    /** @brief Sets the default data of all input fields. The default data is used from the default tool which is given as parameter. */
    void SetDefaultData(mitk::NavigationTool::Pointer DefaultTool);

    QmitkNavigationToolCreationWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationToolCreationWidget();

    /** @return Returns the created tool. Returns NULL if no tool was created yet. */
    mitk::NavigationTool::Pointer GetCreatedTool();


  signals:

    /** @brief This signal is emited if the user finished the creation of the tool. */
    void NavigationToolFinished();

    /** @brief This signal is emited if the user canceld the creation of the tool. */
    void Canceled();

  protected slots:

    void OnCancel();
    void OnFinished();
    void OnLoadSurface();
    void OnLoadCalibrationFile();


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);

    Ui::QmitkNavigationToolCreationWidgetControls* m_Controls;

    /** @brief holds the DataStorage */
    mitk::DataStorage* m_DataStorage;

    /** @brief this pointer holds the tool which is created */
    mitk::NavigationTool::Pointer m_CreatedTool;

    //############## private help methods #######################
    void MessageBox(std::string s);

};
#endif
