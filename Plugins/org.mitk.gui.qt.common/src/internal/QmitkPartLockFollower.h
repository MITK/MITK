/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPartLockFollower_h
#define QmitkPartLockFollower_h

#include <berryIViewReference.h>

#include <QIcon>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QWidget>

#include <functional>
#include <utility>
#include <vector>

namespace berry
{
  struct IWorkbenchPart;
}

/**
 * \brief Makes a part follow QmitkPartLock.
 *
 * While the part is locked, its content is disabled, and its tab shows a lock
 * in the theme's error color instead of its icon, with the reason of the lock
 * as tool tip.
 */
class QmitkPartLockFollower : public QObject
{
  Q_OBJECT

public:
  /** \brief Sets the title image and tool tip of a part, which only the part itself can. */
  using SetTitleFunction = std::function<void(const QIcon& image, const QString& toolTip)>;

  /**
   * \param part The part, for its id and its title. Must outlive the follower.
   * \param content What the user must not use while the part is locked.
   */
  QmitkPartLockFollower(const berry::IWorkbenchPart* part, QWidget* content, SetTitleFunction setTitle);

  ~QmitkPartLockFollower() override;

  bool eventFilter(QObject* watched, QEvent* event) override;

private:
  void Update();

  const berry::IWorkbenchPart* m_Part;
  QString m_PartId;
  QPointer<QWidget> m_Content;
  SetTitleFunction m_SetTitle;

  bool m_Locked;

  /** \brief The title of the part before it was locked. */
  QIcon m_TitleImage;
  QString m_TitleToolTip;
};

/**
 * \brief Makes the tabs of views that do not exist yet follow QmitkPartLock.
 *
 * BlueBerry creates a view, and with it its QmitkPartLockFollower, when the
 * view is shown for the first time. Until then, its tab shows the icon of its
 * descriptor, which this replaces with the lock while the view is locked.
 *
 * The tabs are updated when the lock or the theme changes, and only for the
 * views of the active perspective. A view whose tab appears later, for example
 * after a perspective switch during the lock, shows its icon until it is
 * created. Views that are not QmitkAbstractView cannot be locked: their tab
 * shows the lock until the view is created, and the view itself is not locked.
 */
class QmitkUncreatedViewLockFollower : public QObject
{
  Q_OBJECT

public:
  QmitkUncreatedViewLockFollower();
  ~QmitkUncreatedViewLockFollower() override;

private:
  void Update();
  void Restore();

  /** \brief The views whose tabs show the lock, with the icons they showed before. */
  std::vector<std::pair<berry::IViewReference::Pointer, QIcon>> m_Icons;
};

#endif
