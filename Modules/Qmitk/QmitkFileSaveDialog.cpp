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
#include <QmitkFileSaveDialog.h>

// MITK
#include <mitkFileWriterRegistry.h>

// Test imports, delete later
#include <mitkAbstractFileReader.h>

class DummyReader : public mitk::AbstractFileReader
{
public:

  DummyReader(const DummyReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  DummyReader(const std::string& extension, int priority)
    : mitk::AbstractFileReader("application/dummy", extension, "This is a dummy description")
  {
    this->SetPriority(priority);
    //std::vector<std::string> options;
    OptionList options;
    options.push_back(std::make_pair("isANiceGuy", true));
    options.push_back(std::make_pair("canFly", false));
    options.push_back(std::make_pair("isAwesome", true));
    options.push_back(std::make_pair("hasOptions", true));
    options.push_back(std::make_pair("has more Options", true));
    options.push_back(std::make_pair("has maaaaaaaany Options", true));
    this->SetOptions(options);
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::vector< itk::SmartPointer<mitk::BaseData> >  Read(std::istream& /*stream*/)
  {
    std::vector<mitk::BaseData::Pointer> result;
    return result;
  }

private:

  DummyReader* Clone() const
  {
    return new DummyReader(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
}; // End of internal dummy reader

//
//
//
//

//

//

//
//
//

class QmitkFileSaveDialogPrivate
{
public:

  mitk::IFileWriter* m_FileWriter;
  mitk::IFileWriter::OptionList m_Options;
  mitk::FileWriterRegistry m_FileWriterRegistry;
  mitk::BaseData::Pointer m_BaseData;
};

QmitkFileSaveDialog::QmitkFileSaveDialog(mitk::BaseData::Pointer baseData, QWidget* parent, Qt::WindowFlags f)
  : QmitkFileDialog(parent, f)
  , d(new QmitkFileSaveDialogPrivate)
{
  d->m_BaseData = baseData;
  this->setFileMode(QFileDialog::AnyFile);
}

QmitkFileSaveDialog::~QmitkFileSaveDialog()
{
}

void QmitkFileSaveDialog::ProcessSelectedFile()
{
  std::string file = this->selectedFiles().front().toStdString();
  std::string extension = file;
  extension.erase(0, extension.find_last_of('.'));

  d->m_Options = GetSelectedOptions();
  // We are not looking for specific Options here, which is okay, since the dialog currently only shows the
  // reader with the highest priority. Better behaviour required, if we want selectable readers.

  d->m_FileWriter = d->m_FileWriterRegistry.GetWriter(extension);
  d->m_FileWriter->SetOptions(d->m_Options);
}

mitk::IFileWriter::OptionList QmitkFileSaveDialog::QueryAvailableOptions(const QString& /*path*/)
{
  mitk::IFileWriter* writer = d->m_FileWriterRegistry.GetWriter(d->m_BaseData->GetNameOfClass());

  if (writer == NULL)
  {
    // MITK_WARN << "Did not find WriterService for registered Extension. This should be looked into by a developer.";
    return mitk::IFileWriter::OptionList();
  }

  return writer->GetOptions();
}

mitk::IFileWriter* QmitkFileSaveDialog::GetWriter()
{
  return d->m_FileWriter;
}

void QmitkFileSaveDialog::WriteBaseData()
{
  if (d->m_FileWriter == NULL )
  {
    MITK_WARN << "Tried go write BaseData while no FileWriter was selected in Dialog. Aborting.";
    return;
  }
  d->m_FileWriter->Write(d->m_BaseData, this->selectedFiles().front().toStdString());
}
