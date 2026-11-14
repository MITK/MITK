/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPartLock_h
#define QmitkPartLock_h

#include <MitkQtWidgetsExports.h>

#include <QObject>
#include <QString>

/**
 * \ingroup QmitkModule
 *
 * \brief Locks the views and editors against the user's input while a
 *        program operates the Workbench.
 *
 * Holds only the state, which the Workbench plugins follow: views derived
 * from QmitkAbstractView and editors derived from QmitkAbstractRenderEditor
 * (org.mitk.gui.qt.common), except the one view the holder exempts, and the
 * actions of the Workbench window (org.mitk.gui.qt.ext). A locked part keeps showing what happens, but its content is
 * disabled, which keeps out the mouse, the keyboard and shortcuts, and its
 * tab shows a lock instead of its icon. Menus and tool bars stay usable,
 * except Undo, Redo and Close Project of the Workbench window, and so do file
 * drops onto the editor area and parts that do not follow the lock, such as
 * the Welcome page; the pixel routes of the REST API detect a view that
 * changed in between.
 *
 * There is one holder at a time. Use it in the GUI thread only.
 */
class MITKQTWIDGETS_EXPORT QmitkPartLock : public QObject
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
