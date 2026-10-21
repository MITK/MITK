/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentMemory_h
#define QmitkAgentMemory_h

#include <MitkAgentExports.h>

#include <QString>

#include <cstddef>
#include <vector>

/**
 * \brief Notes that the agent keeps across conversations.
 *
 * The notes are stored in a JSON file and go into the agent's instructions
 * at the start of each conversation, so their number and length are
 * limited. Ids are never reused, so that an id from an earlier list cannot
 * hit a different note.
 *
 * Not thread-safe; use it from the GUI thread only.
 */
class MITKAGENT_EXPORT QmitkAgentMemory
{
public:
  struct Note
  {
    int Id;
    QString Text;
  };

  static constexpr std::size_t MaxNoteCount = 50;
  static constexpr qsizetype MaxNoteLength = 500;

  /** \brief Loads the notes from the file, if it exists. */
  explicit QmitkAgentMemory(const QString& filePath);

  const std::vector<Note>& GetNotes() const;

  /**
   * \brief Adds a note and returns its id.
   *
   * \throw std::invalid_argument if the text is empty or too long, or the memory is full.
   * \throw std::runtime_error if the file cannot be written.
   */
  int Add(const QString& text);

  /**
   * \brief Replaces the text of a note.
   *
   * \throw std::invalid_argument if there is no such note or the text is empty or too long.
   * \throw std::runtime_error if the file cannot be written.
   */
  void SetText(int id, const QString& text);

  /**
   * \brief Removes a note.
   *
   * \throw std::invalid_argument if there is no such note.
   * \throw std::runtime_error if the file cannot be written.
   */
  void Remove(int id);

  /** \brief \throw std::runtime_error if the file cannot be written. */
  void Clear();

  /** \brief The notes as a section for the agent's instructions. */
  QString Describe() const;

private:
  /** \brief \throw std::invalid_argument if the text is too long. */
  static QString Normalize(const QString& text);

  /** \brief \throw std::invalid_argument if there is no such note. */
  std::vector<Note>::iterator Find(int id);

  QString m_FilePath;
  std::vector<Note> m_Notes;
  int m_NextId;
};

#endif
