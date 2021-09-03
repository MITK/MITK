/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.s

============================================================================*/

#ifndef QmitknnUNetToolGUI_h_Included
#define QmitknnUNetToolGUI_h_Included

#include "QmitkAutoMLSegmentationToolGUIBase.h"
#include "QmitknnUNetWorker.h"
#include "mitknnUnetTool.h"
#include "ui_QmitknnUNetToolGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QCache>
#include <QThread>
#include <ctkPathLineEdit.h>


class MITKSEGMENTATIONUI_EXPORT nnUNetModel
{
public:
  std::vector<mitk::ModelParams> requestQ;
  mitk::LabelSetImage::ConstPointer outputImage;
  size_t hashCode = 0;

  size_t GetUniqueHash()
  {
    if (hashCode == 0)
    {
      for (mitk::ModelParams request : requestQ)
      {
        // sum of individual hash is the final hash
        hashCode += request.generateHash();
      }
    }
    return hashCode;
  }
};

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetToolGUI : public QmitkAutoMLSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnUNetToolGUI, QmitkAutoMLSegmentationToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  QCache<size_t, nnUNetModel> cache;

protected slots:

  void OnSettingsAccept();
  void OnDirectoryChanged(const QString &);
  void OnModelChanged(const QString &);
  void OnTaskChanged(const QString &);
  void OnTrainerChanged(const QString &);
  void OnPythonChanged(const QString &);
  void OnCheckBoxChanged(int);
  void SegmentationProcessFailed();
  void SetSegmentation(const mitk::LabelSetImage*);
  void OnModalitiesNumberChanged(int);

signals:

  /**
   * @brief signal for starting the segmentation which is caught by a worker thread.
   */
  void Operate(mitk::nnUNetTool *tool);
  /**
   * @brief if a segmentation is executed when the tool is started, emit a signal for waiting in the worker thread.
   *
   * @param tool the Segmentation Tool to check in worker, if the segmentation is still running.
   */
  // void Wait(mitk::nnUNetTool *tool);
  

protected:
  QmitknnUNetToolGUI();
  ~QmitknnUNetToolGUI() = default;

  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;
  void EnableWidgets(bool enabled) override;

private:
  void AutoParsePythonPaths();
  void ClearAllComboBoxes();
  mitk::ModelParams MapToRequest(QString &, QString &, QString &, QString &, std::vector<std::string> &);
  std::vector<std::string> FetchSelectedFoldsFromUI();
  std::vector<std::string> FetchMultiModalPathsFromUI();
  template <typename T>
  static T FetchFoldersFromDir(const QString &);
  // Declaring variables for strings and int only.
  QString m_Model;
  QString m_Task;
  QString m_ModelDirectory; // Change datatype to QDir?
  QString m_DatasetName;
  Ui_QmitknnUNetToolGUIControls m_Controls;
  QThread *m_SegmentationThread;
  nnUNetSegmentationWorker *m_Worker;
  std::vector<ctkPathLineEdit*> m_ModalPaths;
  int m_UI_ROWS;
};

#endif
