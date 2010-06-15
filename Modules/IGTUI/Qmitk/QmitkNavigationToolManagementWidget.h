/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKNAVIGATIONTOOLMANAGEMENTWIDGET_H
#define QMITKNAVIGATIONTOOLMANAGEMENTWIDGET_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include "mitkNavigationTool.h"
#include <mitkNavigationToolStorage.h>

//ui header
#include "ui_QmitkNavigationToolManagementWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to manage NavigationTools and 
  *			 NavigationToolStorages. This means a user may create, save and load 
  *  		 single NavigationTools and/or NavigationToolStorages with this widget.
  *
  *      Be sure to call the Initialize-methode before you start the widget
  *      otherwise some errors might occure.
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkNavigationToolManagementWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    void Initialize(mitk::DataStorage* dataStorage);

    QmitkNavigationToolManagementWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationToolManagementWidget();

  protected slots:

    //main widget page:
    void OnAddTool();
    void OnDeleteTool();
    void OnEditTool();
    void OnLoadSingleTool();
    void OnSaveSingleTool();
    void OnLoadStorage();
    void OnSaveStorage();

    //widget page "add tool":
    void OnAddToolCancel();
    void OnAddToolSave();
    void OnLoadSurface();
    void OnLoadCalibrationFile();


  protected:

    /// \brief Creation of the connections
    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);
    
    Ui::QmitkNavigationToolManagementWidgetControls* m_Controls;

    /** @brief holds the DataStorage */
    mitk::DataStorage* m_DataStorage;

    /** @brief holds the NavigationToolStorage we are working with. */
    mitk::NavigationToolStorage::Pointer m_NavigationToolStorage;
    
    /** @brief shows if we are in edit mode, if not we create new navigation tool objects. */
    bool m_edit;

    //############## private help methods #######################
    void MessageBox(std::string s);
    void UpdateToolTable();
};
#endif