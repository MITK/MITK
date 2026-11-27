/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAgentDataNoticeDialog_h
#define QmitkAgentDataNoticeDialog_h

#include <QDialog>

/**
 * \brief Tells what leaves the computer when the agent works, and asks the
 *        user to commit to using it only on data that they may send.
 *
 * Asking, the dialog can be accepted only after the user ticked that
 * commitment. Showing, it only has a Close button.
 */
class QmitkAgentDataNoticeDialog : public QDialog
{
  Q_OBJECT

public:
  enum class Mode
  {
    Ask,
    Show
  };

  /**
   * \brief The version of the notice.
   *
   * Raised when the notice changes in substance, so that users who accepted
   * an earlier one are asked again.
   */
  static constexpr int VERSION = 1;

  explicit QmitkAgentDataNoticeDialog(Mode mode, QWidget* parent = nullptr);
};

#endif
