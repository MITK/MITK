/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMedSAMToolGUI_h
#define QmitkMedSAMToolGUI_h

#include "QmitkSegmentAnythingToolGUI.h"
#include <MitkSegmentationUIExports.h>
#include "ui_QmitkMedSAMGUIControls.h"
#include <mitkIPreferences.h>
#include "QmitknnUNetGPU.h"

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::MedSAMTool.
*/
class MITKSEGMENTATIONUI_EXPORT QmitkMedSAMToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMedSAMToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  /**
   * @brief Enable (or Disable) GUI elements. Currently, on the activate button
   * is affected.
   */
  void EnableAll(bool);

  /**
   * @brief Writes any message in white on the tool pane.
   */
  void WriteStatusMessage(const QString &);

  /**
   * @brief Writes any message in red on the tool pane.
   */
  void WriteErrorMessage(const QString &);

  /**
   * @brief Enable (or Disable) progressbar on GUI
   */
  void ShowProgressBar(bool);

  bool ActivateSAMDaemon();

  /**
   * @brief Function to listen to preference emitters.
   */
  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent &);

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const std::string &, QMessageBox::Icon = QMessageBox::Critical);

  bool ValidatePrefences();

protected:
  QmitkMedSAMToolGUI();
  ~QmitkMedSAMToolGUI();

  void InitializeUI(QBoxLayout *mainLayout) override;
  
  /**
   * @brief Function to listen to tool class status emitters.
   */
  void StatusMessageListener(const std::string&);

protected slots:
  /**
   * @brief Qt Slot
   */
  void OnActivateBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnPreviewBtnClicked();

  /**
   * @brief Qt Slot
   */
  void OnResetPicksClicked();

private:
  mitk::IPreferences *m_Preferences;
  QmitkGPULoader m_GpuLoader;
  Ui_QmitkMedSAMGUIControls m_Controls;
  bool m_FirstPreviewComputation = true;
  const std::string WARNING_SAM_NOT_FOUND =
    "MedSAM is not detected in the selected python environment. Please reinstall MedSAM.";
};
#endif
