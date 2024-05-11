/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMonaiLabelToolGUI_h
#define QmitkMonaiLabelToolGUI_h

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "ui_QmitkMonaiLabelToolGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QMessageBox>
#include <mitkIPreferences.h>
#include <mitkMonaiLabelTool.h>
#include <QMap>

class MITKSEGMENTATIONUI_EXPORT QmitkMonaiLabelToolGUI : public QmitkMultiLabelSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkMonaiLabelToolGUI, QmitkMultiLabelSegWithPreviewToolGUIBase);
  itkCloneMacro(Self);

protected slots:

  void OnPreviewBtnClicked();
  void OnFetchBtnClicked();
  void OnModelChanged(const QString &);

protected:
  QmitkMonaiLabelToolGUI(int);
  ~QmitkMonaiLabelToolGUI();

  void ConnectNewTool(mitk::SegWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;

  void EnableWidgets(bool enabled) override;

  virtual void DisplayWidgets(bool enabled);

  /**
   * @brief Writes any message in white on the tool pane.
   */
  void WriteStatusMessage(const QString &);

  /**
   * @brief Writes any message in red on the tool pane.
   */
  void WriteErrorMessage(const QString &);

  /**
   * @brief Creates a QMessage object and shows on screen.
   */
  void ShowErrorMessage(const std::string &message);

  /**
   * @brief Function to listen to tool class status emitters.
   */
  void StatusMessageListener(const bool status);

  /**
   * @brief Function to listen to Preference changes
   */
  void OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent &event);

  /**
   * @brief Helper function to write MONAI model info in to model combo box
   */
  void PopulateModelBox(QString appName, std::vector<mitk::MonaiModelInfo> models, bool allowAllModels);

  /**
   * @brief Helper function to populate required server metadata into UI
   */
  void PopulateUI(bool allowAllModels);

private:
  mitk::IPreferences *m_Preferences;
  Ui_QmitkMonaiLabelToolGUIControls m_Controls;
  bool m_FirstPreviewComputation = true;
  EnableConfirmSegBtnFunctionType m_SuperclassEnableConfirmSegBtnFnc;
  int m_Dimension;
  static const QString CONFIRM_QUESTION_TEXT;
  static const QMap<QString, QString> WHITELISTED_MODELS;
  static const QMap<QString, QString> BLACKLISTED_MODELS;
};

#endif
