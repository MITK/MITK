/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkFileChooser.h"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

QmitkFileChooser::QmitkFileChooser(QWidget *parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_SelectDir(false),
    m_FileMustExist(true),
    m_SelectFile(new QPushButton("Select File")),
    m_File(new QLineEdit)
{
  m_File->setReadOnly(true);
  this->SetHorizotalLayout(false);

  connect(m_SelectFile, SIGNAL(clicked(bool)), this, SLOT(OnSelectFileClicked(bool)));
  connect(m_File, SIGNAL(editingFinished()), this, SLOT(OnFileEditingFinished()));
}

void QmitkFileChooser::SetHorizotalLayout(bool horizontalLayout)
{
  QBoxLayout *layout = nullptr;
  if (horizontalLayout)
    layout = new QHBoxLayout;
  else
    layout = new QVBoxLayout;

  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_File);
  layout->addWidget(m_SelectFile);

  this->setLayout(layout);
}

void QmitkFileChooser::SetSelectDir(bool selectDir)
{
  m_SelectDir = selectDir;
}

void QmitkFileChooser::SetFileMustExist(bool fileMustExist)
{
  m_FileMustExist = fileMustExist;
}

void QmitkFileChooser::SetReadOnly(bool ReadOnly)
{
  m_File->setReadOnly(ReadOnly);
}

void QmitkFileChooser::SetFile(const std::string &file)
{
  QFileInfo info(QString::fromStdString(file));
  if (info.exists() || m_FileMustExist == false)
  {
    m_File->setText(QString::fromStdString(file));
    emit NewFileSelected(file);
  }
}

void QmitkFileChooser::SetFilePattern(const std::string &filepattern)
{
  m_FilePattern = QString::fromStdString(filepattern);
}

bool QmitkFileChooser::IsValidFile() const
{
  QFileInfo info(m_File->text());
  return info.exists();
}

std::string QmitkFileChooser::GetFile() const
{
  return m_File->text().toStdString();
}

void QmitkFileChooser::OnSelectFileClicked(bool)
{
  QString filename;
  if (m_SelectDir)
    filename = QFileDialog::getExistingDirectory(QApplication::activeWindow(), "Open directory", m_File->text());
  else
  {
    if (m_FileMustExist)
      filename = QFileDialog::getOpenFileName(QApplication::activeWindow(), "Open file", m_File->text(), m_FilePattern);
    else
      filename = QFileDialog::getSaveFileName(QApplication::activeWindow(), "Open file", m_File->text(), m_FilePattern);
  }

  if (!filename.isEmpty())
    m_File->setText(filename);

  emit NewFileSelected(filename.toStdString());
}

void QmitkFileChooser::OnFileEditingFinished()
{
  emit NewFileSelected(m_File->text().toStdString());
}
