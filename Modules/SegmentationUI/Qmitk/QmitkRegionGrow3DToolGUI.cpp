/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "QmitkRegionGrow3DToolGUI.h"
#include "QmitkNewSegmentationDialog.h"
#include <QBoxLayout>

MITK_TOOL_GUI_MACRO( , QmitkRegionGrow3DToolGUI, "")

QmitkRegionGrow3DToolGUI::QmitkRegionGrow3DToolGUI():QmitkToolGUI()
{

  QBoxLayout* mainLayout = new QVBoxLayout( this );

  QBoxLayout* layout1 = new QHBoxLayout();

  m_brightRadio = new QRadioButton("bright structure");
  m_darkRadio = new QRadioButton("dark structure");
  m_brightRadio->setChecked(true);

  connect(m_brightRadio, SIGNAL(toggled(bool)), this, SLOT(OnGrowingDirectionChanged(bool)) );
  connect(m_darkRadio, SIGNAL(toggled(bool)), this, SLOT(OnGrowingDirectionChanged(bool)) );

  layout1->addWidget(m_brightRadio);
  layout1->addWidget(m_darkRadio);

  m_SetSeedButton = new QPushButton(this);
  m_SetSeedButton->setCheckable(true);
  m_SetSeedButton->setFixedSize(32,32);
  const QIcon iconAdd(":/QtWidgetsExt/btnSetSeedPoint.xpm");
  m_SetSeedButton->setIcon(iconAdd);
  connect (m_SetSeedButton, SIGNAL(toggled(bool)), this, SLOT(OnSeedButtonToggled(bool)) );


  QBoxLayout* layout3 = new QHBoxLayout();

  m_DecreaseTHButton = new QPushButton("-", this);
  m_DecreaseTHButton->setMaximumSize(25,25);
  connect(m_DecreaseTHButton, SIGNAL(clicked()), this, SLOT(DecreaseSlider()));
  m_DecreaseTHButton->setEnabled(false);
  m_IncreaseTHButton = new QPushButton("+", this);
  m_IncreaseTHButton->setMaximumSize(25,25);
  connect(m_IncreaseTHButton, SIGNAL(clicked()), this, SLOT(IncreaseSlider()));
  m_IncreaseTHButton->setEnabled(false);
  m_ThSlider = new QSlider(Qt::Horizontal, this);
  connect(m_ThSlider, SIGNAL(valueChanged(int)), this, SLOT(SliderValueChanged(int)) );
  m_ThSlider->setEnabled(false);

  layout3->addWidget(m_DecreaseTHButton);
  layout3->addWidget(m_ThSlider);
  layout3->addWidget(m_IncreaseTHButton);

  m_ThValue = new QLabel("0", this);
  m_ThValue->setAlignment(Qt::AlignHCenter);
  m_ConfirmButton = new QPushButton("Confirm Segmentation", this);
  connect(m_ConfirmButton, SIGNAL(clicked()), this, SLOT(OnConfirmButtonClicked()));
  m_ConfirmButton->setEnabled(false);

  mainLayout->addLayout(layout1);
  mainLayout->addWidget(m_SetSeedButton);
  mainLayout->addLayout(layout3);
  mainLayout->addWidget(m_ThValue);
  mainLayout->addWidget(m_ConfirmButton);

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}

QmitkRegionGrow3DToolGUI::~QmitkRegionGrow3DToolGUI()
{
  if (m_RegionGrow3DTool.IsNotNull())
  {
    m_RegionGrow3DTool->LowerThresholdValueChanged -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::OnLowerThresholdValueChanged);
    m_RegionGrow3DTool->UpperThresholdValueChanged -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::OnUpperThresholdValueChanged);
    m_RegionGrow3DTool->SliderValueChanged -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::SliderValueChanged);
    m_RegionGrow3DTool->SeedButtonToggled -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, bool>(this, &QmitkRegionGrow3DToolGUI::OnSeedButtonToggled);
  }
  QApplication::restoreOverrideCursor();
}

void QmitkRegionGrow3DToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  if (m_RegionGrow3DTool.IsNotNull())
  {
    m_RegionGrow3DTool->LowerThresholdValueChanged -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::OnLowerThresholdValueChanged);
    m_RegionGrow3DTool->UpperThresholdValueChanged -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::OnUpperThresholdValueChanged);
    m_RegionGrow3DTool->SliderValueChanged -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::SliderValueChanged);
    m_RegionGrow3DTool->SeedButtonToggled -= mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, bool>(this, &QmitkRegionGrow3DToolGUI::OnSeedButtonToggled);
  }
  m_RegionGrow3DTool = dynamic_cast<mitk::RegionGrow3DTool*> (tool);
  if (m_RegionGrow3DTool)
  {
    m_RegionGrow3DTool->LowerThresholdValueChanged += mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::OnLowerThresholdValueChanged);
    m_RegionGrow3DTool->UpperThresholdValueChanged += mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::OnUpperThresholdValueChanged);
    m_RegionGrow3DTool->SliderValueChanged += mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, int>(this, &QmitkRegionGrow3DToolGUI::SliderValueChanged);
    m_RegionGrow3DTool->SeedButtonToggled += mitk::MessageDelegate1<QmitkRegionGrow3DToolGUI, bool>(this, &QmitkRegionGrow3DToolGUI::OnSeedButtonToggled);

  }
}

void QmitkRegionGrow3DToolGUI::OnGrowingDirectionChanged(bool flag)
{
  if (flag)
  {
    if (m_brightRadio->isChecked())
      m_RegionGrow3DTool->SetCurrentRGDirectionIsUpwards(false);
    if (m_darkRadio->isChecked())
      m_RegionGrow3DTool->SetCurrentRGDirectionIsUpwards(true);
  }
  m_DecreaseTHButton->setEnabled(false);
  m_IncreaseTHButton->setEnabled(false);
  m_ThSlider->setEnabled(false);
  m_ConfirmButton->setEnabled(false);
}

void QmitkRegionGrow3DToolGUI::OnLowerThresholdValueChanged(int lower)
{
  m_ThSlider->setMinimum(lower);
}

void QmitkRegionGrow3DToolGUI::OnUpperThresholdValueChanged(int upper)
{
  m_ThSlider->setMaximum(upper);
}

void QmitkRegionGrow3DToolGUI::IncreaseSlider()
{
  //moves the slider one step to the right, when the "+"-button is pressed
  if (m_ThSlider->value() != m_ThSlider->maximum())
  {
    int newValue = m_ThSlider->value() + 1;
    m_ThSlider->setValue(newValue);
  }
}

void QmitkRegionGrow3DToolGUI::DecreaseSlider()
{
  //moves the slider one step to the left, when the "-"-button is pressed
  if (m_ThSlider->value() != m_ThSlider->minimum())
  {
    int newValue = m_ThSlider->value() - 1;
    m_ThSlider->setValue(newValue);
  }
}

void QmitkRegionGrow3DToolGUI::OnSeedButtonToggled(bool toggled)
{
  if (toggled)
  {
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  if (m_RegionGrow3DTool.IsNotNull())
  {
    m_RegionGrow3DTool->SetSeedPoint(toggled);

    if (toggled != m_SetSeedButton->isChecked())
    {
      m_SetSeedButton->setChecked(toggled);

      //enable gui components
      m_DecreaseTHButton->setEnabled(!toggled);
      m_IncreaseTHButton->setEnabled(!toggled);
      m_ThSlider->setEnabled(!toggled);
      m_ConfirmButton->setEnabled(!toggled);
    }
  }
}

void QmitkRegionGrow3DToolGUI::OnConfirmButtonClicked()
{
  if (m_RegionGrow3DTool.IsNotNull())
  {
    QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog(this);
    int returnValue = dialog->exec();

    std::string name = dialog->GetSegmentationName().toLocal8Bit().data();
    mitk::Color color = dialog->GetColor();

    if (returnValue != QDialog::Rejected)
    {
      m_RegionGrow3DTool->ConfirmSegmentation(name, color);
    }
    else
    {
      m_RegionGrow3DTool->CancelSegmentation();
    }
  }
}


void QmitkRegionGrow3DToolGUI::SliderValueChanged(int value)
{
  if (m_RegionGrow3DTool.IsNotNull())
  {
    m_ThSlider->setValue(value);
    m_ThValue->setNum(value);
    m_RegionGrow3DTool->ChangeLevelWindow(value);
  }
}
