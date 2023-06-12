/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkSegmentAnythingToolGUI_h
#define QmitkSegmentAnythingToolGUI_h

#include "QmitkSegWithPreviewToolGUIBase.h"
#include <MitkSegmentationUIExports.h>
#include "ui_QmitkSegmentAnythingGUIControls.h"
#include "QmitknnUNetGPU.h"
#include "QmitkSetupVirtualEnvUtil.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <mitkIPreferences.h>

/**
\ingroup org_mitk_gui_qt_interactivesegmentation_internal
\brief GUI for mitk::SegmentAnythingTool.
\sa mitk::PickingTool
*/
class MITKSEGMENTATIONUI_EXPORT QmitkSegmentAnythingToolGUI : public QmitkSegWithPreviewToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitkSegmentAnythingToolGUI, QmitkSegWithPreviewToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  inline static const QMap<QString, QUrl> VALID_MODELS_URL_MAP = {
    {"vit_b", QUrl("https://dl.fbaipublicfiles.com/segment_anything/sam_vit_b_01ec64.pth")},
    //{"vit_b", QUrl("http://www.google.ru/images/srpr/logo3w.png")},
    {"vit_l", QUrl("https://dl.fbaipublicfiles.com/segment_anything/sam_vit_l_0b3195.pth")},
    {"vit_h", QUrl("https://dl.fbaipublicfiles.com/segment_anything/sam_vit_h_4b8939.pth")}};

  /**
   * @brief Checks if SegmentAnything is found inside the selected python virtual environment.
   * @return bool
   */
  static bool IsSAMInstalled(const QString &);

protected slots:
  /**
   * @brief Qt Slot
   */
  void OnResetPicksClicked();

  /**
   * @brief Qt Slot
   */
  void OnActivateBtnClicked();

  /**
   * @brief Qt Slot
   */
  void FileDownloaded(QNetworkReply *); 

  /**
  * @brief Qt Slot
  */
  void OnParametersChanged(const QString&); 

protected:
  QmitkSegmentAnythingToolGUI();
  ~QmitkSegmentAnythingToolGUI() = default;

  void InitializeUI(QBoxLayout *mainLayout) override;
  
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
   * @brief Enable (or Disable) GUI elements.
   */
  void EnableAll(bool);
  
  /**
  * 
  */
  bool DownloadModel(const QString &);

  /**
  * 
  */
  void ShowProgressBar(bool);

  void ActivateSAMDaemon();

  bool ValidatePrefences();


private:
  mitk::IPreferences *m_Prefences;
  QNetworkAccessManager m_Manager;
  Ui_QmitkSegmentAnythingGUIControls m_Controls;
  QString m_PythonPath;
  QmitkGPULoader m_GpuLoader;
  bool m_FirstPreviewComputation = true;
  const std::string WARNING_SAM_NOT_FOUND =
    "SAM is not detected in the selected python environment. Please reinstall SAM.";
};
#endif
