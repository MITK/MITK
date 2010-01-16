/*=========================================================================

 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/


#ifndef BERRYDEFAULTSAVEABLE_H_
#define BERRYDEFAULTSAVEABLE_H_

#include "../berrySaveable.h"

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
  void DoSave(/*IProgressMonitor monitor*/);

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#getName()
   */
  std::string GetName() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#getImageDescriptor()
   */
  SmartPointer<ImageDescriptor> GetImageDescriptor() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#getToolTipText()
   */
  std::string GetToolTipText() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.Saveable#isDirty()
   */
  bool IsDirty() const;

  /* (non-Javadoc)
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool operator<(const Saveable* obj) const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.Saveable#show(org.blueberry.ui.IWorkbenchPage)
   */
  bool Show(SmartPointer<IWorkbenchPage> page);

};

}

#endif /* BERRYDEFAULTSAVEABLE_H_ */
