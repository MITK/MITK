/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelInspector.h>

#include "QmitkTestQApplication.h"

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QTreeView>

class QmitkMultiLabelInspectorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMultiLabelInspectorTestSuite);
  MITK_TEST(GetGroupIDForRemoval_WithoutSegmentation_IsEmpty);
  MITK_TEST(GetGroupIDForRemoval_OnlyEmptyGroups_IsFocusedGroup);
  MITK_TEST(GetGroupIDForRemoval_WithLabels_IsGroupOfSelectedLabel);
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

  static mitk::MultiLabelSegmentation::Pointer GenerateSegmentation()
  {
    auto seg = mitk::MultiLabelSegmentation::New();
    auto regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 5, 5, 5 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);
    seg->Initialize(regularImage);
    return seg;
  }

  /** Moves the focus onto a group row the way a mouse click does: group rows are not
   * selectable, so QmitkMultiLabelTreeView::selectionCommand() yields NoUpdate and only
   * QTreeView::currentIndex moves. The tree view is the only handle on the focus, as it
   * cannot be set through the public interface of the inspector. */
  static void FocusGroup(QmitkMultiLabelInspector& inspector, int groupID)
  {
    auto* view = inspector.findChild<QTreeView*>();
    CPPUNIT_ASSERT_MESSAGE("Inspector has no tree view.", nullptr != view);

    const auto index = view->model()->index(groupID, 0, QModelIndex());
    CPPUNIT_ASSERT_MESSAGE("Group row does not exist.", index.isValid());

    view->selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
  }

  void GetGroupIDForRemoval_WithoutSegmentation_IsEmpty()
  {
    QmitkMultiLabelInspector inspector;

    CPPUNIT_ASSERT(!inspector.GetGroupIDForRemoval().has_value());
  }

  /** Groups are not selectable, so a segmentation without any labels has no selection at all.
   * Group operations must then fall back to the focused group. */
  void GetGroupIDForRemoval_OnlyEmptyGroups_IsFocusedGroup()
  {
    auto seg = GenerateSegmentation();
    seg->AddGroup();

    QmitkMultiLabelInspector inspector;
    inspector.SetMultiLabelSegmentation(seg);

    CPPUNIT_ASSERT(inspector.GetSelectedLabels().empty());

    auto groupID = inspector.GetGroupIDForRemoval();
    CPPUNIT_ASSERT_MESSAGE("First group is not focused after setting a label-less segmentation.",
      groupID.has_value());
    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(0), groupID.value());

    FocusGroup(inspector, 1);

    groupID = inspector.GetGroupIDForRemoval();
    CPPUNIT_ASSERT(groupID.has_value());
    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(1), groupID.value());
  }

  /** As long as a label is selected, it determines the group, regardless of the focus. */
  void GetGroupIDForRemoval_WithLabels_IsGroupOfSelectedLabel()
  {
    auto seg = GenerateSegmentation();
    seg->AddLabel("A", mitk::Color(1.0f), 0);
    seg->AddGroup();

    QmitkMultiLabelInspector inspector;
    inspector.SetMultiLabelSegmentation(seg);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), inspector.GetSelectedLabels().size());

    FocusGroup(inspector, 1);

    const auto groupID = inspector.GetGroupIDForRemoval();
    CPPUNIT_ASSERT(groupID.has_value());
    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(0), groupID.value());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMultiLabelInspector)
