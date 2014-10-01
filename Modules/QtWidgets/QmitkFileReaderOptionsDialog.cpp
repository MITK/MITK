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

QmitkFileReaderOptionsDialog::QmitkFileReaderOptionsDialog(mitk::IOUtil::LoadInfo& loadInfo,
                                                           QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::QmitkFileReaderOptionsDialog)
  , m_LoadInfo(loadInfo)

{
  ui->setupUi(this);

  m_ReaderItems = loadInfo.m_ReaderSelector.Get();

  bool hasOptions = false;
  int selectedIndex = 0;
  long selectedReaderId = loadInfo.m_ReaderSelector.GetSelectedId();
  int i = 0;
  for (std::vector<mitk::FileReaderSelector::Item>::const_reverse_iterator iter = m_ReaderItems.rbegin(),
       iterEnd = m_ReaderItems.rend(); iter != iterEnd; ++iter)
  {
    ui->m_ReaderComboBox->addItem(QString::fromStdString(iter->GetDescription()));
    mitk::IFileReader::Options options = iter->GetReader()->GetOptions();
    if (!options.empty())
    {
      hasOptions = true;
    }
    ui->m_StackedOptionsWidget->addWidget(new QmitkFileReaderWriterOptionsWidget(options));
    if (iter->GetServiceId() == selectedReaderId)
    {
      selectedIndex = i;
    }
  }
  ui->m_ReaderComboBox->setCurrentIndex(selectedIndex);

  if(!hasOptions)
  {
    ui->m_OptionsBox->setVisible(false);
  }

  if (m_ReaderItems.size() < 2)
  {
    ui->m_ReaderLabel->setVisible(false);
    ui->m_ReaderComboBox->setVisible(false);
    ui->m_FilePathLabel->setText(QString("File: %1").arg(QString::fromStdString(loadInfo.m_Path)));
  }
  else
  {
    ui->m_FilePathLabel->setText(QString("for %1").arg(QString::fromStdString(loadInfo.m_Path)));
  }

  this->setWindowTitle("File reading options");
}

QmitkFileReaderOptionsDialog::~QmitkFileReaderOptionsDialog()
{
  delete ui;
}

bool QmitkFileReaderOptionsDialog::ReuseOptions() const
{
  return ui->m_ReuseOptionsCheckBox->isChecked();
}

void QmitkFileReaderOptionsDialog::accept()
{
  const int index = m_ReaderItems.size() - ui->m_ReaderComboBox->currentIndex() - 1;
  m_ReaderItems[index].GetReader()->SetOptions(
        qobject_cast<QmitkFileReaderWriterOptionsWidget*>(ui->m_StackedOptionsWidget->currentWidget())->GetOptions());
  m_LoadInfo.m_ReaderSelector.Select(m_ReaderItems[index]);
  QDialog::accept();
}
