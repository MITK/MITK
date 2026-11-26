/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentMemory.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTemporaryDir>

#include <stdexcept>
#include <string>

namespace
{
  QByteArray ReadFile(const QString& path)
  {
    QFile file(path);
    CPPUNIT_ASSERT(file.open(QIODevice::ReadOnly));
    return file.readAll();
  }

  void WriteFile(const QString& path, const QByteArray& content)
  {
    QFile file(path);
    CPPUNIT_ASSERT(file.open(QIODevice::WriteOnly | QIODevice::Truncate));
    file.write(content);
  }

  /** A memory file as the class writes it, with notes of the ids 1, 2, ... */
  void WriteNotes(const QString& path, const QStringList& texts)
  {
    QJsonArray notes;
    int id = 0;

    for (const auto& text : texts)
      notes.append(QJsonObject{ { "id", ++id }, { "text", text } });

    WriteFile(path, QJsonDocument(QJsonObject{ { "nextId", id + 1 }, { "notes", notes } }).toJson());
  }

  std::string ToString(const QString& text)
  {
    return text.toStdString();
  }
}

class QmitkAgentMemoryTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkAgentMemoryTestSuite);
  MITK_TEST(TestMissingFileIsAnEmptyMemory);
  MITK_TEST(TestAddEditRemove);
  MITK_TEST(TestTextIsNormalized);
  MITK_TEST(TestInvalidChanges);
  MITK_TEST(TestIdsAreNotReused);
  MITK_TEST(TestPersistsAcrossInstances);
  MITK_TEST(TestClear);
  MITK_TEST(TestNoteLengthLimit);
  MITK_TEST(TestNoteCountLimit);
  MITK_TEST(TestDescribe);
  MITK_TEST(TestDescribeLeavesOutNotesAboveTheLimits);
  MITK_TEST(TestUnreadableFileIsNotWrittenOver);
  MITK_TEST(TestUnreadableFileIsReadAgain);
  MITK_TEST(TestTwoInstancesKeepEachOthersNotes);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestMissingFileIsAnEmptyMemory()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");

    const QmitkAgentMemory memory(path);

    CPPUNIT_ASSERT(memory.IsReadable());
    CPPUNIT_ASSERT(memory.GetNotes().empty());
    CPPUNIT_ASSERT_EQUAL(ToString(path), ToString(memory.GetFilePath()));
    CPPUNIT_ASSERT(!QFile::exists(path));
  }

  void TestAddEditRemove()
  {
    const QTemporaryDir directory;
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    const auto first = memory.Add("first");
    const auto second = memory.Add("second");

    CPPUNIT_ASSERT(first != second);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), memory.GetNotes().size());

    memory.SetText(first, "changed");

    CPPUNIT_ASSERT_EQUAL(std::string("changed"), ToString(memory.GetNotes()[0].Text));
    CPPUNIT_ASSERT_EQUAL(std::string("second"), ToString(memory.GetNotes()[1].Text));

    memory.Remove(first);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), memory.GetNotes().size());
    CPPUNIT_ASSERT_EQUAL(second, memory.GetNotes()[0].Id);
  }

  void TestTextIsNormalized()
  {
    const QTemporaryDir directory;
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    const auto id = memory.Add("  one\n\ttwo   three ");
    CPPUNIT_ASSERT_EQUAL(std::string("one two three"), ToString(memory.GetNotes()[0].Text));

    memory.SetText(id, "\n four  five\n");
    CPPUNIT_ASSERT_EQUAL(std::string("four five"), ToString(memory.GetNotes()[0].Text));
  }

  void TestInvalidChanges()
  {
    const QTemporaryDir directory;
    QmitkAgentMemory memory(directory.filePath("memory.json"));
    const auto id = memory.Add("note");

    CPPUNIT_ASSERT_THROW(memory.Add(""), std::invalid_argument);
    CPPUNIT_ASSERT_THROW(memory.Add(" \n\t "), std::invalid_argument);
    CPPUNIT_ASSERT_THROW(memory.SetText(id, " "), std::invalid_argument);
    CPPUNIT_ASSERT_THROW(memory.SetText(id + 1, "text"), std::invalid_argument);
    CPPUNIT_ASSERT_THROW(memory.Remove(id + 1), std::invalid_argument);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), memory.GetNotes().size());
    CPPUNIT_ASSERT_EQUAL(std::string("note"), ToString(memory.GetNotes()[0].Text));
  }

  void TestIdsAreNotReused()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");

    int last = 0;

    {
      QmitkAgentMemory memory(path);
      memory.Add("first");
      last = memory.Add("second");
      memory.Remove(last);

      CPPUNIT_ASSERT(memory.Add("third") > last);
    }

    // Also not by an instance that never saw the removed note.
    QmitkAgentMemory memory(path);
    CPPUNIT_ASSERT(memory.Add("fourth") > last + 1);
  }

  void TestPersistsAcrossInstances()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");

    int id = 0;

    {
      QmitkAgentMemory memory(path);
      memory.Add("first");
      id = memory.Add("second");
      memory.SetText(id, "edited");
    }

    const QmitkAgentMemory memory(path);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), memory.GetNotes().size());
    CPPUNIT_ASSERT_EQUAL(std::string("first"), ToString(memory.GetNotes()[0].Text));
    CPPUNIT_ASSERT_EQUAL(id, memory.GetNotes()[1].Id);
    CPPUNIT_ASSERT_EQUAL(std::string("edited"), ToString(memory.GetNotes()[1].Text));
  }

  void TestClear()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");

    QmitkAgentMemory memory(path);
    memory.Add("first");
    const auto second = memory.Add("second");

    memory.Clear();

    CPPUNIT_ASSERT(memory.GetNotes().empty());
    CPPUNIT_ASSERT(QmitkAgentMemory(path).GetNotes().empty());
    CPPUNIT_ASSERT(memory.Add("third") > second);
  }

  void TestNoteLengthLimit()
  {
    const QTemporaryDir directory;
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    const auto longest = QString(QmitkAgentMemory::MaxNoteLength, QChar('x'));
    const auto id = memory.Add(longest);

    CPPUNIT_ASSERT_EQUAL(longest.size(), memory.GetNotes()[0].Text.size());
    CPPUNIT_ASSERT_THROW(memory.Add(longest + "x"), std::invalid_argument);
    CPPUNIT_ASSERT_THROW(memory.SetText(id, longest + "x"), std::invalid_argument);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), memory.GetNotes().size());
    CPPUNIT_ASSERT_EQUAL(longest.size(), memory.GetNotes()[0].Text.size());
  }

  void TestNoteCountLimit()
  {
    const QTemporaryDir directory;
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    for (std::size_t i = 0; i < QmitkAgentMemory::MaxNoteCount; ++i)
      memory.Add(QString("note %1").arg(i));

    CPPUNIT_ASSERT_THROW(memory.Add("one too many"), std::invalid_argument);
    CPPUNIT_ASSERT_EQUAL(QmitkAgentMemory::MaxNoteCount, memory.GetNotes().size());

    // Room again after a note is gone.
    memory.Remove(memory.GetNotes().front().Id);
    CPPUNIT_ASSERT_NO_THROW(memory.Add("room again"));
  }

  void TestDescribe()
  {
    const QTemporaryDir directory;
    QmitkAgentMemory memory(directory.filePath("memory.json"));

    CPPUNIT_ASSERT(memory.Describe().contains("You have not saved any notes yet."));

    const auto first = memory.Add("the first note");
    const auto second = memory.Add("the second note");
    const auto description = memory.Describe();

    CPPUNIT_ASSERT(!description.contains("You have not saved any notes yet."));
    CPPUNIT_ASSERT_MESSAGE(ToString(description), description.contains(QString("[%1] the first note").arg(first)));
    CPPUNIT_ASSERT_MESSAGE(ToString(description), description.contains(QString("[%1] the second note").arg(second)));

    memory.Remove(first);
    CPPUNIT_ASSERT(!memory.Describe().contains("the first note"));
  }

  void TestDescribeLeavesOutNotesAboveTheLimits()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");

    const QString tooLong(QmitkAgentMemory::MaxNoteLength + 1, QChar('x'));

    // A note that is too long first, then one more than fit.
    QStringList texts = { tooLong };

    for (std::size_t i = 0; i <= QmitkAgentMemory::MaxNoteCount; ++i)
      texts << QString("short-%1").arg(i);

    WriteNotes(path, texts);

    QmitkAgentMemory memory(path);
    const auto description = memory.Describe();

    // They stay in the notes, so that the user can edit them.
    CPPUNIT_ASSERT_EQUAL(texts.size(), static_cast<qsizetype>(memory.GetNotes().size()));

    CPPUNIT_ASSERT(!description.contains(tooLong));
    CPPUNIT_ASSERT(description.contains("short-0"));
    CPPUNIT_ASSERT(description.contains(QString("short-%1\n").arg(QmitkAgentMemory::MaxNoteCount - 1)));
    CPPUNIT_ASSERT(!description.contains(QString("short-%1").arg(QmitkAgentMemory::MaxNoteCount)));
  }

  void TestUnreadableFileIsNotWrittenOver()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");
    const QByteArray damaged = "{ \"notes\": [ not json";
    WriteFile(path, damaged);

    QmitkAgentMemory memory(path);

    CPPUNIT_ASSERT(!memory.IsReadable());
    CPPUNIT_ASSERT(memory.GetNotes().empty());

    CPPUNIT_ASSERT_THROW(memory.Add("new"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(memory.SetText(1, "new"), std::runtime_error);
    CPPUNIT_ASSERT_THROW(memory.Remove(1), std::runtime_error);
    CPPUNIT_ASSERT_THROW(memory.Clear(), std::runtime_error);

    CPPUNIT_ASSERT(ReadFile(path) == damaged);
    CPPUNIT_ASSERT(memory.GetNotes().empty());
  }

  void TestUnreadableFileIsReadAgain()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");
    WriteFile(path, "not json");

    QmitkAgentMemory memory(path);
    CPPUNIT_ASSERT(!memory.IsReadable());

    WriteNotes(path, { "kept" });

    CPPUNIT_ASSERT(memory.IsReadable());
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), memory.GetNotes().size());
    CPPUNIT_ASSERT_NO_THROW(memory.Add("added"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), QmitkAgentMemory(path).GetNotes().size());
  }

  void TestTwoInstancesKeepEachOthersNotes()
  {
    const QTemporaryDir directory;
    const auto path = directory.filePath("memory.json");

    QmitkAgentMemory first(path);
    QmitkAgentMemory second(path);

    const auto idOfFirst = first.Add("from the first");
    const auto idOfSecond = second.Add("from the second");

    CPPUNIT_ASSERT(idOfFirst != idOfSecond);

    const QmitkAgentMemory reader(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), reader.GetNotes().size());
    CPPUNIT_ASSERT_EQUAL(std::string("from the first"), ToString(reader.GetNotes()[0].Text));
    CPPUNIT_ASSERT_EQUAL(std::string("from the second"), ToString(reader.GetNotes()[1].Text));

    // A removal by the first instance keeps the note that only the second one added.
    first.Remove(idOfFirst);

    const QmitkAgentMemory afterRemoval(path);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), afterRemoval.GetNotes().size());
    CPPUNIT_ASSERT_EQUAL(idOfSecond, afterRemoval.GetNotes()[0].Id);

    // And the ids that the second instance used are not used again by the first.
    CPPUNIT_ASSERT(first.Add("later") > idOfSecond);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkAgentMemory)
