/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkLineCompletionTextEdit_h
#define QmitkLineCompletionTextEdit_h

#include <MitkQtWidgetsExports.h>

#include <QPlainTextEdit>
#include <QStringList>

class QCompleter;
class QStringListModel;

/**
 * \ingroup QmitkModule
 * \brief A plain text edit for one entry per line that suggests entries while
 *        the user types.
 *
 * While the user types at the end of a line of at least two characters, a
 * popup lists the completions that contain the line, those that start with it
 * first, ignoring case. Choosing one with Enter, Tab, or a click replaces the
 * line with it. Tab moves the focus on while no suggestions are shown, as a
 * tab character is no entry.
 */
class MITKQTWIDGETS_EXPORT QmitkLineCompletionTextEdit : public QPlainTextEdit
{
  Q_OBJECT

public:
  explicit QmitkLineCompletionTextEdit(QWidget* parent = nullptr);
  ~QmitkLineCompletionTextEdit() override;

  /** \brief Sets the entries to suggest, in the order they are listed in. */
  void SetCompletions(const QStringList& completions);

protected:
  void keyPressEvent(QKeyEvent* event) override;

private:
  /** \brief Lists the completions of the line at the cursor, or hides the list. */
  void UpdateCompletionPopup();

  /** \brief Returns the completions of a line, those that start with it first. */
  QStringList FindCompletions(const QString& line) const;

  /** \brief Replaces the line at the cursor by the given completion. */
  void InsertCompletion(const QString& completion);

  QCompleter* m_Completer;
  QStringListModel* m_Model;
  QStringList m_Completions;

  /** The completions in lower case, so that matching does not convert them on every key press. */
  QStringList m_LowerCaseCompletions;
};

#endif
