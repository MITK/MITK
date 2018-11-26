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

#include "QmitkFitPlotDataWidget.h"
#include "QmitkFitPlotDataModel.h"
#include "QmitkFitParameterWidget.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

QmitkFitPlotDataWidget::QmitkFitPlotDataWidget(QWidget*)
{
  this->m_Controls.setupUi(this);

  m_InternalModel = new QmitkFitPlotDataModel(this);
  m_Controls.tablePlotData->setModel(m_InternalModel);

  connect(m_Controls.btnCopyResultsToClipboard, SIGNAL(clicked()), this, SLOT(OnClipboardResultsButtonClicked()));
  connect(m_Controls.btnSaveToFile, SIGNAL(clicked()), this, SLOT(OnExportClicked()));
}

const mitk::ModelFitPlotData*
QmitkFitPlotDataWidget::
GetPlotData() const
{
  return m_InternalModel->GetPlotData();
};

void
QmitkFitPlotDataWidget::
SetPlotData(const mitk::ModelFitPlotData* data)
{
  m_InternalModel->SetPlotData(data);
};

const std::string&
QmitkFitPlotDataWidget::
GetXName() const
{
  return m_InternalModel->GetXName();
};

void
QmitkFitPlotDataWidget::
SetXName(const std::string& xName)
{
  m_InternalModel->SetXName(xName);
};

QmitkFitPlotDataWidget::~QmitkFitPlotDataWidget()
{
}

std::string QmitkFitPlotDataWidget::StreamModelToString() const
{
  std::ostringstream stream;
  stream.imbue(std::locale("C"));

  //head line
  const auto colCount = this->m_InternalModel->columnCount();
  for (int col = 0; col < colCount; ++col)
  {
    if (col != 0)
    {
      stream << ",";
    }
    stream << SanatizeString(m_InternalModel->headerData(col, Qt::Horizontal, Qt::DisplayRole).toString().toStdString());
  }
  stream << std::endl;

  //content
  const auto rowCount = this->m_InternalModel->rowCount();
  for (int row = 0; row < rowCount; ++row)
  {
    for (int col = 0; col < colCount; ++col)
    {
      QModelIndex index = this->m_InternalModel->index(row, col);
      if (col != 0)
      {
        stream << ",";
      }
      stream << SanatizeString(m_InternalModel->data(index, Qt::DisplayRole).toString().toStdString());
    }
    stream << std::endl;
  }

  return stream.str();
}

void QmitkFitPlotDataWidget::OnClipboardResultsButtonClicked() const
{
  QApplication::clipboard()->setText(QString::fromStdString(this->StreamModelToString()), QClipboard::Clipboard);

}

void QmitkFitPlotDataWidget::OnExportClicked() const
{
  QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save plot data to csv file"));

  if (fileName.isEmpty())
  {
    QMessageBox::critical(nullptr, tr("No file selected!"),
      tr("Cannot export pixel dump. Please selected a file."));
  }
  else
  {
    std::ofstream file;

    std::ios_base::openmode iOpenFlag = std::ios_base::out | std::ios_base::trunc;
    file.open(fileName.toStdString().c_str(), iOpenFlag);

    if (!file.is_open())
    {
      QMessageBox::critical(nullptr, tr("Cannot create/open selected file!"),
        tr("Cannot open or create the selected file. Export will be aborted. Selected file name: ") +
        fileName);
      return;
    }

    file << this->StreamModelToString();

    file.close();
  }

}
