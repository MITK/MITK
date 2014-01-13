#include "mitkTestingMacros.h"

#include <QSignalSpy>

#include "QWonderString.h"

/**
  Just SOME kind of testing, nothing meaningful.

  This test is just company to the Qt4Qt5TestModule,
  which is mean to demostrate how to implement a
  module for both Qt4 and Qt5
*/
int QWonderStringTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_CONDITION( !QWonderString::penguinUID().isEmpty(), "Penguin registry working" )

  QWonderString s;

  QSignalSpy happyCounter( &s, SIGNAL(happy()));
  QSignalSpy sadCounter( &s, SIGNAL(sad()));

  MITK_TEST_CONDITION( happyCounter.count() == 0 && sadCounter.count() == 0, "No feelings in the beginning" )

  s.setText("Happy feet");
  MITK_TEST_CONDITION( happyCounter.count() == 1, "Penguins are happy" )

  s.setText("Talk too much");
  MITK_TEST_CONDITION( sadCounter.count() == 1, "Penguin ears bleed" )

  s.setText("Fishy fish");
  MITK_TEST_CONDITION( happyCounter.count() == 2, "Penguins sleepy" )

  s.setText("Knok knok");
  MITK_TEST_CONDITION( sadCounter.count() == 2, "Penguins irritated" )

  return EXIT_SUCCESS;
}
