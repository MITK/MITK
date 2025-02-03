/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTotalSegmentatorToolGUI_h_Included
#define QmitkTotalSegmentatorToolGUI_h_Included

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "ui_QmitkTotalSegmentatorGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QMessageBox>
#include <mitkIPreferences.h>
/**
 * @brief Installer class for TotalSegmentator Tool.
 * Class specifies the virtual environment name, install version, packages required to pip install
 * and implements SetupVirtualEnv method.
 * 
 */
/*
  \ingroup org_mitk_gui_qt_interactivesegmentation_internal
  \brief GUI for mitk::TotalSegmentatorTool.
  \sa mitk::
*/
class MITKSEGMENTATIONUI_EXPORT QmitkTotalSegmentatorToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkTotalSegmentatorToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

protected slots:

  /**
   * @brief Qt Slot
   */
  void OnPreviewBtnClicked();

protected:
  QmitkTotalSegmentatorToolGUI();
  ~QmitkTotalSegmentatorToolGUI() override;

  void ConnectNewTool(mitk::SegWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;

  /**
   * @brief Enable (or Disable) GUI elements.
   */
  void EnableAll(bool);

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const std::string &, QMessageBox::Icon = QMessageBox::Critical);

  /**
   * @brief Writes any message in white on the tool pane.
   */
  void WriteStatusMessage(const QString &);

  /**
   * @brief Writes any message in red on the tool pane.
   */
  void WriteErrorMessage(const QString &);

  /**
   * @brief Checks for changes in preferences
   */
  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent &);

  Ui_QmitkTotalSegmentatorToolGUIControls m_Controls;
  bool m_FirstPreviewComputation = true;
  EnableConfirmSegBtnFunctionType m_SuperclassEnableConfirmSegBtnFnc;
  mitk::IPreferences *m_Preferences;
  const QStringList VALID_TASKS = {
    "total",
    "total_mr",
    "cerebral_bleed",
    "hip_implant",
    "coronary_arteries",
    "body",
    "lung_vessels",
    "pleural_pericard_effusion",
    "head_glands_cavities",
    "head_muscles",
    "headneck_bones_vessels",  
    "headneck_muscles",                   
    "liver_vessels"
  };
};
#endif
