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

#ifndef mitkSimulationReader_h
#define mitkSimulationReader_h

#include <mitkBaseDataSource.h>
#include <mitkFileReader.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MitkSimulation_EXPORT SimulationReader : public BaseDataSource, FileReader
  {
  public:
    static bool CanReadFile(const std::string& filename, const std::string& filePrefix, const std::string& filePattern);

    mitkClassMacro(SimulationReader, BaseDataSource);
    itkNewMacro(Self);

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);

    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);

    BaseDataSource::DataObjectPointer MakeOutput(DataObjectPointerArraySizeType);
    BaseDataSource::DataObjectPointer MakeOutput(const DataObjectIdentifierType& name);

  protected:
    void GenerateData();
    void GenerateOutputInformation();

  private:
    SimulationReader();
    ~SimulationReader();

    std::string m_FileName;
    std::string m_FilePattern;
    std::string m_FilePrefix;
  };
}

#endif
