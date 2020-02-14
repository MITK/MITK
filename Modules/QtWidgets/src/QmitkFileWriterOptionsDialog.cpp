/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFileWriterOptionsDialog.h"
#include "ui_QmitkFileWriterOptionsDialog.h"

#include "QmitkFileReaderWriterOptionsWidget.h"
#include "mitkFileWriterSelector.h"
#include "mitkIFileWriter.h"

QmitkFileWriterOptionsDialog::QmitkFileWriterOptionsDialog(mitk::IOUtil::SaveInfo &saveInfo, QWidget *parent)
  : QDialog(parent), ui(new Ui::QmitkFileWriterOptionsDialog), m_SaveInfo(saveInfo)
{
  ui->setupUi(this);

  m_WriterItems = saveInfo.m_WriterSelector.Get(saveInfo.m_MimeType.GetName());

  bool hasOptions = false;
  int selectedIndex = 0;
  long selectedWriterId = saveInfo.m_WriterSelector.GetSelectedId();
  int i = 0;
  // Fill the combo box such that at the first entry is the best matching writer
  for (std::vector<mitk::FileWriterSelector::Item>::const_reverse_iterator iter = m_WriterItems.rbegin(),
                                                                           iterEnd = m_WriterItems.rend();
       iter != iterEnd;
       ++iter, ++i)
  {
    ui->m_WriterComboBox->addItem(QString::fromStdString(iter->GetDescription()));
    mitk::IFileWriter::Options options = iter->GetWriter()->GetOptions();
    if (!options.empty())
    {
      hasOptions = true;
    }
    ui->m_StackedOptionsWidget->addWidget(new QmitkFileReaderWriterOptionsWidget(options));
    if (iter->GetServiceId() == selectedWriterId)
    {
      selectedIndex = i;
    }
  }
  ui->m_WriterComboBox->setCurrentIndex(selectedIndex);

  if (!hasOptions)
  {
    ui->m_OptionsBox->setVisible(false);
  }

  if (m_WriterItems.size() < 2)
  {
    ui->m_WriterLabel->setVisible(false);
    ui->m_WriterComboBox->setVisible(false);
    ui->m_FilePathLabel->setText(QString("File: %1").arg(QString::fromStdString(saveInfo.m_Path)));
  }
  else
  {
    ui->m_FilePathLabel->setText(QString("for %1").arg(QString::fromStdString(saveInfo.m_Path)));
  }

  this->setWindowTitle("File writing options");
}

QmitkFileWriterOptionsDialog::~QmitkFileWriterOptionsDialog()
{
  delete ui;
}

bool QmitkFileWriterOptionsDialog::ReuseOptions() const
{
  return ui->m_ReuseOptionsCheckBox->isChecked();
}

void QmitkFileWriterOptionsDialog::accept()
{
  const int index = m_WriterItems.size() - ui->m_WriterComboBox->currentIndex() - 1;
  m_WriterItems[index].GetWriter()->SetOptions(
    qobject_cast<QmitkFileReaderWriterOptionsWidget *>(ui->m_StackedOptionsWidget->currentWidget())->GetOptions());
  m_SaveInfo.m_WriterSelector.Select(m_WriterItems[index]);
  QDialog::accept();
}
