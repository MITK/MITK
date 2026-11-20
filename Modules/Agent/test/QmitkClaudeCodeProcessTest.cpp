/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkClaudeCodeProcess.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QTemporaryDir>

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace
{
  /** Sets and unsets environment variables of the test process and restores them when it goes out of scope. */
  class EnvironmentGuard
  {
  public:
    EnvironmentGuard() = default;
    EnvironmentGuard(const EnvironmentGuard&) = delete;
    EnvironmentGuard& operator=(const EnvironmentGuard&) = delete;

    ~EnvironmentGuard()
    {
      // Backwards: on Windows, names that differ in their case are one
      // variable, and the first value saved for it is the original one.
      for (auto saved = m_Saved.rbegin(); saved != m_Saved.rend(); ++saved)
      {
        if (saved->second.has_value())
          qputenv(saved->first.c_str(), *saved->second);
        else
          qunsetenv(saved->first.c_str());
      }
    }

    void Set(const std::string& name, const QByteArray& value)
    {
      this->Remember(name);
      qputenv(name.c_str(), value);
    }

    void Unset(const std::string& name)
    {
      this->Remember(name);
      qunsetenv(name.c_str());
    }

  private:
    void Remember(const std::string& name)
    {
      m_Saved.emplace_back(name, qEnvironmentVariableIsSet(name.c_str())
        ? std::optional<QByteArray>(qgetenv(name.c_str()))
        : std::nullopt);
    }

    std::vector<std::pair<std::string, std::optional<QByteArray>>> m_Saved;
  };

  const QString Loopback = "127.0.0.1,localhost,::1";

  QProcessEnvironment GetEnvironment(const QString& trustedCertificateFile = QString(), const QString& executable = QString())
  {
    QmitkClaudeCodeProcess::Options options;
    options.TrustedCertificateFile = trustedCertificateFile;
    options.Executable = executable;
    return QmitkClaudeCodeProcess::GetEnvironment(options);
  }

  void Touch(const QString& path)
  {
    QFile file(path);
    CPPUNIT_ASSERT(file.open(QIODevice::WriteOnly));
    file.write("x");
  }
}

class QmitkClaudeCodeProcessTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkClaudeCodeProcessTestSuite);
  MITK_TEST(TestNoProxyWithoutUserSetting);
  MITK_TEST(TestNoProxyKeepsUserSetting);
  MITK_TEST(TestNoProxyBothSpellingsEqual);
#ifndef Q_OS_WIN
  MITK_TEST(TestNoProxyMergesDifferentSpellings);
#endif
  MITK_TEST(TestTrustedCertificate);
  MITK_TEST(TestTrustedCertificateDoesNotOverrideEnvironment);
  MITK_TEST(TestExecutableDirectoryComesFirstOnPath);
  MITK_TEST(TestApiKeyVariable);
  MITK_TEST(TestParentSessionVariablesRemoved);
  MITK_TEST(TestUserSettingsKept);
  MITK_TEST(TestIsUsableExecutableRefusesNonFiles);
  MITK_TEST(TestIsUsableExecutableAcceptsExecutable);
#ifdef Q_OS_WIN
  MITK_TEST(TestIsUsableExecutableOnlyExe);
#else
  MITK_TEST(TestIsUsableExecutableNeedsPermission);
#endif
  CPPUNIT_TEST_SUITE_END();

public:
  void TestNoProxyWithoutUserSetting()
  {
    EnvironmentGuard guard;
    guard.Unset("no_proxy");
    guard.Unset("NO_PROXY");

    const auto environment = GetEnvironment();

    CPPUNIT_ASSERT_EQUAL(Loopback.toStdString(), environment.value("no_proxy").toStdString());
    CPPUNIT_ASSERT_EQUAL(Loopback.toStdString(), environment.value("NO_PROXY").toStdString());
  }

  void TestNoProxyKeepsUserSetting()
  {
    EnvironmentGuard guard;
    guard.Unset("NO_PROXY");
    guard.Set("no_proxy", "example.org,.corp.example");

    const auto environment = GetEnvironment();
    const auto expected = QString("example.org,.corp.example,") + Loopback;

    CPPUNIT_ASSERT_EQUAL(expected.toStdString(), environment.value("no_proxy").toStdString());
    CPPUNIT_ASSERT_EQUAL(expected.toStdString(), environment.value("NO_PROXY").toStdString());
  }

  void TestNoProxyBothSpellingsEqual()
  {
    EnvironmentGuard guard;
    guard.Set("no_proxy", "example.org");
    guard.Set("NO_PROXY", "example.org");

    const auto environment = GetEnvironment();
    const auto expected = QString("example.org,") + Loopback;

    CPPUNIT_ASSERT_EQUAL(expected.toStdString(), environment.value("no_proxy").toStdString());
    CPPUNIT_ASSERT_EQUAL(expected.toStdString(), environment.value("NO_PROXY").toStdString());
  }

  void TestNoProxyMergesDifferentSpellings()
  {
    EnvironmentGuard guard;
    guard.Set("no_proxy", "lower.example");
    guard.Set("NO_PROXY", "upper.example");

    const auto environment = GetEnvironment();
    const auto expected = QString("lower.example,upper.example,") + Loopback;

    CPPUNIT_ASSERT_EQUAL(expected.toStdString(), environment.value("no_proxy").toStdString());
    CPPUNIT_ASSERT_EQUAL(expected.toStdString(), environment.value("NO_PROXY").toStdString());
  }

  void TestTrustedCertificate()
  {
    EnvironmentGuard guard;
    guard.Unset("NODE_EXTRA_CA_CERTS");

    const QString certificate = QDir::cleanPath(QDir::tempPath() + "/server.pem");

    CPPUNIT_ASSERT_EQUAL(
      QDir::toNativeSeparators(certificate).toStdString(),
      GetEnvironment(certificate).value("NODE_EXTRA_CA_CERTS").toStdString());

    CPPUNIT_ASSERT(!GetEnvironment().contains("NODE_EXTRA_CA_CERTS"));
  }

  void TestTrustedCertificateDoesNotOverrideEnvironment()
  {
    EnvironmentGuard guard;
    guard.Set("NODE_EXTRA_CA_CERTS", "from-the-user.pem");

    CPPUNIT_ASSERT_EQUAL(
      std::string("from-the-user.pem"),
      GetEnvironment(QDir::tempPath() + "/server.pem").value("NODE_EXTRA_CA_CERTS").toStdString());
  }

  void TestExecutableDirectoryComesFirstOnPath()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());

    const auto original = QProcessEnvironment::systemEnvironment().value("PATH");
    const auto first = QDir::toNativeSeparators(directory.path()) + QDir::listSeparator();

    const auto path = GetEnvironment(QString(), directory.filePath("claude")).value("PATH");

    CPPUNIT_ASSERT_MESSAGE(path.toStdString(), path.startsWith(first));
    CPPUNIT_ASSERT_EQUAL(first.toStdString() + original.toStdString(), path.toStdString());

    CPPUNIT_ASSERT_EQUAL(original.toStdString(), GetEnvironment().value("PATH").toStdString());
  }

  void TestApiKeyVariable()
  {
    EnvironmentGuard guard;
    guard.Set("ANTHROPIC_API_KEY", "sk-test");

    QmitkClaudeCodeProcess::Options options;

    CPPUNIT_ASSERT_EQUAL(std::string("sk-test"), QmitkClaudeCodeProcess::GetEnvironment(options).value("ANTHROPIC_API_KEY").toStdString());

    options.IgnoreApiKeyVariable = true;

    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::GetEnvironment(options).contains("ANTHROPIC_API_KEY"));
  }

  void TestParentSessionVariablesRemoved()
  {
    EnvironmentGuard guard;
    guard.Set("CLAUDECODE", "1");
    guard.Set("CLAUDE_CODE_SESSION_ID", "parent");
    guard.Set("CLAUDE_CODE_MESSAGING_TOKEN", "parent");

    const auto environment = GetEnvironment();

    CPPUNIT_ASSERT(!environment.contains("CLAUDECODE"));
    CPPUNIT_ASSERT(!environment.contains("CLAUDE_CODE_SESSION_ID"));
    CPPUNIT_ASSERT(!environment.contains("CLAUDE_CODE_MESSAGING_TOKEN"));
  }

  void TestUserSettingsKept()
  {
    EnvironmentGuard guard;
    guard.Set("CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC", "1");
    guard.Set("CLAUDE_CODE_USE_BEDROCK", "1");
    guard.Set("CLAUDE_CODE_OAUTH_TOKEN", "user");

    const auto environment = GetEnvironment();

    CPPUNIT_ASSERT_EQUAL(std::string("1"), environment.value("CLAUDE_CODE_DISABLE_NONESSENTIAL_TRAFFIC").toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("1"), environment.value("CLAUDE_CODE_USE_BEDROCK").toStdString());
    CPPUNIT_ASSERT_EQUAL(std::string("user"), environment.value("CLAUDE_CODE_OAUTH_TOKEN").toStdString());
  }

  void TestIsUsableExecutableRefusesNonFiles()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());

    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::IsUsableExecutable(QString()));
    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::IsUsableExecutable(directory.filePath("missing.exe")));
    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::IsUsableExecutable(directory.filePath("missing")));

    // Also not a directory that is named like an executable.
    CPPUNIT_ASSERT(QDir(directory.path()).mkdir("directory.exe"));
    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::IsUsableExecutable(directory.filePath("directory.exe")));
    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::IsUsableExecutable(directory.path()));
  }

  void TestIsUsableExecutableAcceptsExecutable()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());

#ifdef Q_OS_WIN
    const auto path = directory.filePath("claude.exe");
    Touch(path);
#else
    const auto path = directory.filePath("claude");
    Touch(path);
    CPPUNIT_ASSERT(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner));
#endif

    CPPUNIT_ASSERT(QmitkClaudeCodeProcess::IsUsableExecutable(path));
  }

  void TestIsUsableExecutableOnlyExe()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());

    // The wrapper of an npm install would run through cmd.exe.
    for (const auto& name : { "claude.cmd", "claude.bat", "claude" })
    {
      Touch(directory.filePath(name));
      CPPUNIT_ASSERT_MESSAGE(name, !QmitkClaudeCodeProcess::IsUsableExecutable(directory.filePath(name)));
    }

    Touch(directory.filePath("claude.EXE"));
    CPPUNIT_ASSERT(QmitkClaudeCodeProcess::IsUsableExecutable(directory.filePath("claude.EXE")));
  }

  void TestIsUsableExecutableNeedsPermission()
  {
    const QTemporaryDir directory;
    CPPUNIT_ASSERT(directory.isValid());

    const auto path = directory.filePath("claude");
    Touch(path);
    CPPUNIT_ASSERT(QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner));

    CPPUNIT_ASSERT(!QmitkClaudeCodeProcess::IsUsableExecutable(path));
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkClaudeCodeProcess)
