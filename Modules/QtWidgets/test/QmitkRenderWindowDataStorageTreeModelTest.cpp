/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSignalConsistencyGuard.h"
#include "QmitkTestQApplication.h"

#include <QmitkCustomVariants.h>
#include <QmitkEnums.h>
#include <QmitkRenderWindowDataStorageTreeModel.h>

#include <mitkDataNode.h>
#include <mitkNodePredicateFunction.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkVtkPropRenderer.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>

#include <memory>
#include <string>

namespace
{
  mitk::DataNode::Pointer MakeNode(const std::string &name)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    return node;
  }

  mitk::DataNode::Pointer NodeAt(const QAbstractItemModel &model, const QModelIndex &index)
  {
    return model.data(index, QmitkDataNodeRole).value<mitk::DataNode::Pointer>();
  }

  /** \brief The index of the node, searched in the whole tree, or an invalid index. */
  QModelIndex Find(const QAbstractItemModel &model, const mitk::DataNode *node, const QModelIndex &parent = QModelIndex())
  {
    for (int row = 0; row < model.rowCount(parent); ++row)
    {
      const auto index = model.index(row, 0, parent);

      if (NodeAt(model, index) == node)
        return index;

      if (const auto found = Find(model, node, index); found.isValid())
        return found;
    }

    return QModelIndex();
  }
}

class QmitkRenderWindowDataStorageTreeModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkRenderWindowDataStorageTreeModelTestSuite);
  MITK_TEST(RemoveNodeWithChildren_NodeIsGone);
  MITK_TEST(RemoveNodeWithChildren_ChildrenMoveToItsParent);
  MITK_TEST(SetSources_MovesItemWithChildren);
  MITK_TEST(SetSources_KeepsPersistentIndexes);
  CPPUNIT_TEST_SUITE_END();

  vtkSmartPointer<vtkRenderWindow> m_RenderWindow;
  mitk::VtkPropRenderer::Pointer m_Renderer;
  mitk::StandaloneDataStorage::Pointer m_DataStorage;
  mitk::DataNode::Pointer m_Image;
  mitk::DataNode::Pointer m_Segmentation;
  mitk::DataNode::Pointer m_Surface;
  std::unique_ptr<QmitkRenderWindowDataStorageTreeModel> m_Model;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_RenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    m_Renderer = mitk::VtkPropRenderer::New("QmitkRenderWindowDataStorageTreeModelTest", m_RenderWindow);

    //   image
    //   |- segmentation
    //      |- surface
    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_Image = MakeNode("image");
    m_Segmentation = MakeNode("segmentation");
    m_Surface = MakeNode("surface");
    m_DataStorage->Add(m_Image);
    m_DataStorage->Add(m_Segmentation, m_Image);
    m_DataStorage->Add(m_Surface, m_Segmentation);

    m_Model = std::make_unique<QmitkRenderWindowDataStorageTreeModel>();
    m_Model->SetDataStorage(m_DataStorage);
    m_Model->SetNodePredicate(mitk::NodePredicateFunction::New([](const mitk::DataNode *) { return true; }));
    m_Model->SetCurrentRenderer(m_Renderer);
  }

  void tearDown() override
  {
    m_Model.reset();
    m_DataStorage = nullptr;
    m_Image = nullptr;
    m_Segmentation = nullptr;
    m_Surface = nullptr;
    m_Renderer = nullptr;
    m_RenderWindow = nullptr;
  }

  void RemoveNodeWithChildren_NodeIsGone()
  {
    CPPUNIT_ASSERT(Find(*m_Model, m_Segmentation).isValid());

    m_DataStorage->Remove(m_Segmentation);

    CPPUNIT_ASSERT(!Find(*m_Model, m_Segmentation).isValid());
  }

  void RemoveNodeWithChildren_ChildrenMoveToItsParent()
  {
    SignalConsistencyGuard guard(m_Model.get());

    m_DataStorage->Remove(m_Segmentation);

    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());

    const auto surfaceIndex = Find(*m_Model, m_Surface);
    CPPUNIT_ASSERT(surfaceIndex.isValid());
    CPPUNIT_ASSERT(NodeAt(*m_Model, surfaceIndex.parent()) == m_Image);
  }

  void SetSources_MovesItemWithChildren()
  {
    auto otherImage = MakeNode("other image");
    m_DataStorage->Add(otherImage);

    SignalConsistencyGuard guard(m_Model.get());

    auto sources = mitk::DataStorage::SetOfObjects::New();
    sources->push_back(otherImage);
    m_DataStorage->SetSources(m_Segmentation, sources);

    CPPUNIT_ASSERT_MESSAGE(guard.FailureText(), guard.IsConsistent());

    const auto segmentationIndex = Find(*m_Model, m_Segmentation);
    CPPUNIT_ASSERT(NodeAt(*m_Model, segmentationIndex.parent()) == otherImage);
    CPPUNIT_ASSERT(Find(*m_Model, m_Surface).parent() == segmentationIndex);
  }

  //! A move keeps the indexes that views hold for selection and expanded
  //! state, where a rebuild would invalidate them.
  void SetSources_KeepsPersistentIndexes()
  {
    auto otherImage = MakeNode("other image");
    m_DataStorage->Add(otherImage);

    const QPersistentModelIndex surfaceIndex(Find(*m_Model, m_Surface));

    auto sources = mitk::DataStorage::SetOfObjects::New();
    sources->push_back(otherImage);
    m_DataStorage->SetSources(m_Segmentation, sources);

    CPPUNIT_ASSERT(surfaceIndex.isValid());
    CPPUNIT_ASSERT(NodeAt(*m_Model, surfaceIndex) == m_Surface);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkRenderWindowDataStorageTreeModel)
