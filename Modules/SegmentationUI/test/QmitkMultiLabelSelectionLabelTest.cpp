/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelSelectionLabel.h>

#include "QmitkTestQApplication.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class QmitkMultiLabelSelectionLabelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMultiLabelSelectionLabelTestSuite);
  MITK_TEST(UpdateWidgetWithoutSegmentation_DoesNotCrash);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    // Constructing any QWidget requires a live QApplication. Use the shared,
    // never-destroyed instance so '-platform minimal' from the driver args is honoured.
    EnsureQApplication();
  }

  void tearDown() override
  {
  }

  /**
   * Regression for #858: a non-empty selection with no segmentation set must not
   * dereference the (null) segmentation. On unfixed code SetSelectedLabels ->
   * UpdateWidget null-derefs inside GetMultiLabelSegmentation()->GetLabel(); the
   * process crashes (RED). After the null-segmentation guard in UpdateWidget the
   * widget renders its empty info and the selection is retained (GREEN).
   */
  void UpdateWidgetWithoutSegmentation_DoesNotCrash()
  {
    QmitkMultiLabelSelectionLabel widget; // no segmentation set

    widget.SetSelectedLabels({ 1 });

    CPPUNIT_ASSERT(widget.GetSelectedLabels()
      == mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMultiLabelSelectionLabel)
