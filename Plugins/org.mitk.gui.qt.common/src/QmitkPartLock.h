/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPartLock_h
#define QmitkPartLock_h

#include <org_mitk_gui_qt_common_Export.h>

#include <QObject>
#include <QString>

/**
 * \ingroup org_mitk_gui_qt_common
 *
 * \brief Locks the views and editors against the user's input while a
 *        program operates the Workbench.
 *
 * Views derived from QmitkAbstractView and editors derived from
 * QmitkAbstractRenderEditor follow the lock, except the one view the holder
 * exempts. A locked part keeps showing what happens, but its content is
 * disabled, which keeps out the mouse, the keyboard and shortcuts, and its
 * tab shows a lock instead of its icon. Menus and tool bars stay usable.
 *
 * There is one holder at a time. Use it in the GUI thread only.
 */
class MITK_QT_COMMON QmitkPartLock : public QObject
{
  Q_OBJECT

public:
  static QmitkPartLock* GetInstance();

  /**
   * \brief Locks all views and editors except one view.
   *
   * Calling it again while locked changes the exempt view or the reason.
   *
   * \param exemptPartId The id of the view that stays usable, usually the
   *        holder's own, where the user can end the lock.
   * \param reason Why the parts are locked and how to end it, shown as the
   *        tool tip of their tabs.
   */
  void Lock(const QString& exemptPartId, const QString& reason);

  void Unlock();

  bool IsLocked() const;

  /** \brief Whether the part with the id \p partId is locked. */
  bool IsLocked(const QString& partId) const;

  QString GetReason() const;

Q_SIGNALS:
  /** \brief The parts were locked or unlocked, or the exempt view or the reason changed. */
  void Changed();

private:
  QmitkPartLock();
  ~QmitkPartLock() override;

  bool m_Locked;
  QString m_ExemptPartId;
  QString m_Reason;
};

#endif
