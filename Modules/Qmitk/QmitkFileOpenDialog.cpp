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
#include <mitkIFileReader.h>

// STL Headers
#include <list>

//microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

//QT
#include <mitkCommon.h>

// Test imports, delete later
#include <mitkAbstractFileReader.h>

const std::string QmitkFileOpenDialog::VIEW_ID = "org.mitk.views.QmitkFileOpenDialog";

QmitkFileOpenDialog::QmitkFileOpenDialog(QWidget* parent, Qt::WindowFlags f): QmitkFileDialog(parent, f)
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

    m_Options = GetSelectedOptions();
    // We are not looking for specific options here, which is okay, since the dialog currently only shows the
    // reader with the highest priority. Better behaviour required, if we want selectable readers.

    mitk::IFileReader* fileReader = m_FileReaderRegistry.GetReader(extension);
    fileReader->SetOptions(m_Options);
    m_FileReaders.push_back(fileReader);
  }
}

mitk::IFileReader::OptionList QmitkFileOpenDialog::QueryAvailableOptions(std::string path)
{
  std::string extension = path;
  extension.erase(0, extension.find_last_of('.'));

  us::ModuleContext* context = us::GetModuleContext();
  mitk::IFileReader* reader = m_FileReaderRegistry.GetReader(extension);

  if (reader == NULL)
  {
    // MITK_WARN << "Did not find ReaderService for registered Extension. This should be looked into by a developer.";
    return mitk::IFileReader::OptionList();
  }

  return reader->GetOptions();
}

std::vector<mitk::IFileReader*> QmitkFileOpenDialog::GetReaders()
{
  return this->m_FileReaders;
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
