/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYITESTHARNESS_H_
#define BERRYITESTHARNESS_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>

#include <QtPlugin>

namespace berry {

/**
 * Represents an arbitrary test harness.
 *
 * @since 3.0
 */
struct BERRY_UI_QT ITestHarness : public virtual Object
{

  berryObjectMacro(berry::ITestHarness);

    /**
     * Runs the tests.
     */
  virtual void RunTests() = 0;

};

}

Q_DECLARE_INTERFACE(berry::ITestHarness, "org.blueberry.ui.ITestHarness")

#endif /* BERRYITESTHARNESS_H_ */
