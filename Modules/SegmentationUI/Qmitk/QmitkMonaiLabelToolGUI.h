/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkMonaiLabelToolGUI_h_Included
#define QmitkMonaiLabelToolGUI_h_Included

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "ui_QmitkMonaiLabelToolGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QMessageBox>

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
  void ShowErrorMessage(const std::string &, QMessageBox::Icon = QMessageBox::Critical);

  /**
   * @brief Function to listen to tool class status emitters.
   */
  void StatusMessageListener(const bool);

private:
  Ui_QmitkMonaiLabelToolGUIControls m_Controls;
  bool m_FirstPreviewComputation = true;
  EnableConfirmSegBtnFunctionType m_SuperclassEnableConfirmSegBtnFnc;
  int m_Dimension;
  QString m_CONFIRM_QUESTION_TEXT =
    "Data will be sent to the processing server devoid of any patient information. Are you sure you want continue?";
  const QStringList SUPPORTED_MODELS = { // deepedit and localization_spine not supported
    "deepgrow_2d",
    "deepgrow_3d",
    "deepedit_seg",
    "localization_vertebra",
    "segmentation",
    "segmentation_spleen",
    "segmentation_vertebra",
    "deepgrow_pipeline",
    "vertebra_pipeline,"
  };
};

#endif
