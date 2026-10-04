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
 * While the user types at the end of a line, a popup lists the completions
 * that contain the line, ignoring case: one that equals it first, then those
 * that start with it. A single character only lists those that start with it.
 * Choosing one with Enter, Tab, or a click replaces the line with it and
 * starts the next line. Tab moves the focus on while no suggestions are shown,
 * as a tab character is no entry. The suggestions are in the font size of the
 * text.
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
  /** \brief Keeps Tab for the suggestions while they are shown.
   *
   * QWidget::event() moves the focus on Tab before keyPressEvent() gets to see
   * the key, so a Tab that is meant for the list has to be caught here.
   */
  bool event(QEvent* event) override;

  void keyPressEvent(QKeyEvent* event) override;

private:
  /** \brief Lists the completions of the line at the cursor, or hides the list. */
  void UpdateCompletionPopup();

  /** \brief Returns the completions of a line in the order they are listed in. */
  QStringList FindCompletions(const QString& line) const;

  /** \brief Replaces the line at the cursor by the given completion and starts the next line. */
  void InsertCompletion(const QString& completion);

  QCompleter* m_Completer;
  QStringListModel* m_Model;
  QStringList m_Completions;

  /** The completions in lower case, so that matching does not convert them on every key press. */
  QStringList m_LowerCaseCompletions;
};

#endif
