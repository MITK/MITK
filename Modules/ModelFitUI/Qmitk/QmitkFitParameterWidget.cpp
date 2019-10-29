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

#include "QmitkFitParameterWidget.h"
#include "QmitkFitParameterModel.h"

#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>

QmitkFitParameterWidget::QmitkFitParameterWidget(QWidget*)
{
  this->m_Controls.setupUi(this);

  m_InternalModel = new QmitkFitParameterModel(this);
  m_Controls.tableFitParameter->setModel(m_InternalModel);

  connect(m_Controls.btnCopyResultsToClipboard, SIGNAL(clicked()), this, SLOT(OnClipboardResultsButtonClicked()));
  connect(m_Controls.btnSaveToFile, SIGNAL(clicked()), this, SLOT(OnExportClicked()));
}

const QmitkFitParameterWidget::FitVectorType&
QmitkFitParameterWidget::
getFits() const
{
  return m_InternalModel->getFits();
};

mitk::Point3D
QmitkFitParameterWidget::
getCurrentPosition() const
{
  return m_InternalModel->getCurrentPosition();
};

const mitk::PointSet*
QmitkFitParameterWidget::
getPositionBookmarks() const
{
  return m_InternalModel->getPositionBookmarks();
};

void
QmitkFitParameterWidget::
setFits(const FitVectorType& fits)
{
  m_InternalModel->setFits(fits);
};

void
QmitkFitParameterWidget::
setCurrentPosition(const mitk::Point3D& currentPos)
{
  m_InternalModel->setCurrentPosition(currentPos);
};

void
QmitkFitParameterWidget::
setPositionBookmarks(const mitk::PointSet* bookmarks)
{
  m_InternalModel->setPositionBookmarks(bookmarks);
};

QmitkFitParameterWidget::~QmitkFitParameterWidget()
{
}

std::string SanatizeString(std::string str)
{
  std::replace(std::begin(str), std::end(str), ',', ' ');
  std::replace(std::begin(str), std::end(str), '\n', ' ');
  std::replace(std::begin(str), std::end(str), '\r', ' ');
  return str;
}

std::string QmitkFitParameterWidget::streamModelToString() const
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
    QModelIndex index = this->m_InternalModel->index(row, 0);

    const auto childCount = this->m_InternalModel->rowCount(index);

    if (childCount == 0)
    {
      for (int col = 0; col < colCount; ++col)
      {
        if (col != 0)
        {
          stream << ",";
        }
        stream << SanatizeString(m_InternalModel->data(index.siblingAtColumn(col), Qt::DisplayRole).toString().toStdString());
      }
      stream << std::endl;
    }
    else
    {
      mitkThrow() << "Missing implementation for multiple fits.";
      //TODO FIT REFACTOR
    }
  }

  return stream.str();
}

void QmitkFitParameterWidget::OnClipboardResultsButtonClicked() const
{
  QApplication::clipboard()->setText(QString::fromStdString(this->streamModelToString()), QClipboard::Clipboard);

}

void QmitkFitParameterWidget::OnExportClicked() const
{
  QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save fit parameter to csv file"));

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

    file << this->streamModelToString();

    file.close();
  }

}
