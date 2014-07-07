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

#include "QmitkFileReaderOptionsDialog.h"
#include "ui_QmitkFileReaderOptionsDialog.h"

#include "QmitkFileReaderWriterOptionsWidget.h"
#include "mitkIFileReader.h"

QmitkFileReaderOptionsDialog::QmitkFileReaderOptionsDialog(const QString& filePath,
                                                           const QStringList& labels,
                                                           const QList<mitk::IFileReader*>& readers,
                                                           QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::QmitkFileReaderOptionsDialog)
  , m_Readers(readers)
{
  ui->setupUi(this);

  const int count = qMin(labels.size(), readers.size());
  bool hasOptions = false;
  for (int i = 0; i < count; ++i)
  {
    ui->m_ReaderComboBox->addItem(labels[i]);
    mitk::IFileReader::Options options = readers[i]->GetOptions();
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
    ui->m_ReaderLabel->setVisible(false);
    ui->m_ReaderComboBox->setVisible(false);
    ui->m_FilePathLabel->setText(QString("File: %1").arg(filePath));
  }
  else
  {
    ui->m_FilePathLabel->setText(QString("for %1").arg(filePath));
  }

  this->setWindowTitle("File Reading Options");
}

QmitkFileReaderOptionsDialog::~QmitkFileReaderOptionsDialog()
{
  delete ui;
}

mitk::IFileReader* QmitkFileReaderOptionsDialog::GetReader() const
{
  return m_Readers[ui->m_ReaderComboBox->currentIndex()];
}

bool QmitkFileReaderOptionsDialog::ReuseOptions() const
{
  return ui->m_ReuseOptionsCheckBox->isChecked();
}

void QmitkFileReaderOptionsDialog::accept()
{
  const int index = ui->m_ReaderComboBox->currentIndex();
  m_Readers[index]->SetOptions(
        qobject_cast<QmitkFileReaderWriterOptionsWidget*>(ui->m_StackedOptionsWidget->widget(index))->GetOptions());
  QDialog::accept();
}
