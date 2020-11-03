/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFileReaderOptionsDialog.h"
#include "ui_QmitkFileReaderOptionsDialog.h"

#include "QmitkFileReaderWriterOptionsWidget.h"
#include "mitkIFileReader.h"

QmitkFileReaderOptionsDialog::QmitkFileReaderOptionsDialog(mitk::IOUtil::LoadInfo &loadInfo, QWidget *parent)
  : QDialog(parent, Qt::WindowStaysOnTopHint), ui(new Ui::QmitkFileReaderOptionsDialog), m_LoadInfo(loadInfo)

{
  ui->setupUi(this);

  m_ReaderItems = loadInfo.m_ReaderSelector.Get();

  int selectedIndex = 0;
  long selectedReaderId = loadInfo.m_ReaderSelector.GetSelectedId();
  int i = 0;
  for (std::vector<mitk::FileReaderSelector::Item>::const_reverse_iterator iter = m_ReaderItems.rbegin(),
                                                                           iterEnd = m_ReaderItems.rend();
       iter != iterEnd;
       ++iter)
  {
    ui->m_ReaderComboBox->addItem(QString::fromStdString(iter->GetDescription()));
    mitk::IFileReader::Options options = iter->GetReader()->GetOptions();
    if (!options.empty())
    {
    }
    ui->m_StackedOptionsWidget->addWidget(new QmitkFileReaderWriterOptionsWidget(options));
    if (iter->GetServiceId() == selectedReaderId)
    {
      selectedIndex = i;
    }
  }

  connect(ui->m_ReaderComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SetCurrentReader(int)));
  ui->m_ReaderComboBox->setCurrentIndex(selectedIndex);

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

  ui->m_OptionsBox->setVisible(!qobject_cast<QmitkFileReaderWriterOptionsWidget*>(ui->m_StackedOptionsWidget->currentWidget())->GetOptions().empty());

  this->setWindowTitle("File reading options");
}

QmitkFileReaderOptionsDialog::~QmitkFileReaderOptionsDialog()
{
  delete ui;
}

void QmitkFileReaderOptionsDialog::SetCurrentReader(int index)
{
  ui->m_StackedOptionsWidget->setCurrentIndex(index);
  ui->m_OptionsBox->setVisible(!qobject_cast<QmitkFileReaderWriterOptionsWidget*>(ui->m_StackedOptionsWidget->currentWidget())->GetOptions().empty());
}

bool QmitkFileReaderOptionsDialog::ReuseOptions() const
{
  return ui->m_ReuseOptionsCheckBox->isChecked();
}

void QmitkFileReaderOptionsDialog::accept()
{
  const int index = m_ReaderItems.size() - ui->m_ReaderComboBox->currentIndex() - 1;
  m_ReaderItems[index].GetReader()->SetOptions(
    qobject_cast<QmitkFileReaderWriterOptionsWidget *>(ui->m_StackedOptionsWidget->currentWidget())->GetOptions());
  m_LoadInfo.m_ReaderSelector.Select(m_ReaderItems[index]);
  QDialog::accept();
}
