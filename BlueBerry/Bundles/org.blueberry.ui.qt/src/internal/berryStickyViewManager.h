/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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
  void Add(const std::string& perspectiveId, const std::set<std::string>& stickyViewSet);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#clear()
   */
  void Clear();

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#remove(java.lang.String)
   */
  void Remove(const std::string& perspectiveId);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#restore(org.eclipse.ui.IMemento)
   */
  void Restore(IMemento::Pointer memento);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#save(org.eclipse.ui.IMemento)
   */
  void Save(IMemento::Pointer memento);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.internal.IStickyViewManager#update(org.eclipse.ui.internal.Perspective,
   *      org.eclipse.ui.internal.Perspective)
   */
  void Update(SmartPointer<Perspective> oldPersp, SmartPointer<Perspective> newPersp);

};

}

#endif /* BERRYSTICKYVIEWMANAGER_H_ */
