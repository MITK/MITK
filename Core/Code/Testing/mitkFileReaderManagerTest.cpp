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
#include "mitkAbstractFileReader.h"
#include "mitkIFileReader.h"
#include "mitkFileReaderManager.h"
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include <mitkBaseData.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkLegacyFileReaderService.h>
#include <mitkImage.h>


#include <mitkPointSetReader.h>
#include <mitkPointSet.h>
#include <mitkCoreObjectFactory.h>

#include <itkProcessObject.h>
#include <itkLightObject.h>

#include <usServiceProperties.h>
#include <MitkExports.h>
#include <mitkCommon.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usServiceProperties.h>



class DummyReader : public mitk::AbstractFileReader, public itk::LightObject {

public:

  mitkClassMacro(DummyReader, itk::LightObject);
  itkNewMacro(Self);

  using mitk::AbstractFileReader::Read;

  virtual std::list< itk::SmartPointer<mitk::BaseData> >  Read(const std::istream& stream, mitk::DataStorage *ds = 0)
  {
    std::list<mitk::BaseData::Pointer> result;
    return result;
  }

  virtual void SetOptions(std::list< std::string > options )
  { m_Options = options; m_Registration.SetProperties(ConstructServiceProperties());}

  virtual void Init(std::string extension, int priority)
  {
   m_Extension = extension;
   m_Priority = priority;
   m_Description = "This is a dummy description.";
   this->RegisterMicroservice(us::GetModuleContext());
  }

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

  DummyReader* testDR = new DummyReader();
  DummyReader* otherDR = new DummyReader();
  DummyReader* mediocreTestDR = new DummyReader();
  DummyReader* prettyFlyTestDR = new DummyReader();
  DummyReader* awesomeTestDR = new DummyReader();

  testDR->Init("test",1);
  otherDR->Init("other",1);

  MITK_TEST_CONDITION_REQUIRED(testDR->CanRead("/this/is/a/folder/file.test"),"Positive test of default CanRead() implementation");
  MITK_TEST_CONDITION_REQUIRED(!testDR->CanRead("/this/is/a/folder/file.tes"),"Negative test of default CanRead() implementation");

  mitk::IFileReader* returned = mitk::FileReaderManager::GetReader("test");

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

  std::vector< mitk::IFileReader* > returnedList;
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

  // And now to verify a working read chain for a mps file:
  //mitk::PointSetReader::Pointer psr = mitk::PointSetReader::New();
  //std::list<mitk::BaseData::Pointer> basedata;
  //basedata = mitk::FileReaderManager::Read("F://Build//MITK-Data//pointSet.mps");
  //MITK_TEST_CONDITION_REQUIRED(basedata.size() > 0, "Testing correct read of PointSet");

  // Need to instanciate the CoreObjectFactory, so legacy Readers are available
  mitk::CoreObjectFactory::GetInstance();

  // Testing templated call to ReaderManager
  mitk::PointSet::Pointer pointset = mitk::FileReaderManager::Read< mitk::PointSet >("F://Build//MITK-Data//pointSet.mps");
  MITK_TEST_CONDITION_REQUIRED(pointset.IsNotNull(), "Testing templated call of Read()");

  // And now for something completely different... (Debug)
 // mitk::LegacyFileReaderService::Pointer lfr = mitk::LegacyFileReaderService::New(".nrrd", "Nearly Raw Raster Data");
  //returned = mitk::FileReaderManager::GetReader(".nrrd");
  //MITK_TEST_CONDITION_REQUIRED(lfr == returned, "Testing correct retrieval of specific FileReader with Options: Low priority reader with specific option");

  std::list<mitk::BaseData::Pointer> image = mitk::FileReaderManager::Read("F://Build//MITK-Data//Pic2DplusT.nrrd");
  MITK_TEST_CONDITION_REQUIRED(image.size() > 0, "Testing whether image was returned or not");
  mitk::Image::Pointer image2 = dynamic_cast<mitk::Image*> (image.front().GetPointer());
  MITK_TEST_CONDITION_REQUIRED(image2.IsNotNull(), "Testing if BaseData is an image");

  // always end with this!
  MITK_TEST_END();
}
