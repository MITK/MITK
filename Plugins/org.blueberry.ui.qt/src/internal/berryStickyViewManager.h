/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSTICKYVIEWMANAGER_H_
#define BERRYSTICKYVIEWMANAGER_H_

#include "berryIStickyViewManager.h"

namespace berry {

struct IWorkbenchPage;

class StickyViewManager : public IStickyViewManager {

private:

  IWorkbenchPage* page;

public:

  StickyViewManager(IWorkbenchPage* page);

  static IStickyViewManager::Pointer GetInstance(IWorkbenchPage* page);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#add(java.lang.String,
   *      java.util.Set)
   */
  void Add(const QString& perspectiveId, const std::set<QString>& stickyViewSet) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#clear()
   */
  void Clear() override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#remove(java.lang.String)
   */
  void Remove(const QString& perspectiveId) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#restore(org.eclipse.ui.IMemento)
   */
  void Restore(IMemento::Pointer memento) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#save(org.eclipse.ui.IMemento)
   */
  void Save(IMemento::Pointer memento) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#update(org.eclipse.ui.internal.Perspective,
   *      org.eclipse.ui.internal.Perspective)
   */
  void Update(SmartPointer<Perspective> oldPersp, SmartPointer<Perspective> newPersp) override;

};

}

#endif /* BERRYSTICKYVIEWMANAGER_H_ */
