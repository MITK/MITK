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

#include "mitkTestingMacros.h"
#include "mitkFileReaderInterface.h"
#include "mitkFileReaderManager.h"
#include "mitkGetModuleContext.h"
#include "mitkModuleContext.h"
#include <usServiceProperties.h>
#include <MitkExports.h>
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>


class DummyReader : public mitk::FileReaderInterface {

public:
  mitkClassMacro(DummyReader, mitk::FileReaderInterface);
  itkNewMacro(Self);

  mitk::ServiceRegistration m_ServiceRegistration;

  virtual mitk::BaseData::Pointer DummyReader::Read(std::string path = 0)
  { return 0; }

  virtual mitk::BaseData::Pointer DummyReader::Read(std::istream*)
  { return 0; }

  virtual int DummyReader::GetPriority()
  { return 0; }

  virtual std::list< std::string > DummyReader::GetOptions()
  { std::list< std::string > result;
    return result; }

  virtual void DummyReader::SetOptions(std::list< std::string > Options )
  { }

  virtual bool DummyReader::CanRead(const std::string& path)
  { return true; }

  virtual float DummyReader::GetProgress()
  { return 1; }

  virtual const char* DummyReader::GetFileName() const
  { return ""; }

  virtual void DummyReader::SetFileName(const char* aFileName)
  {}

  virtual const char* DummyReader::GetFilePrefix() const
  { return ""; }

  virtual void DummyReader::SetFilePrefix(const char* aFilePrefix)
  { }

  virtual const char* DummyReader::GetFilePattern() const
  { return ""; }

  virtual void DummyReader::SetFilePattern(const char* aFilePattern)
  { }

  virtual void DummyReader::Init(std::string extension)
  {
    mitk::ModuleContext* context = mitk::GetModuleContext();
    mitk::ServiceProperties props;
    props[mitk::FileReaderInterface::US_EXTENSION] =extension;


    m_ServiceRegistration = context->RegisterService<mitk::FileReaderInterface>(this, props);
  }

};

/**
 *  TODO
 */
int mitkFileReaderManagerTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileReaderManager");
 // mitk::FileReaderManager::Pointer frm = mitk::FileReaderManager::New();
 // MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing FileReaderManager instantiation");

  DummyReader* testDR = new DummyReader();
  DummyReader* otherDR = new DummyReader();
  testDR->Init("test");
  otherDR->Init("other");

  mitk::FileReaderInterface* returned = mitk::FileReaderManager::GetReader("test");

  MITK_TEST_CONDITION_REQUIRED(testDR == returned,"Testing correct retrieval of FileReader 1/2");

  returned = mitk::FileReaderManager::GetReader("other");

  MITK_TEST_CONDITION_REQUIRED(otherDR == returned,"Testing correct retrieval of FileReader 2/2");
  // always end with this!
  MITK_TEST_END();
}
