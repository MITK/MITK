#include "QmitkMonaiLabelToolGUI.h"
#include "mitkMonaiLabelTool.h"

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include "usServiceReference.h"
#include <QIcon>
#include <QUrl>
#include <QmitkStyleManager.h>


MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkMonaiLabelToolGUI, "")

QmitkMonaiLabelToolGUI::QmitkMonaiLabelToolGUI()
  : QmitkMultiLabelSegWithPreviewToolGUIBase(), m_SuperclassEnableConfirmSegBtnFnc(m_EnableConfirmSegBtnFnc)
{
  m_EnableConfirmSegBtnFnc = [this](bool enabled)
  { 
    return !m_FirstPreviewComputation ? m_SuperclassEnableConfirmSegBtnFnc(enabled) : false;
  };
}

void QmitkMonaiLabelToolGUI::ConnectNewTool(mitk::SegWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);
  newTool->IsTimePointChangeAwareOff();
  m_FirstPreviewComputation = true;
}

void QmitkMonaiLabelToolGUI::InitializeUI(QBoxLayout* mainLayout)
{ 
  m_Controls.setupUi(this);
  mainLayout->addLayout(m_Controls.verticalLayout);

  connect(m_Controls.previewButton, SIGNAL(clicked()), this, SLOT(OnPreviewBtnClicked()));
  connect(m_Controls.fetchUrl, SIGNAL(clicked()), this, SLOT(OnFetchBtnClicked()));

  QIcon refreshIcon = QmitkStyleManager::ThemeIcon(QStringLiteral(":/org_mitk_icons/icons/awesome/scalable/actions/view-refresh.svg"));
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
  if (nullptr != tool)
  {
    QString urlString = m_Controls.urlBox->text();
    QUrl url(urlString);
    if (url.isValid() && !url.isLocalFile() && !url.hasFragment() && !url.hasQuery()) // sanity check
    {
      tool->GetOverallInfo(urlString.toStdString());
      // tool->GetOverallInfo("http://localhost:8000/info");
      if (nullptr != tool->m_Parameters)
      {
        std::string response = tool->m_Parameters->name;
        std::vector<mitk::MonaiModelInfo> autoModels = tool->GetAutoSegmentationModels();
        m_Controls.responseNote->setText(QString::fromStdString(response));
        m_Controls.appBox->addItem(QString::fromStdString(response));
        for (auto &model : autoModels)
        {
          m_Controls.modelBox->addItem(QString::fromStdString(model.name));
        }
      }
    }
    else
    {
      MITK_ERROR << "Invalid URL entered: " << urlString.toStdString();
    }
  }
}

void QmitkMonaiLabelToolGUI::OnPreviewBtnClicked()
{
  auto tool = this->GetConnectedToolAs<mitk::MonaiLabelTool>();
  if (nullptr != tool)
  {
    /* QString url = m_Controls.url->text();
    MITK_INFO << "tool found" << url.toStdString();*/

    //tool->GetOverallInfo("https://httpbin.org/get");
    
  }
   //tool->UpdatePreview();
}
