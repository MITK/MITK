/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitknnUNetToolGUI_h_Included
#define QmitknnUNetToolGUI_h_Included

#include "QmitkAutoMLSegmentationToolGUIBase.h"
#include "mitknnUnetTool.h"
#include "ui_QmitknnUNetToolGUIControls.h"
#include <MitkSegmentationUIExports.h>
#include <QCache>
#include <QProcess>

class MITKSEGMENTATIONUI_EXPORT nnUNetModel
{
public:
  mitk::ModelParams request;
  mitk::LabelSetImage::ConstPointer outputImage;
  nnUNetModel(const mitk::LabelSetImage *image) : outputImage(image) {}
  // friend class nnUNetEnsemble; // not really necessary?
};

/*class MITKSEGMENTATIONUI_EXPORT nnUNetEnsemble
{
public:
  nnUNetModel model1;
  nnUNetModel model2;
  std::string ppJsonDir;
};*/

class MITKSEGMENTATIONUI_EXPORT QmitknnUNetToolGUI : public QmitkAutoMLSegmentationToolGUIBase
{
  Q_OBJECT

public:
  mitkClassMacro(QmitknnUNetToolGUI, QmitkAutoMLSegmentationToolGUIBase);
  itkFactorylessNewMacro(Self);
  itkCloneMacro(Self);

  // std::vector<nnUNetModel> models;
  // std::vector<nnUNetEnsemble> ensembles;
  QCache<size_t, nnUNetModel> cache;

protected slots:

  void OnSettingsAccept();
  void OnDirectoryChanged(const QString &);
  void OnModelChanged(const QString &);
  void OnTrainerChanged(const QString &);
  void OnPythonChanged(const QString &);
  void OnCheckBoxChanged(int);

protected:
  QmitknnUNetToolGUI();
  ~QmitknnUNetToolGUI() = default;

  void ConnectNewTool(mitk::AutoSegmentationWithPreviewTool *newTool) override;
  void InitializeUI(QBoxLayout *mainLayout) override;
  void EnableWidgets(bool enabled) override;

private:
  void AutoParsePythonPaths();
  void ClearAllComboBoxes();
  mitk::ModelParams mapToRequest(QString&, QString&, QString&, QString&, std::vector<std::string>&);
  std::vector<std::string> FetchSelectedFoldsFromUI();
  static std::vector<QString> FetchFoldersFromDir(const QString &);
  // Declaring variables for strings and int only.
  QString m_Model;
  QString m_Task;
  // std::string m_nnUNetDirectory;
  // std::string m_ModelDirectory;
  QString m_ModelDirectory; // Change datatype to QDir?
  QString m_DatasetName;
  Ui_QmitknnUNetToolGUIControls m_Controls;
};

#endif
