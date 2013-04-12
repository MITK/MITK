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

#include <itkProcessObject.h>

#include <usServiceProperties.h>
#include <MitkExports.h>
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>
#include <usServiceRegistration.h>


class DummyReader : public mitk::FileReaderInterface, public itk::ProcessObject{

public:
  mitkClassMacro(DummyReader, itk::ProcessObject);
  itkNewMacro(Self);

  mitk::ServiceRegistration m_ServiceRegistration;
  std::list< std::string > m_Options; // this list can be set and will be returned via getOptions and via getSupportedOptions (it's a dummy!)

  virtual mitk::BaseData::Pointer DummyReader::Read(std::string path = 0)
  { return 0; }

  virtual mitk::BaseData::Pointer DummyReader::Read(std::istream*)
  { return 0; }

  virtual int DummyReader::GetPriority()
  { return 0; }

  virtual std::list< std::string > GetSupportedOptions()
  { return m_Options; }

  virtual std::list< std::string > DummyReader::GetOptions()
  { return m_Options; }

  virtual void DummyReader::SetOptions(std::list< std::string > options )
  { m_Options = options; }

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

  virtual void DummyReader::Init(std::string extension, int priority)
  {
    mitk::ModuleContext* context = mitk::GetModuleContext();
    mitk::ServiceProperties props;
    props[mitk::FileReaderInterface::US_EXTENSION] = extension;
    props[mitk::ServiceConstants::SERVICE_RANKING()]  = priority;

    m_ServiceRegistration = context->RegisterService<mitk::FileReaderInterface>(this, props);
  }

  bool DummyReader::CanReadFromMemory(  )
  { return false; }

  void DummyReader::SetReadFromMemory( bool read )
  {  }

  bool DummyReader::GetReadFromMemory(  )
  { return false; }

  void DummyReader::SetMemoryBuffer(const char* dataArray, unsigned int size)
  {   }


}; // End of internal dummy reader

/**
 *  TODO
 */
int mitkFileReaderManagerTest(int argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("FileReaderManager");
 // mitk::FileReaderManager::Pointer frm = mitk::FileReaderManager::New();
 // MITK_TEST_CONDITION_REQUIRED(argc == 2,"Testing FileReaderManager instantiation");

MITK_INFO << mitk::ServiceConstants::SERVICE_RANKING();

  DummyReader* testDR = new DummyReader();
  DummyReader* otherDR = new DummyReader();
  DummyReader* mediocreTestDR = new DummyReader();
  DummyReader* prettyFlyTestDR = new DummyReader();
  DummyReader* awesomeTestDR = new DummyReader();

  testDR->Init("test",1);
  otherDR->Init("other",1);

  mitk::FileReaderInterface* returned = mitk::FileReaderManager::GetReader("test");

  MITK_TEST_CONDITION_REQUIRED(testDR == returned,"Testing correct retrieval of FileReader 1/2");

  returned = mitk::FileReaderManager::GetReader("other");

  MITK_TEST_CONDITION_REQUIRED(otherDR == returned,"Testing correct retrieval of FileReader 2/2");

  mediocreTestDR->Init("test",20);
  prettyFlyTestDR->Init("test",50);
  awesomeTestDR->Init("test",100);

  returned = mitk::FileReaderManager::GetReader("test");
  MITK_TEST_CONDITION_REQUIRED(awesomeTestDR == returned, "Testing correct priorized retrieval of FileReader: Best reader");

  // Now to give those readers some options, then we will try again

  std::list<std::string> options;
  options.push_front("isANiceGuy");
  mediocreTestDR->SetOptions(options);
  options.clear();
  options.push_front("canFly");
  prettyFlyTestDR->SetOptions(options);
  options.push_front("isAwesome");
  awesomeTestDR->SetOptions(options); //note: awesomeReader canFly and isAwesome

  // Reset Options, use to define what we want the reader to do
  options.clear();
  options.push_front("canFly");
  returned = mitk::FileReaderManager::GetReader("test", options);
  MITK_TEST_CONDITION_REQUIRED(awesomeTestDR == returned, "Testing correct retrieval of FileReader with Options: Best reader with options");

  options.push_front("isAwesome");
  returned = mitk::FileReaderManager::GetReader("test", options);
  MITK_TEST_CONDITION_REQUIRED(awesomeTestDR == returned, "Testing correct retrieval of FileReader with multiple Options: Best reader with options");

  options.clear();
  options.push_front("isANiceGuy");
  returned = mitk::FileReaderManager::GetReader("test", options);
  MITK_TEST_CONDITION_REQUIRED(mediocreTestDR == returned, "Testing correct retrieval of specific FileReader with Options: Low priority reader with specific option");

  options.push_front("canFly");
  returned = mitk::FileReaderManager::GetReader("test", options);
  MITK_TEST_CONDITION_REQUIRED(returned == 0, "Testing correct return of 0 value when no matching reader was found");

  // Onward to test the retrieval of multiple readers

  std::list< mitk::FileReaderInterface* > returnedList;
  returnedList = mitk::FileReaderManager::GetReaders("test", options);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 0, "Testing correct return of zero readers when no matching reader was found, asking for all compatibles");

  options.clear();
  options.push_back("canFly");
  returnedList = mitk::FileReaderManager::GetReaders("test", options);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 2, "Testing correct return of two readers when two matching reader was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(returnedList.front() == awesomeTestDR, "Testing correct priorization of returned Readers with options 1/2");
  MITK_TEST_CONDITION_REQUIRED(returnedList.back() == prettyFlyTestDR, "Testing correct priorization of returned Readers with options 2/2");

  options.clear();
  options.push_back("isAwesome");
  returnedList = mitk::FileReaderManager::GetReaders("test", options);
  MITK_TEST_CONDITION_REQUIRED(returnedList.size() == 1, "Testing correct return of one readers when one matching reader was found, asking for all compatibles");
  MITK_TEST_CONDITION_REQUIRED(returnedList.front() == awesomeTestDR, "Testing correctness of result from former query");

  // always end with this!
  MITK_TEST_END();
}
