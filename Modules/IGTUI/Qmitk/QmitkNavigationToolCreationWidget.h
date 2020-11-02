/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkNavigationToolCreationWidget_H
#define QmitkNavigationToolCreationWidget_H

//QT headers
#include <QWidget>

//mitk headers
#include "MitkIGTUIExports.h"
#include <mitkNavigationTool.h>
#include <mitkNavigationToolStorage.h>
#include <mitkNodePredicateDataType.h>
#include "QmitkInteractiveTransformationWidget.h"

#include <QDialog>

//Microservices
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usModuleContext.h>

//ui header
#include "ui_QmitkNavigationToolCreationWidget.h"

/** Documentation:
 *   \brief An object of this class offers an UI to create or modify NavigationTools.
 *
 *          Be sure to call the initialize method before you start the widget
 *          otherwise some errors might occure.
 *
 *   \ingroup IGTUI
 */
class MITKIGTUI_EXPORT QmitkNavigationToolCreationWidget : public QWidget
{
  Q_OBJECT

public:
  static const std::string VIEW_ID;

  /** @brief Initializes the widget.
    * @param dataStorage  The data storage is needed to offer the possibility to choose surfaces from the data storage for tool visualization.
    * @param supposedIdentifier This Identifier is supposed for the user. It is needed because every identifier in a navigation tool storage must be unique and we don't know the others.
    * @param supposedName
    */
  void Initialize(mitk::DataStorage* dataStorage, const std::string &supposedIdentifier, const std::string &supposedName = "NewTool");

  /** @brief This Function will add a new node to the Data Manager with given name to enable a preview of the m_ToolToBeEdited
  */
  void ShowToolPreview(std::string _name);

  /** @brief Sets the default tracking device type. You may also define if it is changeable or not.*/
  void SetTrackingDeviceType(mitk::TrackingDeviceType type, bool changeable = true);

    QmitkNavigationToolCreationWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
    ~QmitkNavigationToolCreationWidget() override;
  /** @brief Sets the default data of all input fields. The default data is used from the default tool which is given as parameter. */
  void SetDefaultData(mitk::NavigationTool::Pointer DefaultTool);

    /** @return Returns the created tool. Returns nullptr if no tool was created yet. */
    mitk::NavigationTool::Pointer GetCreatedTool();


signals:

  /** @brief This signal is emitted if the user finished the creation of the tool. */
  void NavigationToolFinished();

  /** @brief This signal is emitted if the user canceled the creation of the tool. */
  void Canceled();

  protected slots:


  void OnLoadCalibrationFile();
  void OnSurfaceUseToggled();
  void OnLoadSurface();
  void OnEditToolTip();
  void OnEditToolTipFinished(mitk::AffineTransform3D::Pointer toolTip);

  void OnCancel();
  void OnFinished();

  void GetValuesFromGuiElements();

private:

  //############## private help methods #######################
  /** Shows a message box with the given message s. */
  void MessageBox(std::string s);

  /** Set the tool landmark lists in the UI.*/
  void FillUIToolLandmarkLists(mitk::PointSet::Pointer calLandmarks, mitk::PointSet::Pointer regLandmarks);

  /** Returns the tool landmark lists from the UI.
  * @param[out] calLandmarks Returns a pointer to the calibration landmarks point set.
  * @param[out] regLandmarks Returns a pointer to the registration landmarks point set.
  */
  void GetUIToolLandmarksLists(mitk::PointSet::Pointer& calLandmarks, mitk::PointSet::Pointer& regLandmarks);

  /** Initializes the tool landmark lists in the UI. */
  void InitializeUIToolLandmarkLists();
  void RefreshTrackingDeviceCollection();

  void SetGuiElements();

protected:
  /// \brief Creation of the connections
  virtual void CreateConnections();

  virtual void CreateQtPartControl(QWidget *parent);

  Ui::QmitkNavigationToolCreationWidgetControls* m_Controls;

  /** @brief this pointer holds the tool which is edited. If finished is clicked, it will be copied to the final tool, if it is cancled, it is reseted and not used.
      This can be regarded as the clipboard for all changes.  */
  mitk::NavigationTool::Pointer m_ToolToBeEdited;

  /** @brief this pointer holds the tool which is created and returned */
  mitk::NavigationTool::Pointer m_FinalTool;

  QmitkInteractiveTransformationWidget* m_ToolTransformationWidget;

  /** @brief holds the DataStorage */
  mitk::DataStorage* m_DataStorage;


  /** Hold the data nodes which are needed for the landmark widgets. */
  mitk::DataNode::Pointer m_calLandmarkNode, m_regLandmarkNode;
};
#endif
