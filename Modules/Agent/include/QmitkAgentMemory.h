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
 * A file that exists but cannot be read is never written over: while it stays
 * unreadable, every change fails. Every call tries to read it again first.
 * A change reads the file in any case, so that the notes of another instance
 * of the Workbench on the same file are kept, and takes the larger of the
 * next ids of the file and of the memory.
 * Notes of a file that exceed the limits stay in it and in GetNotes(), so that
 * the user can edit them, but are left out of Describe().
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

  /**
   * \brief Whether the file could be read, or does not exist.
   *
   * If it could not be read, reads it again. While this is false, the memory
   * has no notes and refuses to change.
   */
  bool IsReadable() const;

  /** \brief The file the notes are kept in. */
  const QString& GetFilePath() const;

  /** \brief All notes of the file, also those that Describe() leaves out. */
  const std::vector<Note>& GetNotes() const;

  /**
   * \brief Adds a note and returns its id.
   *
   * \throw std::invalid_argument if the text is empty or too long, or the memory is full.
   * \throw std::runtime_error if the file cannot be read or written.
   */
  int Add(const QString& text);

  /**
   * \brief Replaces the text of a note.
   *
   * \throw std::invalid_argument if there is no such note or the text is empty or too long.
   * \throw std::runtime_error if the file cannot be read or written.
   */
  void SetText(int id, const QString& text);

  /**
   * \brief Removes a note.
   *
   * \throw std::invalid_argument if there is no such note.
   * \throw std::runtime_error if the file cannot be read or written.
   */
  void Remove(int id);

  /** \brief \throw std::runtime_error if the file cannot be read or written. */
  void Clear();

  /**
   * \brief The notes as a section for the agent's instructions.
   *
   * Leaves out notes that are longer than MaxNoteLength, and notes beyond
   * the first MaxNoteCount of the others.
   */
  QString Describe() const;

private:
  /** \brief Reads the file; sets m_Readable. */
  void Load() const;

  /** \brief \throw std::runtime_error unless IsReadable(). */
  void RequireReadable() const;

  /** \brief \throw std::invalid_argument if the text is too long. */
  static QString Normalize(const QString& text);

  /** \brief \throw std::invalid_argument if there is no such note. */
  std::vector<Note>::iterator Find(int id);

  QString m_FilePath;

  // Mutable, as the const readers try to read a file again that could not be read.
  mutable std::vector<Note> m_Notes;
  mutable int m_NextId;
  mutable bool m_Readable;
};

#endif
