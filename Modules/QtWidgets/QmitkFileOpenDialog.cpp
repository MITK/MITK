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

//#define _USE_MATH_DEFINES
#include <QmitkFileOpenDialog.h>

// MITK
#include <mitkFileReaderRegistry.h>

class QmitkFileOpenDialogPrivate
{
public:

  std::vector<mitk::IFileReader*> m_FileReaders;
  mitk::IFileReader::OptionList m_Options;
  mitk::FileReaderRegistry m_FileReaderRegistry;
};

QmitkFileOpenDialog::QmitkFileOpenDialog(QWidget* parent, Qt::WindowFlags f)
  : QmitkFileDialog(parent, f)
  , d(new QmitkFileOpenDialogPrivate)
{
  this->setFileMode(QFileDialog::ExistingFiles);
}

QmitkFileOpenDialog::~QmitkFileOpenDialog()
{
}

void QmitkFileOpenDialog::ProcessSelectedFile()
{
  QStringList files = this->selectedFiles();
  while( files.size() > 0)
  {
    std::string file = files.front().toStdString();
    files.pop_front();
    std::string extension = file;
    extension.erase(0, extension.find_last_of('.'));

    d->m_Options = GetSelectedOptions();
    // We are not looking for specific options here, which is okay, since the dialog currently only shows the
    // reader with the highest priority. Better behaviour required, if we want selectable readers.

    mitk::IFileReader* fileReader = d->m_FileReaderRegistry.GetReader(extension);
    fileReader->SetOptions(d->m_Options);
    d->m_FileReaders.push_back(fileReader);
  }
}

mitk::IFileReader::OptionList QmitkFileOpenDialog::QueryAvailableOptions(const QString& path)
{
  QString extension = QFileInfo(path).completeSuffix();

  mitk::IFileReader* reader = d->m_FileReaderRegistry.GetReader(extension.toStdString());

  if (reader == NULL)
  {
    // MITK_WARN << "Did not find ReaderService for registered Extension. This should be looked into by a developer.";
    return mitk::IFileReader::OptionList();
  }

  return reader->GetOptions();
}

std::vector<mitk::IFileReader*> QmitkFileOpenDialog::GetReaders()
{
  return d->m_FileReaders;
}

std::vector< mitk::BaseData::Pointer > QmitkFileOpenDialog::GetBaseDatas()
{
  //if (m_FileReaders.size() == 0 )
  //{
  //  MITK_WARN << "Tried go get BaseData while no FileReader was selected in Dialog. Returning empty list.";
  //  std::vector< mitk::BaseData::Pointer > emptyList;
  //  return  emptyList;
  //}

  //std::vector<mitk::BaseData::Pointer> result;
  //for(std::vector< mitk::IFileReader* >::iterator reader = m_FileReaders.begin(); reader != m_FileReaders.end; reader++)
  //{
  //  result.splice(result.end(), m_FileReaders.)
  //  result.pu
  //}
  //return m_FileReader->Read(this->selectedFiles().front().toStdString());
  std::vector< mitk::BaseData::Pointer > emptyList;
  return  emptyList;
}
