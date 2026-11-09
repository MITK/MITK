/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAbstractFileReader.h>
#include <mitkCustomMimeType.h>
#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <string>
#include <vector>

namespace
{
  class ChoiceOptionReader : public mitk::AbstractFileReader
  {
  public:
    ChoiceOptionReader() : AbstractFileReader(mitk::CustomMimeType("ChoiceOptionTestMimeType"), "Choice option test reader")
    {
      Options options;
      options["Mode"] = std::vector<std::string>{ "first", "second" };
      options["Flag"] = true;
      this->SetDefaultOptions(options);
    }

    using AbstractFileReader::GetChoiceOption;

    std::vector<itk::SmartPointer<mitk::BaseData>> DoRead() override { return {}; }

  private:
    ChoiceOptionReader* Clone() const override { return new ChoiceOptionReader(*this); }
  };

  std::string RefusalMessage(const ChoiceOptionReader& reader, const std::string& name)
  {
    try
    {
      reader.GetChoiceOption(name);
    }
    catch (const mitk::Exception& e)
    {
      return e.GetDescription();
    }
    CPPUNIT_FAIL("The option \"" + name + "\" was resolved instead of refused");
    return {};
  }
}

class mitkFileReaderWriterChoiceOptionTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFileReaderWriterChoiceOptionTestSuite);
  MITK_TEST(UnsetChoiceIsTheFirst);
  MITK_TEST(ChosenValueIsReturned);
  MITK_TEST(UnknownValueIsRefused);
  MITK_TEST(OptionsHandedBackKeepTheDefault);
  MITK_TEST(ForeignListIsRefused);
  MITK_TEST(OptionWithoutChoicesIsRefused);
  CPPUNIT_TEST_SUITE_END();

public:
  void UnsetChoiceIsTheFirst()
  {
    ChoiceOptionReader reader;
    CPPUNIT_ASSERT_EQUAL(std::string("first"), reader.GetChoiceOption("Mode"));
  }

  void ChosenValueIsReturned()
  {
    ChoiceOptionReader reader;
    reader.SetOption("Mode", std::string("second"));
    CPPUNIT_ASSERT_EQUAL(std::string("second"), reader.GetChoiceOption("Mode"));
  }

  void UnknownValueIsRefused()
  {
    ChoiceOptionReader reader;
    reader.SetOption("Mode", std::string("Second"));
    const auto message = RefusalMessage(reader, "Mode");
    CPPUNIT_ASSERT_MESSAGE("The refusal names the value and the choices: " + message,
                           message.find("\"Second\"") != std::string::npos && message.find("first, second") != std::string::npos);
  }

  // IOUtil hands the options of one reader to the reader of the next object of a multi-object load.
  void OptionsHandedBackKeepTheDefault()
  {
    ChoiceOptionReader reader;
    reader.SetOptions(reader.GetOptions());
    CPPUNIT_ASSERT_EQUAL(std::string("first"), reader.GetChoiceOption("Mode"));

    ChoiceOptionReader next;
    next.SetOptions(reader.GetOptions());
    CPPUNIT_ASSERT_EQUAL(std::string("first"), next.GetChoiceOption("Mode"));
  }

  void ForeignListIsRefused()
  {
    ChoiceOptionReader reader;
    reader.SetOption("Mode", std::vector<std::string>{ "second", "first" });
    RefusalMessage(reader, "Mode");
  }

  void OptionWithoutChoicesIsRefused()
  {
    ChoiceOptionReader reader;
    RefusalMessage(reader, "Flag");
    RefusalMessage(reader, "Unregistered");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkFileReaderWriterChoiceOption)
