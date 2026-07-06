/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNLinkSeamWidget.h>
#include <QmitkMxNMultiWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>

/**
 * Tests the link seams' structural behavior against a real
 * QmitkMxNMultiWidget: seam creation for within-splitter adjacent cell
 * pairs (and only those), the pair-state classification per dimension, the
 * chip toggle semantics (fresh pair group / join the linked side / second
 * cell leaves / mixed stays inert), the navigation-bundle toggle, and the
 * non-adjacent fallback query that feeds the corner hue dot. Painting and
 * hover reveal are manual-acceptance territory.
 */
class QmitkMxNLinkSeamTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNLinkSeamTestSuite);

  MITK_TEST(Rebuild_CreatesOneSeamPerAdjacentPair);
  MITK_TEST(State_ClassifiesPairPerDimension);
  MITK_TEST(Toggle_None_CreatesFreshPairGroup);
  MITK_TEST(Toggle_OneSided_JoinsLinkedSidesGroup);
  MITK_TEST(Toggle_Shared_SecondCellLeaves);
  MITK_TEST(Toggle_Mixed_StaysInert);
  MITK_TEST(Bundle_CouplesAndUncouplesNavigation);
  MITK_TEST(NonAdjacentNavGroup_ReportedOnlyWithoutSeamPartner);

  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<QmitkMxNMultiWidget> m_Editor;

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DataStorage = mitk::StandaloneDataStorage::New();

    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->SetLayout(1, 3);  // cells: mxn__widget0 .. mxn__widget2
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  static QString CellId(std::size_t index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  std::vector<QmitkMxNLinkSeamWidget*> Seams() const
  {
    auto seams = m_Editor->findChildren<QmitkMxNLinkSeamWidget*>(
      QString(), Qt::FindDirectChildrenOnly);
    return { seams.begin(), seams.end() };
  }

  bool IsLinked(std::size_t cell, QmitkMxNSyncDimension dimension, const std::string& group) const
  {
    const auto link = m_Editor->GetSyncLink(CellId(cell), dimension);
    return link.has_value() && link->group == group;
  }

  void Rebuild_CreatesOneSeamPerAdjacentPair()
  {
    // 1x3 row: two handles between three cells.
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), this->Seams().size());

    m_Editor->SetLayout(1, 2);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), this->Seams().size());

    m_Editor->SetLayout(1, 1);
    CPPUNIT_ASSERT_MESSAGE("A single cell has no seams", this->Seams().empty());
  }

  void State_ClassifiesPairPerDimension()
  {
    m_Editor->SetLayout(1, 2);
    auto* seam = this->Seams().front();
    using PairState = QmitkMxNLinkSeamWidget::PairState;

    CPPUNIT_ASSERT(PairState::None == seam->GetDimensionState(QmitkMxNSyncDimension::Slice));

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT(PairState::OneSided == seam->GetDimensionState(QmitkMxNSyncDimension::Slice));

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT(PairState::Shared == seam->GetDimensionState(QmitkMxNSyncDimension::Slice));

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "other");
    CPPUNIT_ASSERT(PairState::Mixed == seam->GetDimensionState(QmitkMxNSyncDimension::Slice));

    const auto shared = seam->GetSharedGroups();
    CPPUNIT_ASSERT_MESSAGE("A mixed dimension contributes no shared group", shared.empty());
  }

  void Toggle_None_CreatesFreshPairGroup()
  {
    m_Editor->SetLayout(1, 2);
    auto* seam = this->Seams().front();

    seam->ToggleDimension(QmitkMxNSyncDimension::Windowing);

    const auto first = m_Editor->GetSyncLink(CellId(0), QmitkMxNSyncDimension::Windowing);
    const auto second = m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Windowing);
    CPPUNIT_ASSERT(first.has_value() && second.has_value());
    CPPUNIT_ASSERT_MESSAGE("Both cells join the same fresh group", first->group == second->group);
  }

  void Toggle_OneSided_JoinsLinkedSidesGroup()
  {
    m_Editor->SetLayout(1, 2);
    auto* seam = this->Seams().front();

    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Zoom, "detail");
    seam->ToggleDimension(QmitkMxNSyncDimension::Zoom);

    CPPUNIT_ASSERT_MESSAGE("The unlinked side joins the neighbor's existing group",
                           IsLinked(0, QmitkMxNSyncDimension::Zoom, "detail"));
  }

  void Toggle_Shared_SecondCellLeaves()
  {
    m_Editor->SetLayout(1, 2);
    auto* seam = this->Seams().front();

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");

    seam->ToggleDimension(QmitkMxNSyncDimension::Slice);

    CPPUNIT_ASSERT_MESSAGE("The first cell keeps its link",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "nav"));
    CPPUNIT_ASSERT_MESSAGE("The second cell leaves",
                           !m_Editor->GetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice).has_value());
  }

  void Toggle_Mixed_StaysInert()
  {
    m_Editor->SetLayout(1, 2);
    auto* seam = this->Seams().front();

    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "other");

    seam->ToggleDimension(QmitkMxNSyncDimension::Slice);

    CPPUNIT_ASSERT_MESSAGE("Mixed pairs are the layout editor's job; the seam must not mutate",
                           IsLinked(0, QmitkMxNSyncDimension::Slice, "nav")
                             && IsLinked(1, QmitkMxNSyncDimension::Slice, "other"));
  }

  void Bundle_CouplesAndUncouplesNavigation()
  {
    m_Editor->SetLayout(1, 2);
    auto* seam = this->Seams().front();

    // One existing navigation link seeds the bundle's group choice.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    seam->ToggleNavigationBundle();

    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
    {
      CPPUNIT_ASSERT(IsLinked(0, dimension, "nav"));
      CPPUNIT_ASSERT(IsLinked(1, dimension, "nav"));
    }

    seam->ToggleNavigationBundle();
    for (const auto dimension : { QmitkMxNSyncDimension::Pan, QmitkMxNSyncDimension::Zoom,
                                  QmitkMxNSyncDimension::Slice, QmitkMxNSyncDimension::Crosshair })
    {
      CPPUNIT_ASSERT_MESSAGE("Uncoupling removes the second cell only",
        IsLinked(0, dimension, "nav")
          && !m_Editor->GetSyncLink(CellId(1), dimension).has_value());
    }
  }

  void NonAdjacentNavGroup_ReportedOnlyWithoutSeamPartner()
  {
    // Cells 0 and 2 are not adjacent in a 1x3 row; their coupling has no
    // seam and must surface through the fallback query.
    m_Editor->SetSyncLink(CellId(0), QmitkMxNSyncDimension::Slice, "nav");
    m_Editor->SetSyncLink(CellId(2), QmitkMxNSyncDimension::Slice, "nav");

    const auto fallback = m_Editor->GetNonAdjacentNavGroup(CellId(0));
    CPPUNIT_ASSERT(fallback.has_value());
    CPPUNIT_ASSERT_EQUAL(std::string("nav"), *fallback);

    // Once the direct neighbor shares the group, the seam shows it and the
    // fallback goes quiet.
    m_Editor->SetSyncLink(CellId(1), QmitkMxNSyncDimension::Slice, "nav");
    CPPUNIT_ASSERT(!m_Editor->GetNonAdjacentNavGroup(CellId(0)).has_value());

    CPPUNIT_ASSERT_THROW(m_Editor->GetNonAdjacentNavGroup("mxn__nosuch"), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNLinkSeam)
