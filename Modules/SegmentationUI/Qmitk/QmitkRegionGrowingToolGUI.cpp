#include "QmitkRegionGrowingToolGUI.h"


MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkRegionGrowingToolGUI, "")

QmitkRegionGrowingToolGUI::QmitkRegionGrowingToolGUI()
  : QmitkToolGUI()
{
  m_Controls.setupUi(this);

  connect(this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)));
}

QmitkRegionGrowingToolGUI::~QmitkRegionGrowingToolGUI()
{
  if (m_RegionGrowingTool.IsNotNull())
  {
    m_RegionGrowingTool->thresholdsChanged -=
      mitk::MessageDelegate2<QmitkRegionGrowingToolGUI, double, double>(this, &QmitkRegionGrowingToolGUI::onThresholdsChanged);
  }
}

void QmitkRegionGrowingToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_RegionGrowingTool.IsNotNull())
  {
    m_RegionGrowingTool->thresholdsChanged -=
      mitk::MessageDelegate2<QmitkRegionGrowingToolGUI, double, double>(this, &QmitkRegionGrowingToolGUI::onThresholdsChanged);
  }

  m_RegionGrowingTool = dynamic_cast<mitk::RegionGrowingTool*>(m_Tool.GetPointer());

  if (m_RegionGrowingTool.IsNotNull())
  {
    m_RegionGrowingTool->thresholdsChanged +=
      mitk::MessageDelegate2<QmitkRegionGrowingToolGUI, double, double>(this, &QmitkRegionGrowingToolGUI::onThresholdsChanged);
  }
}

void QmitkRegionGrowingToolGUI::onThresholdsChanged(double lowThreshold, double highThreshold)
{
  m_Controls.lowThresholdValue->setText(QString::number(lowThreshold));
  m_Controls.highThresholdValue->setText(QString::number(highThreshold));
}
