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

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkMonaiLabelToolGUI, "")

QmitkMonaiLabelToolGUI::QmitkMonaiLabelToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false; };
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

  QIcon refreshIcon =
    QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/view-refresh.svg"));
  m_Controls.fetchUrl->setIcon(refreshIcon);

  Superclass::InitializeUI(mainLayout);
}

void QmitkMonaiLabelToolGUI::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
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
        std::vector<mitk::MonaiModelInfo> autoModels = tool->GetAutoSegmentationModels();
        m_Controls.responseNote->setText(QString::fromStdString(response));
        m_Controls.appBox->addItem(QString::fromStdString(response));
        for (auto &model : autoModels)
        {
          m_Controls.modelBox->addItem(QString::fromStdString(model.name));
        }
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
    m_FirstPreviewComputation = false;
    this->SetLabelSetPreview(tool->GetPreviewSegmentation());
    tool->IsTimePointChangeAwareOn();
    this->ActualizePreviewLabelVisibility();
  }
  catch (...)
  {
    MITK_ERROR << "Connection error"; //This catch is never reached when exception is thrown in UpdatePreview method
  }
  
}
