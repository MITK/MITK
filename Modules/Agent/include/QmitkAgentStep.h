/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentStep_h
#define QmitkAgentStep_h

#include <QByteArray>
#include <QList>
#include <QString>

/** \brief A step of the agent: a tool call with its result, as the conversation shows it. */
struct QmitkAgentStep
{
  /** \brief A change of the agent's memory, which the summary of steps mentions. */
  enum class MemoryChange
  {
    None,
    NoteSaved,
    NoteDeleted
  };

  /** \brief The call in one line; empty if it is not known. */
  QString Call;

  /** \brief The result, where it is worth showing, for example why the call failed; may be empty. */
  QString Result;

  bool Failed = false;

  /** \brief The images of the result, encoded, for example as PNG. */
  QList<QByteArray> Images;

  MemoryChange Memory = MemoryChange::None;
};

#endif
