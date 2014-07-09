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

#include "QmitkFileWriterOptionsDialog.h"
#include "ui_QmitkFileWriterOptionsDialog.h"

#include "QmitkFileReaderWriterOptionsWidget.h"
#include "mitkIFileWriter.h"

QmitkFileWriterOptionsDialog::QmitkFileWriterOptionsDialog(const QString& filePath,
                                                           const QStringList& labels,
                                                           const QList<mitk::IFileWriter*>& writers,
                                                           QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::QmitkFileWriterOptionsDialog)
  , m_Writers(writers)
{
  ui->setupUi(this);

  const int count = qMin(labels.size(), writers.size());
  bool hasOptions = false;
  for (int i = 0; i < count; ++i)
  {
    ui->m_WriterComboBox->addItem(labels[i]);
    mitk::IFileWriter::Options options = writers[i]->GetOptions();
    if (!options.empty())
    {
      hasOptions = true;
    }
    ui->m_StackedOptionsWidget->addWidget(new QmitkFileReaderWriterOptionsWidget(options));
  }

  if(!hasOptions)
  {
    ui->m_OptionsBox->setVisible(false);
  }

  if (count < 2)
  {
    ui->m_WriterLabel->setVisible(false);
    ui->m_WriterComboBox->setVisible(false);
    ui->m_FilePathLabel->setText(QString("File: %1").arg(filePath));
  }
  else
  {
    ui->m_FilePathLabel->setText(QString("for %1").arg(filePath));
  }

  this->setWindowTitle("File writing options");
}

QmitkFileWriterOptionsDialog::~QmitkFileWriterOptionsDialog()
{
  delete ui;
}

mitk::IFileWriter* QmitkFileWriterOptionsDialog::GetWriter() const
{
  return m_Writers[ui->m_WriterComboBox->currentIndex()];
}

bool QmitkFileWriterOptionsDialog::ReuseOptions() const
{
  return ui->m_ReuseOptionsCheckBox->isChecked();
}

void QmitkFileWriterOptionsDialog::accept()
{
  const int index = ui->m_WriterComboBox->currentIndex();
  m_Writers[index]->SetOptions(
        qobject_cast<QmitkFileReaderWriterOptionsWidget*>(ui->m_StackedOptionsWidget->widget(index))->GetOptions());
  QDialog::accept();
}
