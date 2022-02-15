/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTransferFunctionGeneratorWidget.h"

#include <QFileDialog>
#include <QFontMetrics>
#include <mitkImageStatisticsHolder.h>
#include <mitkRenderingManager.h>
#include <mitkTransferFunctionInitializer.h>
#include <mitkTransferFunctionProperty.h>
#include <mitkUnstructuredGrid.h>

#include <mitkTransferFunctionPropertySerializer.h>

#include <vtkUnstructuredGrid.h>

QmitkTransferFunctionGeneratorWidget::QmitkTransferFunctionGeneratorWidget(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f), deltaScale(1.0), deltaMax(1024), deltaMin(1)
{
  histoGramm = nullptr;

  this->setupUi(this);

  // LevelWindow Tab
  {
    connect(m_CrossLevelWindow, SIGNAL(SignalDeltaMove(int, int)), this, SLOT(OnDeltaLevelWindow(int, int)));
  }

  // Threshold Tab
  {
    connect(m_CrossThreshold, SIGNAL(SignalDeltaMove(int, int)), this, SLOT(OnDeltaThreshold(int, int)));
    thDelta = 100;
  }

  // Presets Tab
  {
    m_TransferFunctionComboBox->setVisible(false);

    connect(m_TransferFunctionComboBox, SIGNAL(activated(int)), this, SIGNAL(SignalTransferFunctionModeChanged(int)));
    connect(m_TransferFunctionComboBox, SIGNAL(activated(int)), this, SLOT(OnPreset(int)));

    connect(m_SavePreset, SIGNAL(clicked()), this, SLOT(OnSavePreset()));

    connect(m_LoadPreset, SIGNAL(clicked()), this, SLOT(OnLoadPreset()));
  }

  presetFileName = ".";
}

int QmitkTransferFunctionGeneratorWidget::AddPreset(const QString &presetName)
{
  m_TransferFunctionComboBox->setVisible(true);

  m_TransferFunctionComboBox->addItem(presetName);
  return m_TransferFunctionComboBox->count() - 1;
}

void QmitkTransferFunctionGeneratorWidget::SetPresetsTabEnabled(bool enable)
{
  m_PresetTab->setEnabled(enable);
}

void QmitkTransferFunctionGeneratorWidget::SetThresholdTabEnabled(bool enable)
{
  m_ThresholdTab->setEnabled(enable);
}

void QmitkTransferFunctionGeneratorWidget::SetBellTabEnabled(bool enable)
{
  m_BellTab->setEnabled(enable);
}

void QmitkTransferFunctionGeneratorWidget::OnSavePreset()
{
  if (tfpToChange.IsNull())
    return;

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  presetFileName = QFileDialog::getSaveFileName(
    this, "Choose a filename to save the transfer function", presetFileName, "Transferfunction (*.xml)");

  if (!presetFileName.endsWith(".xml"))
    presetFileName.append(".xml");

  MITK_INFO << "Saving Transferfunction under path: " << presetFileName.toStdString();

  if (mitk::TransferFunctionPropertySerializer::SerializeTransferFunction(presetFileName.toLatin1(), tf))
  {
    QFontMetrics metrics(m_InfoPreset->font());
    QString text = metrics.elidedText(presetFileName, Qt::ElideMiddle, m_InfoPreset->width());
    m_InfoPreset->setText(QString("saved ") + text);
  }
  else
  {
    m_InfoPreset->setText(QString("saving failed"));
  }
}

void QmitkTransferFunctionGeneratorWidget::OnLoadPreset()
{
  if (tfpToChange.IsNull())
    return;

  presetFileName = QFileDialog::getOpenFileName(
    this, "Choose a file to open the transfer function from", presetFileName, "Transferfunction (*.xml)");

  MITK_INFO << "Loading Transferfunction from path: " << presetFileName.toStdString();

  mitk::TransferFunction::Pointer tf =
    mitk::TransferFunctionPropertySerializer::DeserializeTransferFunction(presetFileName.toLatin1());

  if (tf.IsNotNull())
  {
    tfpToChange->SetValue(tf);

    QFontMetrics metrics(m_InfoPreset->font());
    QString text = metrics.elidedText(presetFileName, Qt::ElideMiddle, m_InfoPreset->width());
    m_InfoPreset->setText(QString("loaded ") + text);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    emit SignalUpdateCanvas();
  }
}

void QmitkTransferFunctionGeneratorWidget::OnPreset(int mode)
{
  // first item is only information
  if (--mode == -1)
    return;

  m_InfoPreset->setText(QString("selected ") + m_TransferFunctionComboBox->currentText());

  // revert to first item
  m_TransferFunctionComboBox->setCurrentIndex(0);
}

static double transformationGlocke(double x)
{
  double z = 0.1;

  double a = 2 - 2 * z;

  double b = 2 * z - 1;

  x = a * x + b;

  return x;
}

static double stepFunctionGlocke(double x)
{
  x = 1 - (2 * x - 1.0);       // map [0.5;1] to [0,1]
  x = x * (3 * x - 2 * x * x); // apply smoothing function

  x = x * x;

  return x;
}

double QmitkTransferFunctionGeneratorWidget::ScaleDelta(int d) const
{
  return deltaScale * (double)d;
}

void QmitkTransferFunctionGeneratorWidget::OnDeltaLevelWindow(int dx, int dy) // bell
{
  if (tfpToChange.IsNull())
    return;

  thPos += ScaleDelta(dx);
  thDelta -= ScaleDelta(dy);

  if (thDelta < deltaMin)
    thDelta = deltaMin;

  if (thDelta > deltaMax)
    thDelta = deltaMax;

  if (thPos < histoMinimum)
    thPos = histoMinimum;

  if (thPos > histoMaximum)
    thPos = histoMaximum;

  std::stringstream ss;

  ss << "Click on the cross and move the mouse"
     << "\n"
     << "\n"
     << "center at " << thPos << "\n"
     << "width " << thDelta * 2;

  m_InfoLevelWindow->setText(QString(ss.str().c_str()));

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  // grayvalue->opacity
  {
    vtkPiecewiseFunction *f = tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();

    for (int r = 0; r <= 6; r++)
    {
      double relPos = (r / 6.0) * 0.5 + 0.5;
      f->AddPoint(thPos + thDelta * (-transformationGlocke(relPos)), stepFunctionGlocke(relPos));
      f->AddPoint(thPos + thDelta * (transformationGlocke(relPos)), stepFunctionGlocke(relPos));
    }

    f->Modified();
  }

  // gradient at grayvalue->opacity
  {
    vtkPiecewiseFunction *f = tf->GetGradientOpacityFunction();
    f->RemoveAllPoints();

    f->AddPoint(0, 1.0);
    f->Modified();
  }

  tf->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalUpdateCanvas();
}

static double stepFunctionThreshold(double x)
{
  x = 0.5 * x + 0.5;           // map [-1;1] to [0,1]
  x = x * (3 * x - 2 * x * x); // apply smoothing function

  x = x * x;

  return x;
}

void QmitkTransferFunctionGeneratorWidget::OnDeltaThreshold(int dx, int dy) // LEVELWINDOW
{
  if (tfpToChange.IsNull())
    return;

  thPos += ScaleDelta(dx);
  thDelta += ScaleDelta(dy);

  if (thDelta < deltaMin)
    thDelta = deltaMin;

  if (thDelta > deltaMax)
    thDelta = deltaMax;

  if (thPos < histoMinimum)
    thPos = histoMinimum;

  if (thPos > histoMaximum)
    thPos = histoMaximum;

  std::stringstream ss;

  ss << "Click on the cross and move the mouse"
     << "\n"
     << "\n"
     << "threshold at " << thPos << "\n"
     << "width " << thDelta * 2;

  m_InfoThreshold->setText(QString(ss.str().c_str()));

  mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

  // grayvalue->opacity
  {
    vtkPiecewiseFunction *f = tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();

    for (int r = 1; r <= 4; r++)
    {
      double relPos = r / 4.0;
      f->AddPoint(thPos + thDelta * (-relPos), stepFunctionThreshold(-relPos));
      f->AddPoint(thPos + thDelta * (relPos), stepFunctionThreshold(relPos));
    }
    f->Modified();
  }

  // gradient at grayvalue->opacity
  {
    vtkPiecewiseFunction *f = tf->GetGradientOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint(0, 1.0);
    f->Modified();
  }

  tf->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  emit SignalUpdateCanvas();
}

QmitkTransferFunctionGeneratorWidget::~QmitkTransferFunctionGeneratorWidget()
{
}

void QmitkTransferFunctionGeneratorWidget::SetDataNode(mitk::DataNode *node, mitk::TimeStepType timestep)
{
  histoGramm = nullptr;

  if (node)
  {
    tfpToChange = dynamic_cast<mitk::TransferFunctionProperty *>(node->GetProperty("TransferFunction"));

    if (!tfpToChange)
      node->SetProperty("TransferFunction", tfpToChange = mitk::TransferFunctionProperty::New());

    mitk::TransferFunction::Pointer tf = tfpToChange->GetValue();

    if (mitk::Image *image = dynamic_cast<mitk::Image *>(node->GetData()))
    {
      mitk::Image::Pointer inputImage = image;
      if (image->GetTimeSteps() > 1)
      {
        if (!image->GetTimeGeometry()->IsValidTimeStep(timestep))
        {
          return;
        }
        mitk::ImageTimeSelector::Pointer timeselector = mitk::ImageTimeSelector::New();
        timeselector->SetInput(image);
        timeselector->SetTimeNr(timestep);
        timeselector->UpdateLargestPossibleRegion();
        inputImage = timeselector->GetOutput();
      }

      mitk::ImageStatisticsHolder *statistics = inputImage->GetStatistics();
      histoMinimum = statistics->GetScalarValueMin();
      histoMaximum = statistics->GetScalarValueMax();
    }
    else if (mitk::UnstructuredGrid *grid = dynamic_cast<mitk::UnstructuredGrid *>(node->GetData()))
    {
      double *range = grid->GetVtkUnstructuredGrid()->GetScalarRange();
      histoMinimum = range[0];
      histoMaximum = range[1];
      double histoRange = histoMaximum - histoMinimum;
      deltaMax = histoRange / 4.0;
      deltaMin = histoRange / 400.0;
      deltaScale = histoRange / 1024.0;
    }
    else
    {
      MITK_WARN << "QmitkTransferFunctonGeneratorWidget does not support " << node->GetData()->GetNameOfClass()
                << " instances";
    }

    thPos = (histoMinimum + histoMaximum) / 2.0;
  }
  else
  {
    tfpToChange = nullptr;
    m_InfoPreset->setText(QString(""));
  }
}
