/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkHotkeyLineEdit_h
#define QmitkHotkeyLineEdit_h

#include <MitkQtWidgetsExtExports.h>

// qt
#include <QKeySequence>
#include <QLineEdit>

/**
 * \brief A QLineEdit that captures keyboard shortcuts (hotkeys).
 *
 * This read-only line edit captures key press events and displays the
 * resulting key sequence as text. It is used for configuring keyboard
 * shortcuts in the application. Pressing Escape clears the current
 * sequence.
 *
 * \note The line edit is read-only by default; key sequences are set
 * via keyPressEvent rather than text input.
 */
class MITKQTWIDGETSEXT_EXPORT QmitkHotkeyLineEdit : public QLineEdit
{
  Q_OBJECT

public:
  /** \brief Default tooltip text displayed on the widget. */
  static const std::string TOOLTIP;

  /**
   * \brief Construct a hotkey line edit with no initial key sequence.
   * \param[in] parent The parent widget.
   */
  QmitkHotkeyLineEdit(QWidget* parent = nullptr);

  /**
   * \brief Construct a hotkey line edit with an initial key sequence.
   * \param[in] qKeySequence The initial key sequence.
   * \param[in] parent The parent widget.
   */
  QmitkHotkeyLineEdit(const QKeySequence& qKeySequence, QWidget* parent = nullptr);

  /**
   * \brief Construct a hotkey line edit with an initial key sequence string.
   * \param[in] qQString The initial key sequence as a string (e.g. "Ctrl+S").
   * \param[in] parent The parent widget.
   */
  QmitkHotkeyLineEdit(const QString& qQString, QWidget* parent = nullptr);

  /**
   * \brief Programmatically set the key sequence.
   * \param[in] qKeySequence The key sequence to set.
   */
  virtual void SetKeySequence(const QKeySequence& qKeySequence);

  /**
   * \brief Programmatically set the key sequence from a string.
   * \param[in] qKeySequenceAsString The key sequence string (e.g. "Ctrl+S").
   */
  virtual void SetKeySequence(const QString& qKeySequenceAsString);

  /**
   * \brief Get the currently configured key sequence.
   * \return The current QKeySequence.
   */
  virtual QKeySequence GetKeySequence();

  /**
   * \brief Get the currently configured key sequence as a string.
   * \return The key sequence string representation.
   */
  virtual QString GetKeySequenceAsString();

  /**
   * \brief Check whether a key event matches the stored key sequence.
   * \param[in] event The key event to compare against.
   * \return True if the event matches the stored key sequence.
   */
  bool Matches(QKeyEvent *event);

protected Q_SLOTS:

  void LineEditTextChanged(const QString&);

protected:

  void keyPressEvent(QKeyEvent* event) override;
  void Init();

  QKeySequence m_KeySequence;

};

#endif
