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

#include <mitkExceptionMacro.h>
#include <mitkStringProperty.h>
#include "mitkSimulation.h"
#include "mitkSimulationWriter.h"

mitk::SimulationWriter::SimulationWriter()
  : m_FileExtension(".scn"),
    m_MIMEType("application/xml")
{
  this->SetNumberOfRequiredInputs(1);
  this->SetNumberOfIndexedOutputs(0);
}

mitk::SimulationWriter::~SimulationWriter()
{
}

void mitk::SimulationWriter::GenerateData()
{
  if (m_FileName.empty())
    mitkThrow() << "Filename is empty!";

  Simulation* simulation = dynamic_cast<Simulation*>(this->GetInput(0));

  if (simulation == NULL)
    mitkThrow() << "Invalid input!";

  std::string originalPath = "<!-- ORIGINAL_PATH = ";
  originalPath += simulation->GetProperty("Path")->GetValueAsString();
  originalPath += "\n     This comment is parsed by MITK. -->\n";

  std::ofstream scnFile(m_FileName.c_str());
  scnFile << originalPath << simulation->GetProperty("Scene File")->GetValueAsString();
}

mitk::Simulation* mitk::SimulationWriter::GetInput()
{
  if (this->GetNumberOfInputs() == 0)
    return NULL;

  return dynamic_cast<Simulation*>(this->GetInput(0));
}

void mitk::SimulationWriter::SetInput(Simulation *simulation)
{
  this->SetNthInput(0, simulation);
}

const char* mitk::SimulationWriter::GetFileName() const
{
  return m_FileName.c_str();
}

void mitk::SimulationWriter::SetFileName(const char* fileName)
{
  m_FileName = fileName;
}

const char* mitk::SimulationWriter::GetFilePrefix() const
{
  return m_FilePrefix.c_str();
}

void mitk::SimulationWriter::SetFilePrefix(const char* filePrefix)
{
  m_FilePrefix = filePrefix;
}

const char* mitk::SimulationWriter::GetFilePattern() const
{
  return m_FilePattern.c_str();
}

void mitk::SimulationWriter::SetFilePattern(const char* filePattern)
{
  m_FilePattern = filePattern;
}

std::string mitk::SimulationWriter::GetFileExtension()
{
  return m_FileExtension;
}

std::vector<std::string> mitk::SimulationWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(m_FileExtension);
  return possibleFileExtensions;
}

std::string mitk::SimulationWriter::GetWritenMIMEType()
{
  return m_MIMEType;
}

bool mitk::SimulationWriter::CanWriteDataType(DataNode* dataNode)
{
  if (dataNode == NULL)
    return false;

  return this->CanWriteBaseDataType(dataNode->GetData());
}

const char* mitk::SimulationWriter::GetDefaultFilename()
{
  std::string defaultFilename = "Simulation" + m_FileExtension;
  return defaultFilename.c_str();
}

const char* mitk::SimulationWriter::GetFileDialogPattern()
{
  std::string fileDialogPattern = "SOFA Scene Files (*" + m_FileExtension + ")";
  return fileDialogPattern.c_str();
}

const char* mitk::SimulationWriter::GetDefaultExtension()
{
  return m_FileExtension.c_str();
}

bool mitk::SimulationWriter::CanWriteBaseDataType(BaseData::Pointer data)
{
  if (data.IsNull())
    return false;

  return dynamic_cast<mitk::Simulation*>(data.GetPointer()) != NULL;
}

void mitk::SimulationWriter::DoWrite(BaseData::Pointer data)
{
  if (this->CanWriteBaseDataType(data))
  {
    this->SetNthInput(0, dynamic_cast<itk::DataObject*>(data.GetPointer()));
    this->Update();
  }
}


std::string mitk::SimulationWriter::GetSupportedBaseData() const
{
  return Simulation::GetStaticNameOfClass();
}
