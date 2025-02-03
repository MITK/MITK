/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMonaiLabelToolGUI.h"

#include <QIcon>
#include <QMessageBox>
#include <QUrl>
#include <QmitkStyleManager.h>
#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>

namespace
{
  mitk::IPreferences *GetPreferences()
  {
    auto *preferencesService = mitk::CoreServices::GetPreferencesService();
    return preferencesService->GetSystemPreferences()->Node("org.mitk.views.segmentation");
  }

  const static QString CONFIRM_QUESTION_TEXT =
    "Data will be sent to the processing server devoid of any patient information. Are you sure you want continue?";

  /* Pretested models in MITK and corresponding app */
  const static QMap<QString, QString> WHITELISTED_MODELS = {
    {"deepgrow_2d", "Radiology"},
    {"deepgrow_3d", "Radiology"},
    {"deepedit_seg", "Radiology"},
    {"localization_vertebra", "Radiology"},
    {"segmentation", "Radiology"},
    {"segmentation_spleen", "Radiology"},
    {"segmentation_vertebra", "Radiology"},
    {"deepgrow_pipeline", "Radiology"},
    {"vertebra_pipeline", "Radiology"}};

  /* Strictly unsupported models in MITK and corresponding app */
  const static QMap<QString, QString> BLACKLISTED_MODELS = {
    {"deepedit", "Radiology"},            // interaction type not yet supported
    {"localization_spine", "Radiology"}}; // Metadata discrepancy with label names
}

QmitkMonaiLabelToolGUI::QmitkMonaiLabelToolGUI(int dimension)
  : QmitkMultiLabelSegWithPreviewToolGUIBase(),
    m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc),
    m_Dimension(dimension)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false; };
  m_Preferences = GetPreferences();
  m_Preferences->OnPropertyChanged +=
    mitk::MessageDelegate1<QmitkMonaiLabelToolGUI, const mitk::IPreferences::ChangeEvent &>(
      this, &QmitkMonaiLabelToolGUI::OnPreferenceChangedEvent);
}

QmitkMonaiLabelToolGUI::~QmitkMonaiLabelToolGUI()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr != tool)
  {
    tool->MonaiStatusEvent -=
      mitk::MessageDelegate1<QmitkMonaiLabelToolGUI, const bool>(this, &QmitkMonaiLabelToolGUI::StatusMessageListener);
  }
  m_Preferences->OnPropertyChanged -=
    mitk::MessageDelegate1<QmitkMonaiLabelToolGUI, const mitk::IPreferences::ChangeEvent &>(
      this, &QmitkMonaiLabelToolGUI::OnPreferenceChangedEvent);
}

void QmitkMonaiLabelToolGUI::ConnectNewTool(mitk::SegWithPreviewTool *newTool)
{
  Superclass::ConnectNewTool(newTool);
  m_FirstPreviewComputation = true;
}

void QmitkMonaiLabelToolGUI::InitializeUI(QBoxLayout *mainLayout)
{
  auto wrapperWidget = new QWidget(this);
  mainLayout->addWidget(wrapperWidget);
  m_Controls.setupUi(wrapperWidget);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  connect(m_Controls.fetchUrl, SIGNAL(clicked()), this, SLOT(OnFetchBtnClicked()));
  connect(m_Controls.modelBox,
          QOverload<int>::of(&QComboBox::activated),
          [=](int index) { OnModelChanged(m_Controls.modelBox->itemText(index)); });
  QIcon refreshIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/view-refresh.svg"));
  m_Controls.fetchUrl->setIcon(refreshIcon);
  m_Controls.previewButton->setEnabled(false);
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
  m_FirstPreviewComputation = false;
}

void QmitkMonaiLabelToolGUI::DisplayWidgets(bool enabled)
{
  Superclass::DisplayTransferWidgets(enabled);
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
  if (model.IsInteractive())
  {
    this->WriteStatusMessage("Interactive model selected. Please press SHIFT + click on the render windows.\n");
    m_Controls.previewButton->setEnabled(false);
    this->DisplayWidgets(false);
  }
  else
  {
    this->WriteStatusMessage("Auto-segmentation model selected. Please click on Preview.\n");
    m_Controls.previewButton->setEnabled(true);
    this->DisplayWidgets(true);
  }
  auto selectedModel = m_Controls.modelBox->currentText().toStdString();
  for (const auto &modelObject : tool->GetInfoParameters()->models)
  {
    if (modelObject.name == selectedModel)
    {
      auto requestObject = std::make_unique<mitk::MonaiLabelRequest>();
      requestObject->model = modelObject;
      requestObject->hostName = tool->GetInfoParameters()->hostName;
      requestObject->port = tool->GetInfoParameters()->port;
      if (modelObject.IsInteractive()) // set only if interactive model
      {
        tool->m_RequestParameters = std::move(requestObject);
      }
      QStringList supportedLabels;
      for (const auto &label : modelObject.labels)
      {
        supportedLabels << QString::fromStdString(label.first);
      }
      m_Controls.labelListLabel->setText(supportedLabels.join(QStringLiteral(", ")));
      break;
    }
  }
  tool->MonaiStatusEvent +=
    mitk::MessageDelegate1<QmitkMonaiLabelToolGUI, const bool>(this, &QmitkMonaiLabelToolGUI::StatusMessageListener);
}

void QmitkMonaiLabelToolGUI::OnFetchBtnClicked()
{
  m_Controls.previewButton->setEnabled(false);
  m_Controls.labelListLabel->clear();
  auto reply = QMessageBox::question(this, "Confirm", ::CONFIRM_QUESTION_TEXT, QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::No)
  {
    MITK_INFO << "Didn't went ahead with Monai Label inferencing";
    return;
  }
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  QString urlString = m_Controls.urlBox->text();
  QUrl url(urlString);
  if (url.isValid() && !url.isLocalFile() && !url.hasFragment() && !url.hasQuery()) // sanity check
  {
    std::string hostName = url.host().toStdString();
    int port = url.port();
    try
    {
      tool->FetchOverallInfo(hostName, port);
      bool allowAllModels = m_Preferences->GetBool("monailabel allow all models", false);
      this->PopulateUI(allowAllModels);
      auto timeout_sec = std::make_unsigned_t<int>(m_Preferences->GetInt("monailabel timeout", 180));
      tool->SetTimeout(timeout_sec);
    }
    catch (const mitk::Exception &e)
    {
      m_Controls.appBox->clear();
      m_Controls.modelBox->clear();
      MITK_ERROR << e.GetDescription();
      this->WriteErrorMessage(e.GetDescription());
    }
  }
  else
  {
    std::string invalidURLMessage = "Invalid URL entered: " + urlString.toStdString();
    MITK_ERROR << invalidURLMessage;
    this->ShowErrorMessage(invalidURLMessage);
  }
}

void QmitkMonaiLabelToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  tool->ClearPicks(); // clear any interactive segmentation from before
  auto selectedModel = m_Controls.modelBox->currentText().toStdString();
  for (const auto &modelObject : tool->GetInfoParameters()->models)
  {
    if (modelObject.name == selectedModel)
    {
      auto requestObject = std::make_unique<mitk::MonaiLabelRequest>();
      requestObject->model = modelObject;
      requestObject->hostName = tool->GetInfoParameters()->hostName;
      requestObject->port = tool->GetInfoParameters()->port;
      tool->m_RequestParameters = std::move(requestObject);
      break;
    }
  }
  try
  {
    tool->UpdatePreview();
  }
  catch (const std::exception &e)
  {
    std::stringstream errorMsg;
    errorMsg << "<b>STATUS: </b>Error while processing parameters for MONAI Label segmentation. Reason: "
             << e.what();
    this->ShowErrorMessage(errorMsg.str());
    this->WriteErrorMessage(QString::fromStdString(errorMsg.str()));
    m_Controls.previewButton->setEnabled(true);
    return;
  }
  catch (...)
  {
    std::string errorMsg = "Unknown error occurred while generating MONAI Label segmentation.";
    this->ShowErrorMessage(errorMsg);
    m_Controls.previewButton->setEnabled(true);
    return;
  }
}

void QmitkMonaiLabelToolGUI::PopulateUI(bool allowAllModels)
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr == tool)
  {
    return;
  }
  m_Controls.appBox->clear();
  m_Controls.labelListLabel->clear();
  if (nullptr != tool->GetInfoParameters())
  {
    QString appName = QString::fromStdString(tool->GetInfoParameters()->name);
    auto autoModels = tool->GetAutoSegmentationModels(m_Dimension);
    auto interactiveModels = tool->GetInteractiveSegmentationModels(m_Dimension);
    autoModels.insert(autoModels.end(), interactiveModels.begin(), interactiveModels.end());
    this->WriteStatusMessage(appName);
    m_Controls.appBox->addItem(appName);
    this->PopulateModelBox(appName, autoModels, allowAllModels);
    m_Controls.modelBox->setCurrentIndex(-1);
  }
}

void QmitkMonaiLabelToolGUI::PopulateModelBox(QString appName, std::vector<mitk::MonaiModelInfo> models, bool allowAllModels)
{
  m_Controls.modelBox->clear();
  for (const auto &model : models)
  {
    QString modelName = QString::fromStdString(model.name);
    if (allowAllModels)
    {
      if (::BLACKLISTED_MODELS.contains(modelName) && appName.contains(::BLACKLISTED_MODELS[modelName]))
      {
        continue;
      }
      m_Controls.modelBox->addItem(modelName);
    }
    else
    {
      if (::WHITELISTED_MODELS.contains(modelName))
      {
        m_Controls.modelBox->addItem(modelName);
      }
    }
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

void QmitkMonaiLabelToolGUI::ShowErrorMessage(const std::string &message)
{
  this->setCursor(Qt::ArrowCursor);
  QMessageBox::critical(nullptr, "MONAI Label", message.c_str());
  MITK_WARN << message;
}

void QmitkMonaiLabelToolGUI::OnPreferenceChangedEvent(const mitk::IPreferences::ChangeEvent& event)
{
  if (event.GetProperty().rfind("monai", 0) == 0)
  {
    bool allowAllModels = m_Preferences->GetBool("monailabel allow all models", false);
    this->PopulateUI(allowAllModels);
    auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
    if (nullptr != tool)
    {
      auto timeout_sec = std::make_unsigned_t<int>(m_Preferences->GetInt("monailabel timeout", 180));
      tool->SetTimeout(timeout_sec);
    }
  }
}
