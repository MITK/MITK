/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYDEFAULTSAVEABLE_H_
#define BERRYDEFAULTSAVEABLE_H_

#include "berrySaveable.h"

namespace berry {

/**
 * A default {@link Saveable} implementation that wrappers a regular
 * workbench part (one that does not itself adapt to Saveable).
 *
 * @since 3.2
 */
class DefaultSaveable : public Saveable {

private:

  WeakPointer<IWorkbenchPart> part;


public:

  /**
   * Creates a new DefaultSaveable.
   *
   * @param part
   *            the part represented by this model
   */
  DefaultSaveable(SmartPointer<IWorkbenchPart> part);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#doSave(org.blueberry.core.runtime.IProgressMonitor)
   */
  void DoSave(/*IProgressMonitor monitor*/) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#getName()
   */
  QString GetName() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#getImageDescriptor()
   */
  QIcon GetImageDescriptor() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#getToolTipText()
   */
  QString GetToolTipText() const override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#isDirty()
   */
  bool IsDirty() const override;

  /* (non-Javadoc)
   * @see Object#HashCode()
   */
  uint HashCode() const override;

  /* (non-Javadoc)
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool operator<(const Object* obj) const override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.Saveable#show(org.blueberry.ui.IWorkbenchPage)
   */
  bool Show(SmartPointer<IWorkbenchPage> page) override;

};

}

#endif /* BERRYDEFAULTSAVEABLE_H_ */
