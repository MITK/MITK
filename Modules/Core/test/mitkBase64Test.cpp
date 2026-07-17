/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBase64.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cstddef>
#include <span>
#include <string>
#include <vector>

/**
 * \brief Unit tests for mitk::Base64.
 *
 * The reference vectors are from RFC 4648. The round-trip case guards the
 * padding edge cases (input lengths that are 0, 1 and 2 modulo 3), which is
 * what preference byte arrays rely on for lossless storage.
 */
class mitkBase64TestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkBase64TestSuite);
  MITK_TEST(EncodesReferenceVectors);
  MITK_TEST(DecodesReferenceVectors);
  MITK_TEST(RoundTripsEveryLength);
  MITK_TEST(DecodeStopsAtNonAlphabetCharacter);
  CPPUNIT_TEST_SUITE_END();

  static std::string Encode(const std::string& text)
  {
    return mitk::Base64::Encode(std::span(reinterpret_cast<const std::byte*>(text.data()), text.size()));
  }

  static std::string Decode(const std::string& encoded)
  {
    const auto bytes = mitk::Base64::Decode(encoded);
    return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
  }

public:
  void setUp() override {}
  void tearDown() override {}

  void EncodesReferenceVectors()
  {
    CPPUNIT_ASSERT_EQUAL(std::string(""), Encode(""));
    CPPUNIT_ASSERT_EQUAL(std::string("Zg=="), Encode("f"));
    CPPUNIT_ASSERT_EQUAL(std::string("Zm8="), Encode("fo"));
    CPPUNIT_ASSERT_EQUAL(std::string("Zm9v"), Encode("foo"));
    CPPUNIT_ASSERT_EQUAL(std::string("Zm9vYg=="), Encode("foob"));
    CPPUNIT_ASSERT_EQUAL(std::string("Zm9vYmE="), Encode("fooba"));
    CPPUNIT_ASSERT_EQUAL(std::string("Zm9vYmFy"), Encode("foobar"));
  }

  void DecodesReferenceVectors()
  {
    CPPUNIT_ASSERT_EQUAL(std::string(""), Decode(""));
    CPPUNIT_ASSERT_EQUAL(std::string("f"), Decode("Zg=="));
    CPPUNIT_ASSERT_EQUAL(std::string("fo"), Decode("Zm8="));
    CPPUNIT_ASSERT_EQUAL(std::string("foo"), Decode("Zm9v"));
    CPPUNIT_ASSERT_EQUAL(std::string("foob"), Decode("Zm9vYg=="));
    CPPUNIT_ASSERT_EQUAL(std::string("fooba"), Decode("Zm9vYmE="));
    CPPUNIT_ASSERT_EQUAL(std::string("foobar"), Decode("Zm9vYmFy"));
  }

  void RoundTripsEveryLength()
  {
    for (std::size_t n = 0; n <= 300; ++n)
    {
      std::vector<std::byte> data(n);
      for (std::size_t i = 0; i < n; ++i)
        data[i] = static_cast<std::byte>((i * 37 + 11) & 0xff);

      const auto roundTripped = mitk::Base64::Decode(mitk::Base64::Encode(data));
      CPPUNIT_ASSERT_MESSAGE("round trip failed for length " + std::to_string(n), roundTripped == data);
    }
  }

  void DecodeStopsAtNonAlphabetCharacter()
  {
    // Decoding terminates at padding and ignores any trailing content.
    CPPUNIT_ASSERT_EQUAL(std::string("f"), Decode("Zg==ignored"));
    CPPUNIT_ASSERT_EQUAL(std::string("foobar"), Decode("Zm9vYmFy\n"));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkBase64)
