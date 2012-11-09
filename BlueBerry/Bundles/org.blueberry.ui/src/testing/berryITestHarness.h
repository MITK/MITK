/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef BERRYITESTHARNESS_H_
#define BERRYITESTHARNESS_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_ui_Export.h>

#include <QtPlugin>

namespace berry {

/**
 * Represents an arbitrary test harness.
 *
 * @since 3.0
 */
struct BERRY_UI ITestHarness : public virtual Object {

  berryInterfaceMacro(ITestHarness, berry)

    /**
     * Runs the tests.
     */
  virtual void RunTests() = 0;

};

}

Q_DECLARE_INTERFACE(berry::ITestHarness, "org.blueberry.ITestHarness")

#endif /* BERRYITESTHARNESS_H_ */
