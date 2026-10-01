/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkTestQApplication.h"

#include <QmitkCrashDumpDialog.h>
#include <QmitkCrashDumpListWidget.h>
#include <QmitkCrashDumpManagerDialog.h>

#include <mitkCrashDumpFacility.h>
#include <mitkICrashReportService.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <usGetModuleContext.h>
#include <usModuleContext.h>
#include <usServiceRegistration.h>

#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>

namespace
{
  class TestReportService : public mitk::ICrashReportService
  {
  public:
    void FileReport(const std::vector<mitk::CrashDumpInfo>& dumps, QWidget*) override
    {
      Received = dumps;
      ++Calls;
    }

    std::vector<mitk::CrashDumpInfo> Received;
    int Calls = 0;
  };

  std::vector<QWidget*> VisibleManagers()
  {
    std::vector<QWidget*> managers;

    for (auto* widget : QApplication::topLevelWidgets())
    {
      if (widget->objectName() == "QmitkCrashDumpManagerDialog" && widget->isVisible())
        managers.push_back(widget);
    }

    return managers;
  }

  /** Runs \p action on the next modal dialog, from inside its exec(). */
  QTimer* WhenModalDialogShows(std::function<void(QWidget*)> action)
  {
    auto* timer = new QTimer;
    timer->setInterval(20);

    QObject::connect(timer, &QTimer::timeout, [timer, action] {
      if (auto* modal = QApplication::activeModalWidget())
      {
        timer->stop();
        action(modal);
      }
    });

    timer->start();
    return timer;
  }
}

class QmitkCrashDumpUiTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkCrashDumpUiTestSuite);
  MITK_TEST(ListWidgetShowsColumnsAndSelection);
  MITK_TEST(NoReportServiceMeansNoReportButton);
  MITK_TEST(ManagerIsSingleInstanceAndReportsTheSelection);
  MITK_TEST(NextStartDialogKeepAcknowledges);
  MITK_TEST(NextStartDialogCloseRemoves);
  MITK_TEST(NextStartDialogFileReportKeepsTheDumps);
  CPPUNIT_TEST_SUITE_END();

  std::filesystem::path m_DatabaseDirectory;
  std::filesystem::path m_CrashDump;
  std::filesystem::path m_OnDemandDump;
  std::filesystem::path m_FreezeDump;
  TestReportService m_Service;
  us::ServiceRegistration<mitk::ICrashReportService> m_Registration;

  std::filesystem::path CreateDump(const std::string& relativePath, int ageInSeconds)
  {
    const auto path = m_DatabaseDirectory / relativePath;

    std::filesystem::create_directories(path.parent_path());
    std::ofstream(path) << "minidump placeholder";
    std::filesystem::last_write_time(path,
      std::filesystem::file_time_type::clock::now() - std::chrono::seconds(ageInSeconds));

    return path;
  }

  /** The run-info sidecar the facility keeps next to a dump. Temporary
   *  directory paths are plain ASCII, so no JSON escaping is needed. */
  static void WriteSidecar(const std::filesystem::path& dump, const std::string& release,
    const std::filesystem::path& logFile)
  {
    std::ofstream(std::filesystem::path(dump) += ".json")
      << "{ \"release\": \"" << release << "\", \"installDirectory\": \"\", \"logFile\": \""
      << logFile.generic_string() << "\" }";
  }

  void RegisterService()
  {
    m_Registration = us::GetModuleContext()->RegisterService<mitk::ICrashReportService>(&m_Service);
  }

public:
  void setUp() override
  {
    EnsureQApplication();

    m_DatabaseDirectory = mitk::IOUtil::CreateTemporaryDirectory("QmitkCrashDumpUiTest_XXXXXX");
    m_Service = TestReportService();

    const auto log = m_DatabaseDirectory / "session.log";
    std::ofstream(log) << "log";

    m_CrashDump = this->CreateDump("reports/crash.dmp", 30);
    m_OnDemandDump = this->CreateDump("mitk-snapshots/ondemand.dmp", 20);
    m_FreezeDump = this->CreateDump("mitk-pending-freeze/freeze.dmp", 10);

    WriteSidecar(m_CrashDump, "MITK Test 1.0", log);
    std::ofstream(std::filesystem::path(m_CrashDump) += ".log") << "kept log";
    WriteSidecar(m_OnDemandDump, "", m_DatabaseDirectory / "rotated-away.log");

    mitk::CrashDumpFacility::Config config;
    config.DatabaseDirectory = m_DatabaseDirectory;
    config.ApplicationName = "QmitkCrashDumpUiTest";
    config.ApplicationVersion = "1.0";
    config.Arm = false;
    mitk::CrashDumpFacility::Initialize(config);
  }

  void tearDown() override
  {
    for (auto* manager : VisibleManagers())
      manager->close();
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    if (m_Registration)
      m_Registration.Unregister();
    m_Registration = {};

    std::error_code error;
    std::filesystem::remove_all(m_DatabaseDirectory, error);
  }

  void ListWidgetShowsColumnsAndSelection()
  {
    QmitkCrashDumpListWidget widget;
    widget.SetDumps(mitk::CrashDumpFacility::ListAllDumps());

    auto* tree = widget.findChild<QTreeWidget*>("crashDumpTree");
    CPPUNIT_ASSERT(tree != nullptr);
    CPPUNIT_ASSERT_EQUAL(3, tree->topLevelItemCount());

    using Column = QmitkCrashDumpListWidget::Column;
    const auto text = [tree](int row, Column column) { return tree->topLevelItem(row)->text(column).toStdString(); };

    CPPUNIT_ASSERT_EQUAL(std::string("Unresponsive (terminated)"), text(0, Column::KindColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("Captured on request"), text(1, Column::KindColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("Crash"), text(2, Column::KindColumn));

    CPPUNIT_ASSERT_EQUAL(std::string("unknown"), text(0, Column::VersionColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("unknown"), text(1, Column::VersionColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK Test 1.0"), text(2, Column::VersionColumn));

    CPPUNIT_ASSERT_EQUAL(std::string("unknown"), text(0, Column::LogColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("not available"), text(1, Column::LogColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("available"), text(2, Column::LogColumn));

    CPPUNIT_ASSERT_EQUAL(std::string("new"), text(0, Column::StatusColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("kept"), text(1, Column::StatusColumn));
    CPPUNIT_ASSERT_EQUAL(std::string("new"), text(2, Column::StatusColumn));

    CPPUNIT_ASSERT(widget.GetSelectedDumps().empty());

    tree->topLevelItem(2)->setSelected(true);
    tree->topLevelItem(0)->setSelected(true);

    const auto selected = widget.GetSelectedDumps();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), selected.size());
    CPPUNIT_ASSERT(m_FreezeDump == selected[0].Path);
    CPPUNIT_ASSERT(m_CrashDump == selected[1].Path);
  }

  void NoReportServiceMeansNoReportButton()
  {
    CPPUNIT_ASSERT(mitk::GetCrashReportService() == nullptr);

    QmitkCrashDumpManagerDialog::ShowManager();
    const auto managers = VisibleManagers();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), managers.size());

    auto* button = managers.front()->findChild<QPushButton*>("fileReportButton");
    CPPUNIT_ASSERT(button != nullptr);
    CPPUNIT_ASSERT(!button->isVisible());
  }

  void ManagerIsSingleInstanceAndReportsTheSelection()
  {
    this->RegisterService();
    CPPUNIT_ASSERT(mitk::GetCrashReportService() == &m_Service);

    QmitkCrashDumpManagerDialog::ShowManager();
    QmitkCrashDumpManagerDialog::ShowManager();

    const auto managers = VisibleManagers();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("a second request raises the same instance", std::size_t(1), managers.size());

    auto* tree = managers.front()->findChild<QTreeWidget*>("crashDumpTree");
    tree->topLevelItem(1)->setSelected(true);

    auto* button = managers.front()->findChild<QPushButton*>("fileReportButton");
    CPPUNIT_ASSERT(button->isVisible());
    button->click();

    CPPUNIT_ASSERT_EQUAL(1, m_Service.Calls);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), m_Service.Received.size());
    CPPUNIT_ASSERT(m_OnDemandDump == m_Service.Received.front().Path);
    CPPUNIT_ASSERT_MESSAGE("filing a report must not delete the dump", std::filesystem::exists(m_OnDemandDump));
  }

  void NextStartDialogKeepAcknowledges()
  {
    bool hadReportButton = true;
    auto* timer = WhenModalDialogShows([&hadReportButton](QWidget* dialog) {
      hadReportButton = dialog->findChild<QPushButton*>("fileReportButton") != nullptr;
      dialog->findChild<QPushButton*>("keepButton")->click();
    });

    QmitkCrashDumpDialog::ShowIfCrashedLastRun();
    delete timer;

    CPPUNIT_ASSERT(!hadReportButton);
    CPPUNIT_ASSERT(std::filesystem::exists(m_CrashDump));
    CPPUNIT_ASSERT(std::filesystem::exists(m_FreezeDump));
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListUnacknowledgedDumps().empty());

    bool shownAgain = false;
    timer = WhenModalDialogShows([&shownAgain](QWidget* dialog) {
      shownAgain = true;
      dialog->close();
    });

    QmitkCrashDumpDialog::ShowIfCrashedLastRun();
    QCoreApplication::processEvents();
    delete timer;

    CPPUNIT_ASSERT_MESSAGE("kept dumps must never trigger the dialog again", !shownAgain);
  }

  void NextStartDialogCloseRemoves()
  {
    auto* timer = WhenModalDialogShows([](QWidget* dialog) { dialog->close(); });

    QmitkCrashDumpDialog::ShowIfCrashedLastRun();
    delete timer;

    CPPUNIT_ASSERT(!std::filesystem::exists(m_CrashDump));
    CPPUNIT_ASSERT(!std::filesystem::exists(m_FreezeDump));
    CPPUNIT_ASSERT_MESSAGE("on-demand snapshots are not the dialog's to remove", std::filesystem::exists(m_OnDemandDump));
  }

  void NextStartDialogFileReportKeepsTheDumps()
  {
    this->RegisterService();

    auto* timer = WhenModalDialogShows([](QWidget* dialog) {
      dialog->findChild<QPushButton*>("fileReportButton")->click();
    });

    QmitkCrashDumpDialog::ShowIfCrashedLastRun();
    delete timer;

    CPPUNIT_ASSERT_EQUAL(1, m_Service.Calls);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), m_Service.Received.size());
    CPPUNIT_ASSERT(std::filesystem::exists(m_CrashDump));
    CPPUNIT_ASSERT(std::filesystem::exists(m_FreezeDump));
    CPPUNIT_ASSERT(mitk::CrashDumpFacility::ListUnacknowledgedDumps().empty());
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkCrashDumpUi)
