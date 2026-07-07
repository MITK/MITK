/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkDataStorageTreeModel.h>

#include <mitkDataNode.h>
#include <mitkImage.h>
#include <mitkStandaloneDataStorage.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QMimeData>
#include <QStack>

#include <memory>
#include <string>
#include <vector>

namespace
{
  /**
   * Live validator for a model's structural change signals.
   *
   * QAbstractItemModel promises its views that every rowsInserted /
   * rowsRemoved names the parent under which the change happened and a range
   * that matches the resulting change in that parent's rowCount(). A
   * QTreeView and a QSortFilterProxyModel keep their internal bookkeeping in
   * sync by trusting exactly this. When a model announces the change under
   * the wrong parent, that bookkeeping is corrupted and later index lookups
   * dereference stale items - the mechanism behind the intermittent Data
   * Manager crashes on drag-and-drop.
   *
   * The guard connects to the model and records, per emitted change, whether
   * the announced parent's rowCount() actually moved by the announced amount.
   * It also rejects a structurally invalid announced range (negative first row
   * or last < first), which no valid model may emit.
   */
  class SignalConsistencyGuard
  {
  public:
    explicit SignalConsistencyGuard(QAbstractItemModel *model)
      : m_Model(model)
    {
      m_Connections.push_back(QObject::connect(
        model, &QAbstractItemModel::rowsAboutToBeInserted,
        [this](const QModelIndex &parent, int first, int last)
        {
          const int before = m_Model->rowCount(parent);
          if (first < 0 || last < first)
            this->Fail(QStringLiteral("rowsAboutToBeInserted announced an invalid range [%1,%2] under parent(row=%3)")
                         .arg(first).arg(last).arg(parent.row()));
          else if (first > before)
            this->Fail(QStringLiteral("rowsAboutToBeInserted announced first row %1 under parent(row=%2), "
                                      "but that parent only has %3 row(s)")
                         .arg(first).arg(parent.row()).arg(before));
          m_InsertBefore.push(before);
        }));

      m_Connections.push_back(QObject::connect(
        model, &QAbstractItemModel::rowsInserted,
        [this](const QModelIndex &parent, int first, int last)
        {
          const int before = m_InsertBefore.isEmpty() ? -1 : m_InsertBefore.pop();
          const int actual = m_Model->rowCount(parent);
          if (before < 0 || actual != before + (last - first + 1))
          {
            this->Fail(QStringLiteral("rowsInserted announced %1 new row(s) under parent(row=%2), "
                                      "but that parent's rowCount went from %3 to %4")
                         .arg(last - first + 1).arg(parent.row()).arg(before).arg(actual));
          }
        }));

      m_Connections.push_back(QObject::connect(
        model, &QAbstractItemModel::rowsAboutToBeRemoved,
        [this](const QModelIndex &parent, int first, int last)
        {
          if (first < 0 || last < first)
            this->Fail(QStringLiteral("rowsAboutToBeRemoved announced an invalid range [%1,%2] under parent(row=%3)")
                         .arg(first).arg(last).arg(parent.row()));
          m_RemoveBefore.push(m_Model->rowCount(parent));
        }));

      m_Connections.push_back(QObject::connect(
        model, &QAbstractItemModel::rowsRemoved,
        [this](const QModelIndex &parent, int first, int last)
        {
          const int before = m_RemoveBefore.isEmpty() ? -1 : m_RemoveBefore.pop();
          const int actual = m_Model->rowCount(parent);
          if (before < 0 || actual != before - (last - first + 1))
          {
            this->Fail(QStringLiteral("rowsRemoved announced %1 removed row(s) under parent(row=%2), "
                                      "but that parent's rowCount went from %3 to %4")
                         .arg(last - first + 1).arg(parent.row()).arg(before).arg(actual));
          }
        }));
    }

    ~SignalConsistencyGuard()
    {
      for (const auto &connection : m_Connections)
        QObject::disconnect(connection);
    }

    SignalConsistencyGuard(const SignalConsistencyGuard &) = delete;
    SignalConsistencyGuard &operator=(const SignalConsistencyGuard &) = delete;

    bool IsConsistent() const { return m_Consistent; }
    std::string FailureText() const { return m_Failure.toStdString(); }

  private:
    void Fail(const QString &message)
    {
      if (m_Consistent)
      {
        m_Consistent = false;
        m_Failure = message;
      }
    }

    QAbstractItemModel *m_Model;
    std::vector<QMetaObject::Connection> m_Connections;
    QStack<int> m_InsertBefore;
    QStack<int> m_RemoveBefore;
    bool m_Consistent = true;
    QString m_Failure;
  };

  mitk::DataNode::Pointer MakeImageNode(const std::string &name)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(mitk::Image::New());
    return node;
  }
}

/**
 * Tests for QmitkDataStorageTreeModel, the model backing the Data Manager
 * view. The focus is dropMimeData: sibling reordering under the default
 * preference and, where a test enables it, hierarchy-changing reparent drops,
 * pinning observable tree behaviour so a fix there cannot silently regress it.
 */
class QmitkDataStorageTreeModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkDataStorageTreeModelTestSuite);
  MITK_TEST(ModelMirrorsStorageHierarchy);
  MITK_TEST(TopLevelReorder_ReordersAndKeepsSignalsConsistent);
  MITK_TEST(ChildReorder_ReordersNodes);
  MITK_TEST(ChildReorder_KeepsSignalsConsistent);
  MITK_TEST(ChildReorder_ToLastPosition_KeepsCorrectOrder);
  MITK_TEST(MultiNodeChildReorder_KeepsSignalsConsistent);
  MITK_TEST(MultiNodeDownwardReorder_KeepsCorrectOrder);
  MITK_TEST(AllChildrenReorder_KeepsSignalsConsistent);
  MITK_TEST(ChildReorder_ParentNotAtRowZero_KeepsCorrectOrder);
  MITK_TEST(Reparent_OntoChildlessNodeAtNonzeroRow_KeepsSignalsConsistent);
  CPPUNIT_TEST_SUITE_END();

  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_ImageA;
  mitk::DataNode::Pointer m_ImageB;
  mitk::DataNode::Pointer m_ChildA;
  mitk::DataNode::Pointer m_ChildB;
  std::unique_ptr<QmitkDataStorageTreeModel> m_Model;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    // Two top-level images; the first carries two derived child nodes.
    //   root
    //   |- imageA
    //   |  |- childA
    //   |  |- childB
    //   |- imageB
    m_ImageA = MakeImageNode("imageA");
    m_ImageB = MakeImageNode("imageB");
    m_ChildA = MakeImageNode("childA");
    m_ChildB = MakeImageNode("childB");

    m_DataStorage->Add(m_ImageA);
    m_DataStorage->Add(m_ImageB);
    m_DataStorage->Add(m_ChildA, m_ImageA);
    m_DataStorage->Add(m_ChildB, m_ImageA);

    // Mirror the production Data Manager wiring: it constructs the model with
    // "Place new nodes on top" defaulting to true. Hierarchy changes via DnD
    // are governed by a separate flag (m_AllowHierarchyChange, off by default),
    // not by this argument.
    m_Model = std::make_unique<QmitkDataStorageTreeModel>(m_DataStorage, true);
  }

  void tearDown() override
  {
    m_Model.reset();
    m_DataStorage = nullptr;
    m_ImageA = nullptr;
    m_ImageB = nullptr;
    m_ChildA = nullptr;
    m_ChildB = nullptr;
  }

  //! Characterization: the model mirrors the storage source hierarchy.
  void ModelMirrorsStorageHierarchy()
  {
    CPPUNIT_ASSERT_EQUAL(2, m_Model->rowCount(QModelIndex()));

    const QModelIndex imageAIndex = m_Model->GetIndex(m_ImageA);
    const QModelIndex imageBIndex = m_Model->GetIndex(m_ImageB);
    CPPUNIT_ASSERT(imageAIndex.isValid());
    CPPUNIT_ASSERT(imageBIndex.isValid());

    CPPUNIT_ASSERT_EQUAL(2, m_Model->rowCount(imageAIndex));
    CPPUNIT_ASSERT_EQUAL(0, m_Model->rowCount(imageBIndex));

    CPPUNIT_ASSERT(m_Model->GetIndex(m_ChildA).parent() == imageAIndex);
    CPPUNIT_ASSERT(m_Model->GetIndex(m_ChildB).parent() == imageAIndex);
  }

  //! Characterization of the currently working case: reordering two
  //! top-level nodes announces the change under the root and swaps them.
  void TopLevelReorder_ReordersAndKeepsSignalsConsistent()
  {
    const mitk::DataNode::Pointer top0 = m_Model->GetNode(m_Model->index(0, 0, QModelIndex()));
    const mitk::DataNode::Pointer top1 = m_Model->GetNode(m_Model->index(1, 0, QModelIndex()));

    SignalConsistencyGuard guard(m_Model.get());

    // Drag the second top-level node and drop it before the first.
    const QModelIndex moverIndex = m_Model->GetIndex(top1);
    const std::unique_ptr<QMimeData> mime(m_Model->mimeData(QModelIndexList{moverIndex}));
    const bool handled = m_Model->dropMimeData(mime.get(), Qt::MoveAction, 0, 0, QModelIndex());

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(0, 0, QModelIndex())) == top1);
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(1, 0, QModelIndex())) == top0);
  }

  //! Reordering the children of a node changes their order in the tree; this
  //! pins the resulting order independently of how the change is announced.
  void ChildReorder_ReordersNodes()
  {
    QModelIndex imageAIndex = m_Model->GetIndex(m_ImageA);
    const mitk::DataNode::Pointer child0 = m_Model->GetNode(m_Model->index(0, 0, imageAIndex));
    const mitk::DataNode::Pointer child1 = m_Model->GetNode(m_Model->index(1, 0, imageAIndex));

    const QModelIndex moverIndex = m_Model->GetIndex(child1);
    const std::unique_ptr<QMimeData> mime(m_Model->mimeData(QModelIndexList{moverIndex}));
    const bool handled = m_Model->dropMimeData(mime.get(), Qt::MoveAction, 0, 0, imageAIndex);

    CPPUNIT_ASSERT(handled);

    imageAIndex = m_Model->GetIndex(m_ImageA);
    CPPUNIT_ASSERT_EQUAL(2, m_Model->rowCount(imageAIndex));
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(0, 0, imageAIndex)) == child1);
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(1, 0, imageAIndex)) == child0);
  }

  //! Reordering the children of a non-root node must announce the insertion
  //! under that node, not under the root. A parentItem vs dropItem mix-up in
  //! dropMimeData would announce it under the wrong parent, corrupting the
  //! view's row bookkeeping - the mechanism behind the intermittent Data
  //! Manager drag-and-drop crash.
  void ChildReorder_KeepsSignalsConsistent()
  {
    const QModelIndex imageAIndex = m_Model->GetIndex(m_ImageA);
    const mitk::DataNode::Pointer child1 = m_Model->GetNode(m_Model->index(1, 0, imageAIndex));

    SignalConsistencyGuard guard(m_Model.get());

    const QModelIndex moverIndex = m_Model->GetIndex(child1);
    const std::unique_ptr<QMimeData> mime(m_Model->mimeData(QModelIndexList{moverIndex}));
    const bool handled = m_Model->dropMimeData(mime.get(), Qt::MoveAction, 0, 0, imageAIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());
  }

  //! Dropping a child past the last row (row == childCount) must move it to the
  //! tail: the append position (childCount) has to be a reachable drop index,
  //! otherwise the last slot cannot be targeted.
  void ChildReorder_ToLastPosition_KeepsCorrectOrder()
  {
    QModelIndex imageAIndex = m_Model->GetIndex(m_ImageA);
    const mitk::DataNode::Pointer child0 = m_Model->GetNode(m_Model->index(0, 0, imageAIndex));
    const mitk::DataNode::Pointer child1 = m_Model->GetNode(m_Model->index(1, 0, imageAIndex));

    SignalConsistencyGuard guard(m_Model.get());

    // Drag the first child and drop it past the last row (Qt passes row == count).
    const QModelIndex moverIndex = m_Model->GetIndex(child0);
    const std::unique_ptr<QMimeData> mime(m_Model->mimeData(QModelIndexList{moverIndex}));
    const bool handled = m_Model->dropMimeData(mime.get(), Qt::MoveAction, 2, 0, imageAIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());

    imageAIndex = m_Model->GetIndex(m_ImageA);
    CPPUNIT_ASSERT_EQUAL(2, m_Model->rowCount(imageAIndex));
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(0, 0, imageAIndex)) == child1);
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(1, 0, imageAIndex)) == child0);
  }

  //! Dragging several children at once removes them one at a time before
  //! re-inserting (dropMimeData relies on GetIndex() staying valid across
  //! those removals). This exercises that multi-item path and checks that the
  //! removal and insertion signals stay consistent and the order is correct.
  void MultiNodeChildReorder_KeepsSignalsConsistent()
  {
    // A third child so we can drag two of them together.
    const mitk::DataNode::Pointer childC = MakeImageNode("childC");
    m_DataStorage->Add(childC, m_ImageA);

    QModelIndex imageAIndex = m_Model->GetIndex(m_ImageA);
    CPPUNIT_ASSERT_EQUAL(3, m_Model->rowCount(imageAIndex));

    const mitk::DataNode::Pointer row0 = m_Model->GetNode(m_Model->index(0, 0, imageAIndex));
    const mitk::DataNode::Pointer row1 = m_Model->GetNode(m_Model->index(1, 0, imageAIndex));
    const mitk::DataNode::Pointer row2 = m_Model->GetNode(m_Model->index(2, 0, imageAIndex));

    SignalConsistencyGuard guard(m_Model.get());

    // Drag the last two children and drop them at the front.
    QModelIndexList movers;
    movers << m_Model->GetIndex(row1) << m_Model->GetIndex(row2);
    const std::unique_ptr<QMimeData> mime(m_Model->mimeData(movers));
    const bool handled = m_Model->dropMimeData(mime.get(), Qt::MoveAction, 0, 0, imageAIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());

    imageAIndex = m_Model->GetIndex(m_ImageA);
    CPPUNIT_ASSERT_EQUAL(3, m_Model->rowCount(imageAIndex));
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(0, 0, imageAIndex)) == row1);
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(1, 0, imageAIndex)) == row2);
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(2, 0, imageAIndex)) == row0);
  }

  //! Dragging several children downward within the same parent must shift the
  //! drop index up by the count of dragged items removed above the drop row, so
  //! the moved block lands at the drop position, not one or more slots too low.
  void MultiNodeDownwardReorder_KeepsCorrectOrder()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    const mitk::DataNode::Pointer parent = MakeImageNode("parent");
    storage->Add(parent);
    for (int i = 0; i < 6; ++i)
      storage->Add(MakeImageNode("c" + std::to_string(i)), parent);

    QmitkDataStorageTreeModel model(storage, false);

    QModelIndex parentIndex = model.GetIndex(parent);
    CPPUNIT_ASSERT_EQUAL(6, model.rowCount(parentIndex));

    std::vector<mitk::DataNode::Pointer> ordered;
    for (int i = 0; i < 6; ++i)
      ordered.push_back(model.GetNode(model.index(i, 0, parentIndex)));

    SignalConsistencyGuard guard(&model);

    // Drag the first two children and drop them between the original 3rd and 4th.
    QModelIndexList movers;
    movers << model.GetIndex(ordered[0]) << model.GetIndex(ordered[1]);
    const std::unique_ptr<QMimeData> mime(model.mimeData(movers));
    const bool handled = model.dropMimeData(mime.get(), Qt::MoveAction, 3, 0, parentIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());

    parentIndex = model.GetIndex(parent);
    CPPUNIT_ASSERT_EQUAL(6, model.rowCount(parentIndex));
    // Both dragged items were above the drop row, so it shifts up by two:
    // [c0, c1, c2, c3, c4, c5] -> [c2, c0, c1, c3, c4, c5].
    CPPUNIT_ASSERT(model.GetNode(model.index(0, 0, parentIndex)) == ordered[2]);
    CPPUNIT_ASSERT(model.GetNode(model.index(1, 0, parentIndex)) == ordered[0]);
    CPPUNIT_ASSERT(model.GetNode(model.index(2, 0, parentIndex)) == ordered[1]);
    CPPUNIT_ASSERT(model.GetNode(model.index(3, 0, parentIndex)) == ordered[3]);
    CPPUNIT_ASSERT(model.GetNode(model.index(4, 0, parentIndex)) == ordered[4]);
    CPPUNIT_ASSERT(model.GetNode(model.index(5, 0, parentIndex)) == ordered[5]);
  }

  //! Dragging every child of a node to a between-drop (row != -1) empties the
  //! insert target in the removal loop before the re-insertion. With the target
  //! emptied, the drop index must still resolve to a valid position within it
  //! so the announced beginInsertRows range matches the actual insertion and
  //! the view's index bookkeeping stays consistent.
  void AllChildrenReorder_KeepsSignalsConsistent()
  {
    const QModelIndex imageAIndex = m_Model->GetIndex(m_ImageA);
    CPPUNIT_ASSERT_EQUAL(2, m_Model->rowCount(imageAIndex));

    const mitk::DataNode::Pointer child0 = m_Model->GetNode(m_Model->index(0, 0, imageAIndex));
    const mitk::DataNode::Pointer child1 = m_Model->GetNode(m_Model->index(1, 0, imageAIndex));

    SignalConsistencyGuard guard(m_Model.get());

    // Drag both children - the node's entire child set - to the front.
    QModelIndexList movers;
    movers << m_Model->GetIndex(child0) << m_Model->GetIndex(child1);
    const std::unique_ptr<QMimeData> mime(m_Model->mimeData(movers));
    const bool handled = m_Model->dropMimeData(mime.get(), Qt::MoveAction, 0, 0, imageAIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());
    CPPUNIT_ASSERT_EQUAL(2, m_Model->rowCount(imageAIndex));
    // Dropping the whole child set at the front preserves their relative order.
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(0, 0, imageAIndex)) == child0);
    CPPUNIT_ASSERT(m_Model->GetNode(m_Model->index(1, 0, imageAIndex)) == child1);
  }

  //! Complements ChildReorder_KeepsSignalsConsistent by putting the child-bearing
  //! node at a nonzero top-level row (higher than its own child count), so its
  //! model index carries a nonzero row(). The reorder must still be announced
  //! under that node and keep the resulting child order - catching any code that
  //! conflates the node's own sibling row with a child position.
  void ChildReorder_ParentNotAtRowZero_KeepsCorrectOrder()
  {
    // Lay out four top-level nodes via explicit layers so the child-bearing
    // node is deterministically last (row 3) and has three children.
    auto storage = mitk::StandaloneDataStorage::New();
    const mitk::DataNode::Pointer top0 = MakeImageNode("top0");
    const mitk::DataNode::Pointer top1 = MakeImageNode("top1");
    const mitk::DataNode::Pointer top2 = MakeImageNode("top2");
    const mitk::DataNode::Pointer parent = MakeImageNode("parentWithChildren");
    top0->SetIntProperty("layer", 30);
    top1->SetIntProperty("layer", 20);
    top2->SetIntProperty("layer", 10);
    parent->SetIntProperty("layer", 1);
    storage->Add(top0);
    storage->Add(top1);
    storage->Add(top2);
    storage->Add(parent);
    storage->Add(MakeImageNode("c0"), parent);
    storage->Add(MakeImageNode("c1"), parent);
    storage->Add(MakeImageNode("c2"), parent);

    QmitkDataStorageTreeModel model(storage, false);

    QModelIndex parentIndex = model.GetIndex(parent);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(parentIndex));
    // The child-bearing node sits at a top-level row (3) above its own child
    // count, so its model index carries a nonzero row() - the aspect this test
    // adds over the row-0 cases. Assert the layout actually exercises that.
    CPPUNIT_ASSERT(parentIndex.row() > model.rowCount(parentIndex) - 1);

    const mitk::DataNode::Pointer childRow0 = model.GetNode(model.index(0, 0, parentIndex));
    const mitk::DataNode::Pointer childRow1 = model.GetNode(model.index(1, 0, parentIndex));
    const mitk::DataNode::Pointer childRow2 = model.GetNode(model.index(2, 0, parentIndex));

    SignalConsistencyGuard guard(&model);

    // Drag the last child to the front.
    const QModelIndex moverIndex = model.GetIndex(childRow2);
    const std::unique_ptr<QMimeData> mime(model.mimeData(QModelIndexList{moverIndex}));
    const bool handled = model.dropMimeData(mime.get(), Qt::MoveAction, 0, 0, parentIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());

    parentIndex = model.GetIndex(parent);
    CPPUNIT_ASSERT_EQUAL(3, model.rowCount(parentIndex));
    CPPUNIT_ASSERT(model.GetNode(model.index(0, 0, parentIndex)) == childRow2);
    CPPUNIT_ASSERT(model.GetNode(model.index(1, 0, parentIndex)) == childRow0);
    CPPUNIT_ASSERT(model.GetNode(model.index(2, 0, parentIndex)) == childRow1);
  }

  //! With hierarchy changes enabled, dropping a node onto a childless target
  //! that sits at a nonzero top-level row must append it as the target's first
  //! child. Using the target's own sibling row as the child insert position
  //! announces an out-of-range beginInsertRows and corrupts the view bookkeeping
  //! - the same crash class the sibling-reorder fix addresses, via the reparent
  //! branch.
  void Reparent_OntoChildlessNodeAtNonzeroRow_KeepsSignalsConsistent()
  {
    auto storage = mitk::StandaloneDataStorage::New();
    const mitk::DataNode::Pointer source = MakeImageNode("source");
    const mitk::DataNode::Pointer target = MakeImageNode("target");
    // Explicit layers so "target" is deterministically the last top-level row
    // (nonzero) and "source" is a separate top-level node carrying the dragged
    // child. Higher layer sorts to the top (row 0), as in the row-not-zero test.
    source->SetIntProperty("layer", 20);
    target->SetIntProperty("layer", 10);
    storage->Add(source);
    storage->Add(target);
    const mitk::DataNode::Pointer child = MakeImageNode("child");
    storage->Add(child, source);

    QmitkDataStorageTreeModel model(storage);
    model.SetAllowHierarchyChange(true);

    const QModelIndex targetIndex = model.GetIndex(target);
    CPPUNIT_ASSERT(targetIndex.row() > 0);
    CPPUNIT_ASSERT_EQUAL(0, model.rowCount(targetIndex));

    SignalConsistencyGuard guard(&model);

    // Drag the child of "source" onto the childless "target" (drop onto node).
    const QModelIndex moverIndex = model.GetIndex(child);
    const std::unique_ptr<QMimeData> mime(model.mimeData(QModelIndexList{moverIndex}));
    const bool handled = model.dropMimeData(mime.get(), Qt::MoveAction, -1, -1, targetIndex);

    CPPUNIT_ASSERT(handled);
    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());
    CPPUNIT_ASSERT_EQUAL(1, model.rowCount(model.GetIndex(target)));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkDataStorageTreeModel)
