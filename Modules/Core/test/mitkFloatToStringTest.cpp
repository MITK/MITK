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

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <boost/lexical_cast.hpp>

#include <math.h>
#include "mitkEqual.h"

#include "mitkLog.h"


#include <limits>
#include <functional>

//!
//! Verifies boost::lexical<cast> for MITK's serialization purposes
//!
//! Verifies:
//! - special numbers behavior:
//!   - infinity converted to "inf", parsed from "inf" or "infinity" (case-independent)
//!   - not-a-number converted to "nan", parsed from "nan" (case-independent)
//! - Default round-trip conversion double-to-string-to-double compares equal given
//!   the MITK default epsilon value (mitk::Equal)
//!
class mitkFloatToStringTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFloatToStringTestSuite);
    MITK_TEST(ConfirmStringValues<float>);
    MITK_TEST(ConfirmStringValues<double>);
    MITK_TEST(TestConversions<float>);
    MITK_TEST(TestConversions<double>);
  CPPUNIT_TEST_SUITE_END();

public:

  template <typename DATATYPE>
  void ConfirmNumberToString(DATATYPE number, const std::string& s)

  {
      CPPUNIT_ASSERT_EQUAL( boost::lexical_cast<std::string>(number), s );
  }


  template <typename DATATYPE>
  void ConfirmStringToNumber(const std::string& s, DATATYPE number)

  {
      CPPUNIT_ASSERT_EQUAL( number,  boost::lexical_cast<DATATYPE>(s) );
  }

  template <typename DATATYPE>
  void ConfirmStringValues()
  {
    // we want to make sure that the following strings will be accepted and returned
    // by our conversion functions. This must not change in the future to ensure compatibility
    DATATYPE nan = boost::lexical_cast<DATATYPE>("nan");
    CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
    nan = boost::lexical_cast<DATATYPE>("NAN");
    CPPUNIT_ASSERT_MESSAGE("NAN==NAN must be false", !(nan==nan) );

    std::string s_nan = boost::lexical_cast<std::string>(nan);
    CPPUNIT_ASSERT_EQUAL(std::string("nan"), s_nan);

    ConfirmStringToNumber("inf", std::numeric_limits<DATATYPE>::infinity());
    ConfirmStringToNumber("INF", std::numeric_limits<DATATYPE>::infinity());
    ConfirmStringToNumber("infinity", std::numeric_limits<DATATYPE>::infinity());
    ConfirmStringToNumber("INFINITY", std::numeric_limits<DATATYPE>::infinity());

    ConfirmStringToNumber("-inf", -std::numeric_limits<DATATYPE>::infinity());
    ConfirmStringToNumber("-INF", -std::numeric_limits<DATATYPE>::infinity());
    ConfirmStringToNumber("-infinity", -std::numeric_limits<DATATYPE>::infinity());
    ConfirmStringToNumber("-INFINITY", -std::numeric_limits<DATATYPE>::infinity());

    ConfirmNumberToString(std::numeric_limits<DATATYPE>::infinity(), "inf");
    ConfirmNumberToString(-std::numeric_limits<DATATYPE>::infinity(), "-inf");
  }


  template <typename DATATYPE>
  void CheckRoundTrip(DATATYPE number,
                      DATATYPE tolerance)
  {
    std::string s = boost::lexical_cast<std::string>(number);
    DATATYPE number2 = boost::lexical_cast<DATATYPE>(s);

    CPPUNIT_ASSERT_MESSAGE(std::string("Must not parse string ") + s + " as NaN", number2 == number2);
    if (tolerance == 0)
    {
      CPPUNIT_ASSERT_EQUAL(number, number2);
    }
    else // mitk::Equal cannot take 0 as tolerance
    {
      CPPUNIT_ASSERT(mitk::Equal(number, number2, tolerance));
    }
  }

  template <typename DATATYPE>
  void CheckRoundTrip(const std::string& input)
  {
    DATATYPE number = boost::lexical_cast<DATATYPE>(input);
    std::string result = boost::lexical_cast<std::string>(number);

    // There are normal imprecisions when converting to string
    // We do only compare if the numeric values match "close enough"
    DATATYPE number2 = boost::lexical_cast<DATATYPE>(result);
    CPPUNIT_ASSERT(mitk::Equal(number, number2));
  }

  template <typename DATATYPE>
  void TestConversions()
  {
    // we cannot test the NaN roundtrip because nan == nan will never be true
    CheckRoundTrip<DATATYPE>(std::numeric_limits<DATATYPE>::infinity(), 0.0);
    CheckRoundTrip<DATATYPE>(-std::numeric_limits<DATATYPE>::infinity(), 0.0);

    CheckRoundTrip<DATATYPE>(std::numeric_limits<DATATYPE>::denorm_min(), mitk::eps);
    CheckRoundTrip<DATATYPE>(std::numeric_limits<DATATYPE>::epsilon(), mitk::eps);
    CheckRoundTrip<DATATYPE>(std::numeric_limits<DATATYPE>::lowest(), mitk::eps);
    CheckRoundTrip<DATATYPE>(std::numeric_limits<DATATYPE>::min(), mitk::eps);
    CheckRoundTrip<DATATYPE>(std::numeric_limits<DATATYPE>::max(), mitk::eps);
    CheckRoundTrip<DATATYPE>(sqrt(2), mitk::eps);
    CheckRoundTrip<DATATYPE>(0.000000042, mitk::eps);
    CheckRoundTrip<DATATYPE>(422345678.2345678, mitk::eps);
    CheckRoundTrip<DATATYPE>(0.0, 0);
    CheckRoundTrip<DATATYPE>(-0.0, 0);

    CheckRoundTrip<DATATYPE>("1");
    CheckRoundTrip<DATATYPE>("1.1");
    CheckRoundTrip<DATATYPE>("1.12121212");
    CheckRoundTrip<DATATYPE>("1.1e-2");

  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkFloatToString)
