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
#include <mitkFileReaderManager.h>
#include <mitkIFileReader.h>

// STL Headers
#include <list>

//microservices
#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceProperties.h>

//QT
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <mitkCommon.h>

// Test imports, delete later
#include <mitkAbstractFileReader.h>

const std::string QmitkFileOpenDialog::VIEW_ID = "org.mitk.views.QmitkFileOpenDialog";

QmitkFileOpenDialog::QmitkFileOpenDialog(QWidget* parent, Qt::WindowFlags f): QmitkFileDialog(parent, f)
{
}

QmitkFileOpenDialog::~QmitkFileOpenDialog()
{
}

void QmitkFileOpenDialog::ProcessSelectedFile()
{
  std::string file = this->selectedFiles().front().toStdString();
  std::string extension = file;
  extension.erase(0, extension.find_last_of('.'));

  m_Options = GetSelectedOptions();
  // We are not looking for specific options here, which is okay, since the dialog currently only shows the
  // reader with the highest priority. Better behaviour required, if we want selectable readers.

  m_FileReader = m_FileReaderManager.GetReader(extension);
  m_FileReader->SetOptions(m_Options);
}

mitk::IFileReader* QmitkFileOpenDialog::GetReader()
{
  return this->m_FileReader;
}

std::list< mitk::BaseData::Pointer > QmitkFileOpenDialog::GetBaseData()
{
  if (m_FileReader == NULL )
  {
    MITK_WARN << "Tried go get BaseData while no FileReader was selected in Dialog. Returning empty list.";
    std::list< mitk::BaseData::Pointer > emptyList;
    return  emptyList;
  }
  return m_FileReader->Read(this->selectedFiles().front().toStdString());
}