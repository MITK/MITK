#include "QmitkResectionToolGUI.h"

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkResectionToolGUI, "");

QmitkResectionToolGUI::QmitkResectionToolGUI() :
  QmitkToolGUI()
{
  this->setContentsMargins(0, 0, 0, 0);
  QBoxLayout* mainLayout = new QHBoxLayout(this);

  m_CutInsideButton = new QPushButton(tr("Cut Inside"), this);
  mainLayout->addWidget(m_CutInsideButton);
  connect(m_CutInsideButton, &QPushButton::clicked,
      [this](bool checked) {m_ResectionTool->Resect(mitk::ResectionTool::ResectionType::INSIDE); });

  m_CutOutsideButton = new QPushButton(tr("Cut Outside"), this);
  mainLayout->addWidget(m_CutOutsideButton);
  connect(m_CutOutsideButton, &QPushButton::clicked,
      [this](bool checked) {m_ResectionTool->Resect(mitk::ResectionTool::ResectionType::OUTSIDE); });

  m_DivideButton = new QPushButton(tr("Divide"), this);
  mainLayout->addWidget(m_DivideButton);
  connect(m_DivideButton, &QPushButton::clicked,
    [this](bool checked) {m_ResectionTool->Resect(mitk::ResectionTool::ResectionType::DIVIDE); });

  connect(this, &QmitkResectionToolGUI::NewToolAssociated,
      this, &QmitkResectionToolGUI::OnNewToolAssociated);
}

QmitkResectionToolGUI::~QmitkResectionToolGUI()
{
  if (m_ResectionTool.IsNotNull()) {
    m_ResectionTool->stateChanged -= mitk::MessageDelegate1<QmitkResectionToolGUI, mitk::ResectionTool::ResectionState>(this, &QmitkResectionToolGUI::OnToolStateChanged);
  }
}

void QmitkResectionToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_ResectionTool.IsNotNull()) {
    m_ResectionTool->stateChanged -= mitk::MessageDelegate1<QmitkResectionToolGUI, mitk::ResectionTool::ResectionState>(this, &QmitkResectionToolGUI::OnToolStateChanged);
  }

  m_ResectionTool = dynamic_cast<mitk::ResectionTool*>(tool);
  OnToolStateChanged(m_ResectionTool->GetState());

  if (m_ResectionTool.IsNotNull()) {
    m_ResectionTool->stateChanged += mitk::MessageDelegate1<QmitkResectionToolGUI, mitk::ResectionTool::ResectionState>(this, &QmitkResectionToolGUI::OnToolStateChanged);
  }
}

void QmitkResectionToolGUI::OnToolStateChanged(mitk::ResectionTool::ResectionState state)
{
  bool enable = state == mitk::ResectionTool::ResectionState::CONTOUR_AVAILABLE;
  m_CutInsideButton->setEnabled(enable);
  m_CutOutsideButton->setEnabled(enable);
  m_DivideButton->setEnabled(enable);
}
