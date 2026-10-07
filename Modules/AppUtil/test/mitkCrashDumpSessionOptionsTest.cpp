/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCrashDumpSessionOptions.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkCrashDumpSessionOptionsTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCrashDumpSessionOptionsTestSuite);
  MITK_TEST(SettingsApplyWithoutOverrides);
  MITK_TEST(EnvironmentDisablesDespiteSettings);
  MITK_TEST(FlagDisablesAndNamesItself);
  MITK_TEST(WatchdogFlagBeatsEnvironmentBeatsSettings);
  MITK_TEST(WatchdogOverrideOfZeroTurnsItOff);
  MITK_TEST(WatchdogOverrideIsNotClamped);
  CPPUNIT_TEST_SUITE_END();

  using Source = mitk::CrashDumpOptionSource;

public:
  static mitk::CrashDumpSettings Settings(bool enabled, int watchdogSeconds)
  {
    mitk::CrashDumpSettings settings;
    settings.Enabled = enabled;
    settings.WatchdogTimeoutSeconds = watchdogSeconds;
    return settings;
  }

  void SettingsApplyWithoutOverrides()
  {
    auto options = mitk::ResolveCrashDumpSessionOptions({}, Settings(false, 30));

    CPPUNIT_ASSERT(!options.Arm);
    CPPUNIT_ASSERT(Source::Settings == options.ArmSource);
    CPPUNIT_ASSERT_EQUAL(30, options.WatchdogTimeoutSeconds);
    CPPUNIT_ASSERT(Source::Settings == options.WatchdogSource);

    options = mitk::ResolveCrashDumpSessionOptions({}, Settings(true, 0));
    CPPUNIT_ASSERT(options.Arm);
    CPPUNIT_ASSERT_EQUAL(0, options.WatchdogTimeoutSeconds);
  }

  void EnvironmentDisablesDespiteSettings()
  {
    mitk::CrashDumpOptionOverrides overrides;
    overrides.NoCrashDumpsEnvironment = true;

    const auto options = mitk::ResolveCrashDumpSessionOptions(overrides, Settings(true, 0));

    CPPUNIT_ASSERT(!options.Arm);
    CPPUNIT_ASSERT(Source::EnvironmentVariable == options.ArmSource);
  }

  void FlagDisablesAndNamesItself()
  {
    mitk::CrashDumpOptionOverrides overrides;
    overrides.NoCrashDumpsFlag = true;
    overrides.NoCrashDumpsEnvironment = true;

    const auto options = mitk::ResolveCrashDumpSessionOptions(overrides, Settings(true, 0));

    CPPUNIT_ASSERT(!options.Arm);
    CPPUNIT_ASSERT(Source::CommandLine == options.ArmSource);
  }

  void WatchdogFlagBeatsEnvironmentBeatsSettings()
  {
    mitk::CrashDumpOptionOverrides overrides;
    overrides.WatchdogEnvironmentSeconds = 20;

    auto options = mitk::ResolveCrashDumpSessionOptions(overrides, Settings(true, 60));
    CPPUNIT_ASSERT_EQUAL(20, options.WatchdogTimeoutSeconds);
    CPPUNIT_ASSERT(Source::EnvironmentVariable == options.WatchdogSource);

    overrides.WatchdogFlagSeconds = 15;
    options = mitk::ResolveCrashDumpSessionOptions(overrides, Settings(true, 60));
    CPPUNIT_ASSERT_EQUAL(15, options.WatchdogTimeoutSeconds);
    CPPUNIT_ASSERT(Source::CommandLine == options.WatchdogSource);
  }

  void WatchdogOverrideOfZeroTurnsItOff()
  {
    mitk::CrashDumpOptionOverrides overrides;
    overrides.WatchdogEnvironmentSeconds = 0;

    const auto options = mitk::ResolveCrashDumpSessionOptions(overrides, Settings(true, 60));

    CPPUNIT_ASSERT_EQUAL(0, options.WatchdogTimeoutSeconds);
    CPPUNIT_ASSERT(Source::EnvironmentVariable == options.WatchdogSource);
  }

  void WatchdogOverrideIsNotClamped()
  {
    mitk::CrashDumpOptionOverrides overrides;
    overrides.WatchdogFlagSeconds = 2;

    const auto options = mitk::ResolveCrashDumpSessionOptions(overrides, Settings(true, 0));

    CPPUNIT_ASSERT_EQUAL(2, options.WatchdogTimeoutSeconds);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCrashDumpSessionOptions)
