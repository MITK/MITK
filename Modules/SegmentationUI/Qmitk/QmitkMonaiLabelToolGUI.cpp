/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMonaiLabelToolGUI.h"

#include "mitkMonaiLabelTool.h"
#include "usServiceReference.h"
#include <QIcon>
#include <QUrl>
#include <QmitkStyleManager.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <QMessageBox>


QmitkMonaiLabelToolGUI::QmitkMonaiLabelToolGUI(int dimension)
  : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  m_Dimension = dimension;
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false; };
}

QmitkMonaiLabelToolGUI::~QmitkMonaiLabelToolGUI()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr != tool)
  {
    tool->MonaiStatusEvent -= mitk::MessageDelegate1<QmitkMonaiLabelToolGUI, const bool>(this, &QmitkMonaiLabelToolGUI::StatusMessageListener);
  }
}

void QmitkMonaiLabelToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkMonaiLabelToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  connect(m_Controls.fetchUrl, SIGNAL(clicked()), this, SLOT(OnFetchBtnClicked()));
  connect(m_Controls.modelBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnModelChanged(m_Controls.modelBox->itemText(index)); });
  QIcon refreshIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/view-refresh.svg"));
  m_Controls.fetchUrl->setIcon(refreshIcon);

  Superclass::InitializeUI(mainLayout);
}

void QmitkMonaiLabelToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
}

void QmitkMonaiLabelToolGUI::StatusMessageListener(const bool status)
{
  if (!status)
  {
    return;
  }
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  this->SetLabelSetPreview(tool->GetPreviewSegmentation());
  this->ActualizePreviewLabelVisibility();
  //Superclass::DisplayWidgets(false);
  m_FirstPreviewComputation = false;
}

void QmitkMonaiLabelToolGUI::DisplayWidgets(bool enabled)
{
  Superclass::DisplayWidgets(enabled);
  m_Controls.previewButton->setVisible(enabled);
}

void QmitkMonaiLabelToolGUI::OnModelChanged(const QString &modelName)
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  m_Controls.labelListLabel->clear();
  mitk::MonaiModelInfo model = tool->GetModelInfoFromName(modelName.toStdString());
  if ("deepgrow" == model.type || "deepedit" == model.type)
  {
    this->WriteStatusMessage("Interactive model selected. Please press SHIFT + click on the render windows.\n");
    m_Controls.previewButton->setEnabled(false);
    this->DisplayWidgets(false);
  }
  else
  {
    this->WriteStatusMessage("Auto-segmentation model selected. Please click on Preview. Label selection will be ignored.\n");
    m_Controls.previewButton->setEnabled(true);
    this->DisplayWidgets(true);
  }
  std::string selectedModel = m_Controls.modelBox->currentText().toStdString();
  for (const mitk::MonaiModelInfo &modelObject : tool->m_InfoParameters->models)
  {
    if (modelObject.name == selectedModel)
    {
      auto requestObject = std::make_unique<mitk::MonaiLabelRequest>();
      requestObject->model = modelObject;
      requestObject->hostName = tool->m_InfoParameters->hostName;
      requestObject->port = tool->m_InfoParameters->port;
      tool->m_RequestParameters = std::move(requestObject);
      QStringList supportedLabels;
      for (const auto &label : modelObject.labels)
      {
        supportedLabels << QString::fromStdString(label.first);
      }
      m_Controls.labelListLabel->setText(supportedLabels.join(','));
      break;
    }
  }
  tool->MonaiStatusEvent += mitk::MessageDelegate1<QmitkMonaiLabelToolGUI, const bool>(this, &QmitkMonaiLabelToolGUI::StatusMessageListener);
}

void QmitkMonaiLabelToolGUI::OnFetchBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  tool->m_InfoParameters.reset();
  m_Controls.modelBox->clear();
  m_Controls.appBox->clear();
  QString urlString = m_Controls.urlBox->text();
  QUrl url(urlString);
  if (url.isValid() && !url.isLocalFile() && !url.hasFragment() && !url.hasQuery()) // sanity check
  {
    QString hostName = url.host();
    int port = url.port();
    try
    {
      tool->GetOverallInfo(hostName.toStdString(), port);  // tool->GetOverallInfo("localhost",8000");
      if (nullptr != tool->m_InfoParameters)
      {
        std::string response = tool->m_InfoParameters->name;
        std::vector<mitk::MonaiModelInfo> autoModels = tool->GetAutoSegmentationModels(m_Dimension);
        std::vector<mitk::MonaiModelInfo> interactiveModels = tool->GetInteractiveSegmentationModels(m_Dimension);
        autoModels.insert(autoModels.end(), interactiveModels.begin(), interactiveModels.end());
        this->WriteStatusMessage(QString::fromStdString(response));
        m_Controls.appBox->addItem(QString::fromStdString(response));
        for (auto &model : autoModels)
        {
          m_Controls.modelBox->addItem(QString::fromStdString(model.name));
        }
        m_Controls.modelBox->setCurrentIndex(-1);
      }
    }
    catch (const mitk::Exception &e)
    {
      MITK_ERROR << e.GetDescription(); // Add GUI msg box to show
    }
  }
  else
  {
    MITK_ERROR << "Invalid URL entered: " << urlString.toStdString(); // set as status message on GUI
  }
}

void QmitkMonaiLabelToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(this, "Confirm", m_CONFIRM_QUESTION_TEXT, QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::No)
  {
    MITK_INFO << "Didn't went ahead with Monai Label inferencing";
    return;
  }

  bool test = false;
  if (!test)
  {
    std::string selectedModel = m_Controls.modelBox->currentText().toStdString();
    for (const mitk::MonaiModelInfo &modelObject : tool->m_InfoParameters->models)
    {
      if (modelObject.name == selectedModel)
      {
        auto requestObject = std::make_unique<mitk::MonaiLabelRequest>();
        requestObject->model = modelObject;
        requestObject->hostName = tool->m_InfoParameters->hostName;
        requestObject->port = tool->m_InfoParameters->port;
        if (!m_FirstPreviewComputation && tool->GetIsLastSuccess() && *requestObject == *(tool->m_RequestParameters))
        {
          MITK_INFO << "won't do segmentation...";
          return;
        }
        else
        {
          tool->m_RequestParameters = std::move(requestObject);
        }
        break;
      }
    }
  }
  else
  {
    MITK_INFO << " RUNNING ON TEST parameters...";
    tool->m_RequestParameters = std::make_unique<mitk::MonaiLabelRequest>();
    mitk::MonaiModelInfo modelObject;
    modelObject.name = "deepedit_seg";
    tool->m_RequestParameters->model = modelObject;
    tool->m_RequestParameters->hostName = "localhost";
    tool->m_RequestParameters->port = 8000;
  }
  try
  {
    tool->UpdatePreview();
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for TotalSegmentator segmentation. Reason: "
             << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    m_Controls.previewButton->setEnabled(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unkown error occured while generation TotalSegmentator segmentation.";
    this->ShowErrorMessage(errorMsg);
    m_Controls.previewButton->setEnabled(true);
    return;
  }
}

void QmitkMonaiLabelToolGUI::WriteStatusMessage(const QString &message)
{
  m_Controls.responseNote->setText(message);
  m_Controls.responseNote->setStyleSheet("font-weight: bold; color: white");
  qApp->processEvents();
}

void QmitkMonaiLabelToolGUI::WriteErrorMessage(const QString &message)
{
  m_Controls.responseNote->setText(message);
  m_Controls.responseNote->setStyleSheet("font-weight: bold; color: red");
  qApp->processEvents();
}

void QmitkMonaiLabelToolGUI::ShowErrorMessage(const std::string &message, QMessageBox::Icon icon)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox *messageBox = new QMessageBox(icon, nullptr, message.c_str());
  messageBox->exec();
  delete messageBox;
  MITK_WARN << message;
}
