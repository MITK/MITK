/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkMxNCellOverlay.h>
#include <QmitkMxNMultiWidget.h>
#include <QmitkRenderWindowWidget.h>

#include <mitkException.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <nlohmann/json.hpp>

#include <QApplication>
#include <QElapsedTimer>
#include <QInputDialog>
#include <QMenu>
#include <QStringList>
#include <QTimer>

#include <functional>
#include <memory>
#include <optional>

/**
 * Window display names: the 'SetCellDisplayName' contract, the round trip
 * through the layout document, and "Rename window..." in the cell's context
 * menu and in its arrange-mode plate menu.
 */
class QmitkMxNCellNameTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkMxNCellNameTestSuite);

  MITK_TEST(SetCellDisplayName_TrimsAndSetsTheName);
  MITK_TEST(SetCellDisplayName_EmptyClearsTheName);
  MITK_TEST(SetCellDisplayName_SignalsOncePerChange);
  MITK_TEST(SetCellDisplayName_UnknownWindowThrows);
  MITK_TEST(Name_RoundTripsThroughTheLayoutDocument);
  MITK_TEST(ContextMenu_OffersRenameOnlyWithoutStructureLock);
  MITK_TEST(PlateMenu_HoldsItsEntries);
  MITK_TEST(Rename_FromContextMenu_SetsTheAcceptedName);
  MITK_TEST(Rename_FromContextMenu_CancelChangesNothing);
  MITK_TEST(Rename_FromPlateMenu_SetsTheAcceptedName);
  MITK_TEST(Rename_LockedWhileTheDialogIsOpen_ChangesNothing);

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
    m_Editor->SetLayout(1, 2);  // cells: mxn__widget0, mxn__widget1
  }

  void tearDown() override
  {
    m_Editor.reset();
    m_DataStorage = nullptr;
  }

  static QString CellId(int index)
  {
    return QStringLiteral("mxn__widget") + QString::number(index);
  }

  QString DisplayName(int index) const
  {
    return m_Editor->GetRenderWindowWidget(CellId(index))->GetDisplayName();
  }

  QmitkMxNCellOverlay* Overlay(int index) const
  {
    auto* overlay = m_Editor->GetRenderWindowWidget(CellId(index))->findChild<QmitkMxNCellOverlay*>();
    CPPUNIT_ASSERT(nullptr != overlay);
    return overlay;
  }

  static QAction* FindAction(const QMenu& menu, const QString& text)
  {
    for (auto* action : menu.actions())
    {
      if (action->text() == text)
      {
        return action;
      }
    }
    return nullptr;
  }

  struct DialogOutcome
  {
    bool opened = false;
    QString prefill;
  };

  /**
   * Trigger 'action' and answer the input dialog it opens: with 'answer', or
   * by cancelling without one. The dialog is modal and opens deferred, so the
   * answer polls from a timer, which also fires inside the dialog's own loop;
   * 'whileOpen' runs there, before the answer.
   */
  static DialogOutcome AnswerRename(QAction* action, const std::optional<QString>& answer,
                                    const std::function<void()>& whileOpen = {})
  {
    DialogOutcome outcome;
    bool answered = false;
    QObject context;
    QTimer poll;
    poll.setInterval(5);
    QObject::connect(&poll, &QTimer::timeout, &context, [&]()
    {
      auto* dialog = qobject_cast<QInputDialog*>(QApplication::activeModalWidget());
      if (nullptr == dialog || answered)
      {
        return;
      }
      answered = true;
      outcome.opened = true;
      outcome.prefill = dialog->textValue();
      if (whileOpen)
      {
        whileOpen();
      }
      if (answer.has_value())
      {
        dialog->setTextValue(*answer);
        dialog->accept();
      }
      else
      {
        dialog->reject();
      }
    });
    poll.start();

    action->trigger();
    QElapsedTimer timer;
    timer.start();
    while (!answered && timer.elapsed() < 3000)
    {
      QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    poll.stop();
    // The deferred handler finishes once the dialog's loop has returned.
    QCoreApplication::processEvents();
    return outcome;
  }

  // --- The setter's contract ---------------------------------------------------

  void SetCellDisplayName_TrimsAndSetsTheName()
  {
    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("  Working plane \t"));
    CPPUNIT_ASSERT_EQUAL(std::string("Working plane"), DisplayName(0).toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The other window keeps its (empty) name", std::string(),
                                 DisplayName(1).toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The sync surfaces show the name", std::string("Working plane"),
                                 m_Editor->CellLabel(CellId(0)).toStdString());
  }

  void SetCellDisplayName_EmptyClearsTheName()
  {
    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("Overview"));
    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("   "));
    CPPUNIT_ASSERT_MESSAGE("White space alone clears the name", DisplayName(0).isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Without a name the sync surfaces show the bare id", std::string("widget0"),
                                 m_Editor->CellLabel(CellId(0)).toStdString());
  }

  void SetCellDisplayName_SignalsOncePerChange()
  {
    int linksChanged = 0;
    int nameChanged = 0;
    QObject::connect(m_Editor.get(), &QmitkMxNMultiWidget::SyncLinksChanged, [&linksChanged]() { ++linksChanged; });
    QObject::connect(m_Editor->GetRenderWindowWidget(CellId(0)).get(), &QmitkRenderWindowWidget::DisplayNameChanged,
                     [&nameChanged](const QString&) { ++nameChanged; });

    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("Detail"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("A change signals once", 1, linksChanged);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The window announces its new name", 1, nameChanged);

    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral(" Detail "));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The same name after trimming is no change", 1, linksChanged);
    CPPUNIT_ASSERT_EQUAL(1, nameChanged);

    m_Editor->SetCellDisplayName(CellId(0), QString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clearing is a change", 2, linksChanged);
    m_Editor->SetCellDisplayName(CellId(0), QString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clearing an empty name is none", 2, linksChanged);
  }

  void SetCellDisplayName_UnknownWindowThrows()
  {
    CPPUNIT_ASSERT_THROW(m_Editor->SetCellDisplayName(QStringLiteral("mxn__nowhere"), QStringLiteral("x")),
                         mitk::Exception);
  }

  static const nlohmann::json* FindWindow(const nlohmann::json& node, const std::string& id)
  {
    if (node.value("id", std::string()) == id)
    {
      return &node;
    }
    if (node.contains("children"))
    {
      for (const auto& child : node.at("children"))
      {
        if (const auto* found = FindWindow(child, id))
        {
          return found;
        }
      }
    }
    return nullptr;
  }

  void Name_RoundTripsThroughTheLayoutDocument()
  {
    m_Editor->SetCellDisplayName(CellId(1), QStringLiteral("Context 1"));
    const auto document = m_Editor->SerializeLayout();

    // A fresh editor under the same name, which is free once the first is gone.
    m_Editor.reset();
    m_Editor = std::make_unique<QmitkMxNMultiWidget>();
    m_Editor->SetDataStorage(m_DataStorage);
    m_Editor->InitializeMultiWidget();
    m_Editor->ApplyLayout(document);

    CPPUNIT_ASSERT_EQUAL(std::string("Context 1"), DisplayName(1).toStdString());
    CPPUNIT_ASSERT_MESSAGE("An unnamed window stays unnamed", DisplayName(0).isEmpty());

    // The schema rejects empty names, so a cleared one must be left out.
    m_Editor->SetCellDisplayName(CellId(1), QString());
    const auto cleared = m_Editor->SerializeLayout();
    const auto* window = FindWindow(cleared.at("root"), CellId(1).toStdString());
    CPPUNIT_ASSERT(nullptr != window);
    CPPUNIT_ASSERT_MESSAGE("A cleared name is not written", !window->contains("name"));
    CPPUNIT_ASSERT_NO_THROW(m_Editor->ApplyLayout(cleared));
  }

  // --- The menus -----------------------------------------------------------------

  void ContextMenu_OffersRenameOnlyWithoutStructureLock()
  {
    QMenu unlocked;
    Overlay(0)->PopulateContextMenu(&unlocked, QPoint());
    CPPUNIT_ASSERT_MESSAGE("Unlocked, the context menu offers the rename",
                           nullptr != FindAction(unlocked, QStringLiteral("Rename window...")));

    m_Editor->SetLayoutLocks(QmitkMxNLayoutLock::Structure);
    QMenu locked;
    Overlay(0)->PopulateContextMenu(&locked, QPoint());
    CPPUNIT_ASSERT_MESSAGE("Under the Structure lock it does not",
                           nullptr == FindAction(locked, QStringLiteral("Rename window...")));
  }

  void PlateMenu_HoldsItsEntries()
  {
    QMenu menu;
    Overlay(0)->PopulatePlateMenu(&menu);

    QStringList entries;
    for (const auto* action : menu.actions())
    {
      if (!action->isSeparator())
      {
        entries << action->text();
      }
    }
    CPPUNIT_ASSERT_EQUAL(std::string("Add to group | Remove from group | Rename window... | Clear selection"),
                         entries.join(QStringLiteral(" | ")).toStdString());
  }

  void Rename_FromContextMenu_SetsTheAcceptedName()
  {
    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("Edit"));
    QMenu menu;
    Overlay(0)->PopulateContextMenu(&menu, QPoint());

    const auto outcome = AnswerRename(FindAction(menu, QStringLiteral("Rename window...")), QStringLiteral(" Edit 2 "));

    CPPUNIT_ASSERT_MESSAGE("The entry opens the input dialog", outcome.opened);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Prefilled with the current name", std::string("Edit"),
                                 outcome.prefill.toStdString());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The accepted name is set, trimmed", std::string("Edit 2"),
                                 DisplayName(0).toStdString());
    CPPUNIT_ASSERT_MESSAGE("No other window is renamed", DisplayName(1).isEmpty());
  }

  void Rename_FromContextMenu_CancelChangesNothing()
  {
    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("Edit"));
    QMenu menu;
    Overlay(0)->PopulateContextMenu(&menu, QPoint());

    const auto outcome = AnswerRename(FindAction(menu, QStringLiteral("Rename window...")), std::nullopt);

    CPPUNIT_ASSERT_MESSAGE("The entry opens the input dialog", outcome.opened);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Cancelling keeps the name", std::string("Edit"), DisplayName(0).toStdString());
  }

  void Rename_FromPlateMenu_SetsTheAcceptedName()
  {
    QMenu menu;
    Overlay(1)->PopulatePlateMenu(&menu);

    const auto outcome = AnswerRename(FindAction(menu, QStringLiteral("Rename window...")), QStringLiteral("3D check"));

    CPPUNIT_ASSERT_MESSAGE("The entry opens the input dialog", outcome.opened);
    CPPUNIT_ASSERT_MESSAGE("Prefilled with the current, empty name", outcome.prefill.isEmpty());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The accepted name is set on that window", std::string("3D check"),
                                 DisplayName(1).toStdString());
    CPPUNIT_ASSERT_MESSAGE("No other window is renamed", DisplayName(0).isEmpty());
  }

  void Rename_LockedWhileTheDialogIsOpen_ChangesNothing()
  {
    // The dialog's own event loop delivers timers and REST calls; a
    // Structure lock set there must stop the rename as one set before.
    m_Editor->SetCellDisplayName(CellId(0), QStringLiteral("Edit"));
    QMenu menu;
    Overlay(0)->PopulateContextMenu(&menu, QPoint());

    const auto outcome = AnswerRename(FindAction(menu, QStringLiteral("Rename window...")), QStringLiteral("Edit 2"),
                                      [this]() { m_Editor->SetLayoutLocks(QmitkMxNLayoutLock::Structure); });

    CPPUNIT_ASSERT_MESSAGE("Fixture: the dialog opened", outcome.opened);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The locked window keeps its name", std::string("Edit"), DisplayName(0).toStdString());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkMxNCellName)
