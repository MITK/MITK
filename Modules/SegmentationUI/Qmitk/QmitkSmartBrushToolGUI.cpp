#include "QmitkSmartBrushToolGUI.h"

#include <qlabel.h>
#include <qslider.h>
#include <qlayout.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkSmartBrushToolGUI, "");

QmitkSmartBrushToolGUI::QmitkSmartBrushToolGUI() :
  QmitkToolGUI()
{
  static const int TEXT_LABEL_SIZE = 50;
  static const int VALUE_LABEL_SIZE = 30;
  this->setContentsMargins(0, 0, 0, 0);
  QBoxLayout* mainLayout = new QVBoxLayout(this);

  QBoxLayout* radiusLayout = new QHBoxLayout();

  QLabel* radLabel = new QLabel(tr("Radius: "), this);
  radLabel->setFixedWidth(TEXT_LABEL_SIZE);
  radiusLayout->addWidget(radLabel);

  m_RadiusLabel = new QLabel("10", this);
  m_RadiusLabel->setFixedWidth(VALUE_LABEL_SIZE);
  radiusLayout->addWidget(m_RadiusLabel);

  m_RadiusSlider = new QSlider(Qt::Horizontal, this);
  m_RadiusSlider->setMinimum(1);
  m_RadiusSlider->setMaximum(50);
  m_RadiusSlider->setPageStep(1);
  m_RadiusSlider->setValue(10);
  connect(m_RadiusSlider, &QSlider::valueChanged,
      this, &QmitkSmartBrushToolGUI::OnRadiusSliderChanged);
  radiusLayout->addWidget(m_RadiusSlider);

  mainLayout->addLayout(radiusLayout);

  QBoxLayout* sensitivityLayout = new QHBoxLayout();

  QLabel* sensLabel = new QLabel(tr("Sensitivity: "), this);
  sensLabel->setFixedWidth(TEXT_LABEL_SIZE);
  sensitivityLayout->addWidget(sensLabel);

  m_SensitivityLabel = new QLabel("50%", this);
  m_SensitivityLabel->setFixedWidth(VALUE_LABEL_SIZE);
  sensitivityLayout->addWidget(m_SensitivityLabel);

  m_SensitivitySlider = new QSlider(Qt::Horizontal, this);
  m_SensitivitySlider->setMinimum(0);
  m_SensitivitySlider->setMaximum(100);
  m_SensitivitySlider->setPageStep(1);
  m_SensitivitySlider->setValue(50);
  connect(m_SensitivitySlider, &QSlider::valueChanged,
      this, &QmitkSmartBrushToolGUI::OnSensitivitySliderChanged);
  sensitivityLayout->addWidget(m_SensitivitySlider);

  mainLayout->addLayout(sensitivityLayout);

  connect(this, &QmitkSmartBrushToolGUI::NewToolAssociated,
      this, &QmitkSmartBrushToolGUI::OnNewToolAssociated);
}

QmitkSmartBrushToolGUI::~QmitkSmartBrushToolGUI()
{
  if (m_SmartBrushTool.IsNotNull()) {
    m_SmartBrushTool->radiusChanged -= mitk::MessageDelegate1<QmitkSmartBrushToolGUI, int>(this, &QmitkSmartBrushToolGUI::OnRadiusChanged);
    m_SmartBrushTool->sensitivityChanged -= mitk::MessageDelegate1<QmitkSmartBrushToolGUI, int>(this, &QmitkSmartBrushToolGUI::OnSensitivityChanged);
  }
}

void QmitkSmartBrushToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_SmartBrushTool.IsNotNull()) {
    m_SmartBrushTool->radiusChanged -= mitk::MessageDelegate1<QmitkSmartBrushToolGUI, int>(this, &QmitkSmartBrushToolGUI::OnRadiusChanged);
    m_SmartBrushTool->sensitivityChanged -= mitk::MessageDelegate1<QmitkSmartBrushToolGUI, int>(this, &QmitkSmartBrushToolGUI::OnSensitivityChanged);
  }

  m_SmartBrushTool = dynamic_cast<mitk::SmartBrushTool*>(tool);

  if (m_SmartBrushTool.IsNotNull()) {
    m_SmartBrushTool->radiusChanged += mitk::MessageDelegate1<QmitkSmartBrushToolGUI, int>(this, &QmitkSmartBrushToolGUI::OnRadiusChanged);
    m_SmartBrushTool->sensitivityChanged += mitk::MessageDelegate1<QmitkSmartBrushToolGUI, int>(this, &QmitkSmartBrushToolGUI::OnSensitivityChanged);
  }
}

void QmitkSmartBrushToolGUI::OnRadiusSliderChanged(int value)
{
  if (m_SmartBrushTool.IsNotNull()) {
    m_SmartBrushTool->SetRadius(value);
  }

  VisualizeRadius(value);
}

void QmitkSmartBrushToolGUI::VisualizeRadius(int radius)
{
  m_RadiusLabel->setText(QString::number(radius));
}

void QmitkSmartBrushToolGUI::OnRadiusChanged(int current)
{
  m_RadiusSlider->setValue(current);
}

void QmitkSmartBrushToolGUI::OnSensitivitySliderChanged(int value)
{
  if (m_SmartBrushTool.IsNotNull()) {
    m_SmartBrushTool->SetSensitivity(value);
  }

  VisualizeSensitivity(value);
}

void QmitkSmartBrushToolGUI::VisualizeSensitivity(int sens)
{
  m_SensitivityLabel->setText(QString::number(sens) + "%");
}

void QmitkSmartBrushToolGUI::OnSensitivityChanged(int current)
{
  m_SensitivitySlider->setValue(current);
}
