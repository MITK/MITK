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

class DummyReader : public mitk::AbstractFileReader
{
public:

  DummyReader(const DummyReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  DummyReader(const std::string& extension, int priority)
    : mitk::AbstractFileReader(extension, "This is a dummy description")
  {
    m_Priority = priority;
    //std::list<std::string> options;
    m_Options.push_front(std::make_pair("isANiceGuy", true));
    m_Options.push_front(std::make_pair("canFly", false));
    m_Options.push_front(std::make_pair("isAwesome", true));
    m_Options.push_front(std::make_pair("hasOptions", true));
    m_Options.push_front(std::make_pair("has more Options", true));
    m_Options.push_front(std::make_pair("has maaaaaaaany Options", true));
    m_ServiceReg = this->RegisterService();
  }

  ~DummyReader()
  {
    if (m_ServiceReg) m_ServiceReg.Unregister();
  }

  using mitk::AbstractFileReader::Read;

  virtual std::list< itk::SmartPointer<mitk::BaseData> >  Read(const std::istream& /*stream*/, mitk::DataStorage* /*ds*/ = 0)
  {
    std::list<mitk::BaseData::Pointer> result;
    return result;
  }

  virtual void SetOptions(const std::list< mitk::IFileWriter::FileServiceOption >& options )
  {
    m_Options = options;
    //m_Registration.SetProperties(GetServiceProperties());
  }

private:

  DummyReader* Clone() const
  {
    return new DummyReader(*this);
  }

  us::ServiceRegistration<mitk::IFileReader> m_ServiceReg;
}; // End of internal dummy reader

const std::string QmitkFileSaveDialog::VIEW_ID = "org.mitk.views.QmitkFileSaveDialog";

//
//
//
//

//

//

//
//
//

QmitkFileSaveDialog::QmitkFileSaveDialog(mitk::BaseData::Pointer baseData, QWidget* parent, Qt::WindowFlags f): QmitkFileDialog(parent, f)
{
  m_BaseData = baseData;
}

QmitkFileSaveDialog::~QmitkFileSaveDialog()
{
}

void QmitkFileSaveDialog::ProcessSelectedFile()
{
  std::string file = this->selectedFiles().front().toStdString();
  std::string extension = file;
  extension.erase(0, extension.find_last_of('.'));

  m_Options = GetSelectedOptions();
  // We are not looking for specific Options here, which is okay, since the dialog currently only shows the
  // reader with the highest priority. Better behaviour required, if we want selectable readers.

  m_FileWriter = m_FileWriterManager.GetWriter(extension);
  m_FileWriter->SetOptions(m_Options);
}

std::list<mitk::IFileWriter::FileServiceOption> QmitkFileSaveDialog::QueryAvailableOptions(std::string path)
{
  us::ModuleContext* context = us::GetModuleContext();
  mitk::IFileWriter* writer = m_FileWriterManager.GetWriter(m_BaseData->GetNameOfClass());

  if (writer == NULL)
  {
    // MITK_WARN << "Did not find ReaderService for registered Extension. This should be looked into by a developer.";
    std::list<mitk::IFileReader::FileServiceOption> emptyList;
    return emptyList;
  }

  return writer->GetOptions();
}

mitk::IFileWriter* QmitkFileSaveDialog::GetWriter()
{
  return this->m_FileWriter;
}

void QmitkFileSaveDialog::WriteBaseData(std::list< mitk::BaseData::Pointer > data)
{
}

//std::list< mitk::BaseData::Pointer > QmitkFileSaveDialog::GetBaseData()
//{
//  if (m_FileReader == NULL )
//  {
//    MITK_WARN << "Tried go get BaseData while no FileReader was selected in Dialog. Returning empty list.";
//    std::list< mitk::BaseData::Pointer > emptyList;
//    return  emptyList;
//  }
//  return m_FileReader->Read(this->selectedFiles().front().toStdString());
//}