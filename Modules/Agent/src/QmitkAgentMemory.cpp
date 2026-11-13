/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkAgentMemory.h>

#include <mitkLog.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace
{
  /** The message reaches the agent as well, so it leaves out the path, which holds the name of the user's account. */
  [[noreturn]] void ThrowWriteError(const QString& filePath)
  {
    MITK_ERROR << "Cannot write the agent's memory to " << filePath.toStdString();
    throw std::runtime_error("The agent's memory could not be saved. The log of the Workbench tells more.");
  }

  void WriteNotes(const QString& filePath, const std::vector<QmitkAgentMemory::Note>& notes, int nextId)
  {
    QJsonArray array;

    for (const auto& note : notes)
      array.append(QJsonObject{ { "id", note.Id }, { "text", note.Text } });

    QSaveFile file(filePath);

    if (!file.open(QIODevice::WriteOnly))
      ThrowWriteError(filePath);

    file.write(QJsonDocument(QJsonObject{ { "nextId", nextId }, { "notes", array } }).toJson());

    if (!file.commit())
      ThrowWriteError(filePath);
  }
}

QmitkAgentMemory::QmitkAgentMemory(const QString& filePath)
  : m_FilePath(filePath),
    m_NextId(1)
{
  QFile file(filePath);

  if (!file.exists())
    return;

  if (!file.open(QIODevice::ReadOnly))
  {
    MITK_WARN << "Cannot read the agent's memory from " << filePath.toStdString();
    return;
  }

  const auto document = QJsonDocument::fromJson(file.readAll());

  if (!document.isObject())
  {
    MITK_WARN << "The agent's memory in " << filePath.toStdString() << " is not valid JSON.";
    return;
  }

  const auto object = document.object();

  for (const auto& entry : object.value("notes").toArray())
  {
    const auto note = entry.toObject();
    const auto id = note.value("id").toInt();
    const auto text = note.value("text").toString().simplified();

    if (id > 0 && !text.isEmpty())
    {
      m_Notes.push_back({ id, text });
      m_NextId = std::max(m_NextId, id + 1);
    }
  }

  m_NextId = std::max(m_NextId, object.value("nextId").toInt());
}

const std::vector<QmitkAgentMemory::Note>& QmitkAgentMemory::GetNotes() const
{
  return m_Notes;
}

QString QmitkAgentMemory::Normalize(const QString& text)
{
  const auto normalized = text.simplified();

  if (normalized.size() > MaxNoteLength)
  {
    throw std::invalid_argument(QString("The note has %1 characters, but at most %2 fit. Shorten it.")
      .arg(normalized.size())
      .arg(MaxNoteLength)
      .toStdString());
  }

  return normalized;
}

std::vector<QmitkAgentMemory::Note>::iterator QmitkAgentMemory::Find(int id)
{
  const auto note = std::find_if(m_Notes.begin(), m_Notes.end(), [id](const Note& note) { return note.Id == id; });

  if (note == m_Notes.end())
    throw std::invalid_argument(QString("There is no note %1.").arg(id).toStdString());

  return note;
}

int QmitkAgentMemory::Add(const QString& text)
{
  const auto normalized = Normalize(text);

  if (normalized.isEmpty())
    throw std::invalid_argument("The note is empty.");

  if (m_Notes.size() >= MaxNoteCount)
  {
    throw std::invalid_argument(QString("The memory is full (%1 notes). Forget notes that are no longer useful first.")
      .arg(MaxNoteCount)
      .toStdString());
  }

  auto notes = m_Notes;
  notes.push_back({ m_NextId, normalized });
  WriteNotes(m_FilePath, notes, m_NextId + 1);

  m_Notes = std::move(notes);
  return m_NextId++;
}

void QmitkAgentMemory::SetText(int id, const QString& text)
{
  const auto normalized = Normalize(text);

  if (normalized.isEmpty())
    throw std::invalid_argument("The note is empty.");

  const auto index = std::distance(m_Notes.begin(), this->Find(id));

  auto notes = m_Notes;
  notes[index].Text = normalized;
  WriteNotes(m_FilePath, notes, m_NextId);

  m_Notes = std::move(notes);
}

void QmitkAgentMemory::Remove(int id)
{
  const auto index = std::distance(m_Notes.begin(), this->Find(id));

  auto notes = m_Notes;
  notes.erase(notes.begin() + index);
  WriteNotes(m_FilePath, notes, m_NextId);

  m_Notes = std::move(notes);
}

void QmitkAgentMemory::Clear()
{
  WriteNotes(m_FilePath, {}, m_NextId);
  m_Notes.clear();
}

QString QmitkAgentMemory::Describe() const
{
  QString text = "\n## Your notes\n\n";

  if (m_Notes.empty())
    return text + "You have not saved any notes yet.\n";

  text += "What you saved with `remember` in earlier conversations, with their ids:\n\n";

  for (const auto& note : m_Notes)
    text += QString("- [%1] %2\n").arg(note.Id).arg(note.Text);

  return text;
}
