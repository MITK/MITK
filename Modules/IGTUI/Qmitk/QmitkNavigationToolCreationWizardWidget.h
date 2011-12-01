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

#ifndef QmitkNavigationToolCreationWizardWidget_H
#define QmitkNavigationToolCreationWizardWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include "mitkNavigationTool.h"
#include <mitkNavigationToolStorage.h>

//ui header
#include "ui_QmitkNavigationToolCreationWizardWidgetControls.h"

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
class MitkIGTUI_EXPORT QmitkNavigationToolCreationWizardWidget : public QWidget
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    void Initialize(mitk::DataStorage* dataStorage);

    QmitkNavigationToolCreationWizardWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationToolCreationWizardWidget();

  public signals:

    void NavigationToolFinished();
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

    Ui::QmitkNavigationToolCreationWizardWidgetControls* m_Controls;

    /** @brief holds the DataStorage */
    mitk::DataStorage* m_DataStorage;
    
    //############## private help methods #######################
    void MessageBox(std::string s);
    
};
#endif