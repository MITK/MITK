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

    /** Sets a default tooltip transform, which will shown in the beginning.
     *  If the windows is already open, the transform will be set to default
     *  immediately.
     */
    void SetDefaultTooltip(mitk::AffineTransform3D::Pointer defaultToolTip);

    /** @return Returns the manipulated tip transform. Returns an identity transform if
     *          nothing was manipulated.
     */
    mitk::AffineTransform3D::Pointer GetManipulatedToolTip();

    /** Reinitializes the widget, e.g. after it was closed. */
    void ReInitialize();

signals:
    void DialogCloseRequested();
    void RetrieveDataForManualToolTipManipulation();

  protected slots:
    void OnToolTypeChanged(int state);
    void OnClose();
    void OnApplyManipulatedToolTip();

  protected:

    virtual void CreateConnections();

    virtual void CreateQtPartControl(QWidget *parent);
    void RetrieveAndInitializeDataForTooltipManipulation();
    // Member variables
    Ui::QmitkNavigationToolCreationAdvancedWidgetControls* m_Controls;

    mitk::AffineTransform3D::Pointer m_DefaultToolTip;

    mitk::DataStorage::Pointer m_DataStorage;
    mitk::Surface::Pointer m_ToolTipSurface;
    mitk::Surface::Pointer m_ManipulatedToolTip; ///< manipulated surface object, which holds the tooltip as geometry
    ToolType m_ToolType; ///< \brief The variable holds the type of the tool selected by the user.
    std::string m_SurfaceNodeName;
};
#endif // QmitkNavigationToolCreationAdvancedWidget_H
