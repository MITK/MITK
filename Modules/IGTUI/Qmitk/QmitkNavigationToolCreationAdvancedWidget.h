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

#ifndef QmitkNavigationToolCreationAdvancedWidget_H
#define QmitkNavigationToolCreationAdvancedWidget_H

//QT headers
#include <QDialog>

//Mitk headers
#include "MitkIGTUIExports.h"
#include "mitkDataStorage.h"
#include "mitkSurface.h"

// Qmitk headers
#include "QmitkInteractiveTransformationWidget.h"

//ui header
#include "ui_QmitkNavigationToolCreationAdvancedWidgetControls.h"

 /** Documentation:
  *   \brief An object of this class offers an UI to create a widget to access the advance tool creation options.
  *
  *
  *   \ingroup IGTUI
  */
class MitkIGTUI_EXPORT QmitkNavigationToolCreationAdvancedWidget : public QDialog
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkNavigationToolCreationAdvancedWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationToolCreationAdvancedWidget();

    enum ToolType{
    Instrument,
    Fiducial,
    Skinmarker,
    Unknown
    };

    ToolType GetToolType();
    void SetToolType(int type);

    std::string GetToolIdentifier();
    void SetToolIdentifier(std::string _arg);

    std::string GetSerialNumber();
    void SetSerialNumber(std::string _arg);

    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

    void SetToolTipSurface(bool cone, mitk::DataNode::Pointer node = NULL);

    mitk::DataNode::Pointer GetManipulatedToolTip();

signals:
    void DialogCloseRequested();
    void RetrieveDataForManualToolTipManipulation();

  protected slots:
    void OnToolTypeChanged(int state);
    void OnManipulateTooltip(int state);
    void OnClose();
    void OnApplyManipulatedToolTip();

  protected:

    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);
    void RetrieveAndInitializeDataForTooltipManipulation();
    // Member variables
    Ui::QmitkNavigationToolCreationAdvancedWidgetControls* m_Controls;

    mitk::DataStorage::Pointer m_DataStorage;
    mitk::Surface::Pointer m_ToolTipSurface;
    mitk::Surface::Pointer m_ManipulatedToolTip;
    ToolType m_ToolType;    ///< \brief The variable holds the type of the tool selected by the user.
    std::string m_SurfaceNodeName;
};
#endif // QmitkNavigationToolCreationAdvancedWidget_H
