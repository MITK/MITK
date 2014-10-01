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

#ifndef mitkSimulationWriter_h
#define mitkSimulationWriter_h

#include <mitkFileWriterWithInformation.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class Simulation;

  class MitkSimulation_EXPORT SimulationWriter : public FileWriterWithInformation
  {
  public:
    mitkClassMacro(SimulationWriter, FileWriterWithInformation);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    using Superclass::SetInput;
    void SetInput(Simulation* simulation);

    // mitk::FileWriter interface

    mitkWriterMacro;
    using Superclass::GetInput;
    Simulation* GetInput();

    const char* GetFileName() const;
    void SetFileName(const char* fileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* filePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* filePattern);
    std::string GetFileExtension();
    std::vector<std::string> GetPossibleFileExtensions();
    std::string GetWritenMIMEType();
    bool CanWriteDataType(DataNode* dataNode);

    // mitk::FileWriterWithInformation interface

    const char* GetDefaultFilename();
    const char* GetFileDialogPattern();
    const char* GetDefaultExtension();
    bool CanWriteBaseDataType(BaseData::Pointer data);
    void DoWrite(BaseData::Pointer data);

    std::string GetSupportedBaseData() const;

  protected:
    SimulationWriter();
    ~SimulationWriter();

    void GenerateData();

  private:
    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;
    std::string m_FileExtension;
    std::string m_MIMEType;
  };
}

#endif
