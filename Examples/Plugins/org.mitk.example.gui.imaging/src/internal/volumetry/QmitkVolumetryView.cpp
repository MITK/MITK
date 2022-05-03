/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkVolumetryView.h"

#include "ui_QmitkVolumetryViewControls.h"

#include "mitkImageStatisticsHolder.h"
#include "mitkVolumeCalculator.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

const std::string QmitkVolumetryView::VIEW_ID = "org.mitk.views.volumetry";

QmitkVolumetryView::QmitkVolumetryView() : m_Controls(nullptr), m_ParentWidget(nullptr)
{
}

void QmitkVolumetryView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_ParentWidget = parent;
    // create GUI widgets
    m_Controls = new Ui::QmitkVolumetryViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkVolumetryView::SetFocus()
{
  m_Controls->m_CalcButton->setFocus();
}

void QmitkVolumetryView::CreateConnections()
{
  if (m_Controls)
  {
    connect(
      (QObject *)(m_Controls->m_ThresholdSlider), SIGNAL(valueChanged(int)), this, SLOT(OnThresholdSliderChanged(int)));
    connect((QObject *)(m_Controls->m_CalcButton), SIGNAL(clicked()), this, SLOT(OnCalculateVolume()));
    connect((QObject *)(m_Controls->m_TimeSeriesButton), SIGNAL(clicked()), this, SLOT(OnTimeSeriesButtonClicked()));
    connect((QObject *)(m_Controls->m_SaveCsvButton), SIGNAL(clicked()), this, SLOT(OnSaveCsvButtonClicked()));
  }
}

void QmitkVolumetryView::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*part*/,
                                            const QList<mitk::DataNode::Pointer> &nodes)
{
  m_SelectedDataNode = nullptr;
  if (!nodes.isEmpty() && dynamic_cast<mitk::Image *>(nodes.front()->GetData()))
  {
    m_SelectedDataNode = nodes.front();
    m_ParentWidget->setEnabled(true);
  }

  if (m_SelectedDataNode.IsExpired() || m_SelectedDataNode == m_OverlayNode)
  {
    m_SelectedDataNode = nullptr;
    m_ParentWidget->setEnabled(false);
    return;
  }

  if (m_OverlayNode)
  {
    this->GetDataStorage()->Remove(m_OverlayNode);
    m_OverlayNode = nullptr;
  }

  this->CreateOverlayChild();

  m_Controls->m_CalcButton->setEnabled(false);
  m_Controls->m_TimeSeriesButton->setEnabled(false);
  m_Controls->m_SaveCsvButton->setEnabled(false);
  m_Controls->m_TextEdit->clear();

  mitk::Image *image = dynamic_cast<mitk::Image *>(m_SelectedDataNode.Lock()->GetData());
  image->Update();
  if (image && image->IsInitialized())
  {
    if (image->GetDimension() == 4)
    {
      m_Controls->m_TimeSeriesButton->setEnabled(true);
    }
    else
    {
      m_Controls->m_CalcButton->setEnabled(true);
    }
    int minVal = (int)image->GetStatistics()->GetScalarValue2ndMin();
    int maxVal = (int)image->GetStatistics()->GetScalarValueMaxNoRecompute();
    if (minVal == maxVal)
      --minVal;
    m_Controls->m_ThresholdSlider->setMinimum(minVal);
    m_Controls->m_ThresholdSlider->setMaximum(maxVal);
    m_Controls->m_ThresholdSlider->setEnabled(true);
    this->UpdateSlider();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkVolumetryView::OnCalculateVolume()
{
  auto selectedDataNode = m_SelectedDataNode.Lock();

  if (selectedDataNode.IsNotNull())
  {
    mitk::Image *image = dynamic_cast<mitk::Image *>(selectedDataNode->GetData());
    std::cout << "Dimension:" << image->GetDimension() << std::endl;
    std::cout << "Dimension[3]:" << image->GetDimension(3) << std::endl;
    mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
    volCalc->SetImage(image);
    volCalc->SetThreshold(m_Controls->m_ThresholdSlider->value());
    volCalc->ComputeVolume();
    std::stringstream vs;
    vs << volCalc->GetVolume() << " ml";
    m_Controls->m_Result->setText(vs.str().c_str());
  }
}

void QmitkVolumetryView::OnTimeSeriesButtonClicked()
{
  auto selectedDataNode = m_SelectedDataNode.Lock();

  if (selectedDataNode.IsNotNull())
  {
    mitk::Image *image = dynamic_cast<mitk::Image *>(selectedDataNode->GetData());
    mitk::VolumeCalculator::Pointer volCalc = mitk::VolumeCalculator::New();
    volCalc->SetImage(image);
    volCalc->SetThreshold(m_Controls->m_ThresholdSlider->value());
    volCalc->ComputeVolume();
    std::vector<float> volumes = volCalc->GetVolumes();
    std::stringstream vs;
    int timeStep = 0;
    for (auto it = volumes.begin(); it != volumes.end(); it++)
    {
      vs << timeStep++ << "\t" << *it << std::endl;
    }
    m_Controls->m_TextEdit->setText(vs.str().c_str());
    m_Controls->m_TextEdit->setTabStopWidth(20);
    m_Controls->m_SaveCsvButton->setEnabled(true);
  }
}

const mitk::DataNode *QmitkVolumetryView::GetImageNode() const
{
  return m_SelectedDataNode.Lock();
}

void QmitkVolumetryView::UpdateSlider()
{
  auto selectedDataNode = m_SelectedDataNode.Lock();

  if (selectedDataNode.IsNotNull() && dynamic_cast<mitk::Image *>(selectedDataNode->GetData()))
  {
    int intSliderValue = (int)m_Controls->m_ThresholdSlider->value();
    QString stringSliderValue;
    stringSliderValue.setNum(intSliderValue);
    m_Controls->m_ThresholdLineEdit->setText(stringSliderValue);
  }
}

void QmitkVolumetryView::UpdateSliderLabel()
{
  int sliderValue = atoi(m_Controls->m_ThresholdLineEdit->text().toLatin1());
  m_Controls->m_ThresholdSlider->setValue(sliderValue);
  this->UpdateSlider();
}

void QmitkVolumetryView::OnThresholdSliderChanged(int value)
{
  if (m_OverlayNode)
  {
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(value, 1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    this->UpdateSlider();
  }
}

void QmitkVolumetryView::CreateOverlayChild()
{
  auto selectedDataNode = m_SelectedDataNode.Lock();

  if (selectedDataNode.IsNotNull())
  {
    m_OverlayNode = mitk::DataNode::New();
    mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("volume threshold overlay image");
    m_OverlayNode->SetProperty("reslice interpolation", selectedDataNode->GetProperty("reslice interpolation"));
    m_OverlayNode->SetProperty("name", nameProp);
    m_OverlayNode->SetData(selectedDataNode->GetData());
    m_OverlayNode->SetColor(0.0, 1.0, 0.0);
    m_OverlayNode->SetOpacity(.25);
    int layer = 0;
    selectedDataNode->GetIntProperty("layer", layer);
    m_OverlayNode->SetIntProperty("layer", layer + 1);
    m_OverlayNode->SetLevelWindow(mitk::LevelWindow(m_Controls->m_ThresholdSlider->value(), 1));

    this->GetDataStorage()->Add(m_OverlayNode);
  }
}

mitk::DataNode *QmitkVolumetryView::GetOverlayNode() const
{
  return m_OverlayNode;
}

mitk::Image *QmitkVolumetryView::GetImage() const
{
  return dynamic_cast<mitk::Image *>(m_SelectedDataNode.Lock()->GetData());
}

void QmitkVolumetryView::OnSaveCsvButtonClicked()
{
  static QString lastSavePath = QDir::homePath();

  QString s = QFileDialog::getSaveFileName(this->m_ParentWidget, "Save as..", lastSavePath, "CSV Files (*.csv)");
  /*"Save file dialog"
  "Choose a filename to save under" );*/

  if (!s.isEmpty())
  {
    lastSavePath = s;
    QFile saveFile(s);

    if (saveFile.open(QIODevice::WriteOnly))
    {
      QTextStream stream(&saveFile);
      stream << m_Controls->m_TextEdit->toPlainText().replace('\t', ';');
      saveFile.close();
    }
    else
    {
      // QMessageBox::critical(nullptr,"Save Error!",QString("Saving of CSV failed! Couldn't open output file \"") +
      // saveFile + QString("\""),QMessageBox:Ok,QMessageBox::NoButton);
      // QMessageBox::critical(nullptr,"Save Error!","Saving of CSV failed! Couldn't open output file \"" + saveFile.name()
      // +"\"",QMessageBox::Ok,QMessageBox::NoButton);
    }
  }
}
