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
  *   \brief An object of this class offers an UI to modify a tooltip of an IGT navigation tool.
  *
  *          The user can modify translation and orientation of the tooltip. The current tooltip
  *          might be provided as default data, then the widget starts with the given values for
  *          translation and orientation.
  *
  *          As long as the UI is open, there will also be a (temporary) preview data node, so
  *          the user can see the effect of his manipulations.
  *
  *          Please call the method SetDataStorage to initialize the UI with the data storage
  *          of the current application.
  *
  *   \ingroup IGTUI
  */
class MITKIGTUI_EXPORT QmitkNavigationToolCreationAdvancedWidget : public QDialog
{
  Q_OBJECT

  public:
    static const std::string VIEW_ID;

    QmitkNavigationToolCreationAdvancedWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkNavigationToolCreationAdvancedWidget();

    /** Initializes the view with the a data storage. This data storage is needed for the
     *  preview node during tooltip manipulation.
     */
    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);

    /** Sets the current tooltip surface, also for preview purposes (the preview node
     *  will be a clone of this surface). If there is no surface, a simple cone can be used.
     *  Please set cone to true in this case.
     */
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

    /** Reinitializes the view, e.g. after it was closed.
      */
    void ReInitialize();

signals:
    void DialogCloseRequested();
    void RetrieveDataForManualToolTipManipulation();

  protected slots:
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
    std::string m_SurfaceNodeName;
};
#endif // QmitkNavigationToolCreationAdvancedWidget_H
