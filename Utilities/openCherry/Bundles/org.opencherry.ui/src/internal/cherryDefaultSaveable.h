/*=========================================================================

 Program:   openCherry Platform
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


#ifndef CHERRYDEFAULTSAVEABLE_H_
#define CHERRYDEFAULTSAVEABLE_H_

#include "../cherrySaveable.h"

namespace cherry {

/**
 * A default {@link Saveable} implementation that wrappers a regular
 * workbench part (one that does not itself adapt to Saveable).
 *
 * @since 3.2
 */
class DefaultSaveable : public Saveable {

private:

  SmartPointer<IWorkbenchPart> part;


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
   * @see org.opencherry.ui.Saveable#doSave(org.opencherry.core.runtime.IProgressMonitor)
   */
  void DoSave(/*IProgressMonitor monitor*/);

  /*
   * (non-Javadoc)
   *
   * @see org.opencherry.ui.Saveable#getName()
   */
  std::string GetName();

  /*
   * (non-Javadoc)
   *
   * @see org.opencherry.ui.Saveable#getImageDescriptor()
   */
  void* GetImageDescriptor();

  /*
   * (non-Javadoc)
   *
   * @see org.opencherry.ui.Saveable#getToolTipText()
   */
  std::string GetToolTipText();

  /*
   * (non-Javadoc)
   *
   * @see org.opencherry.ui.Saveable#isDirty()
   */
  bool IsDirty();

  /* (non-Javadoc)
   * @see java.lang.Object#equals(java.lang.Object)
   */
  bool operator<(const Saveable* obj) const;

  /* (non-Javadoc)
   * @see org.opencherry.ui.Saveable#show(org.opencherry.ui.IWorkbenchPage)
   */
  bool Show(SmartPointer<IWorkbenchPage> page);

};

}

#endif /* CHERRYDEFAULTSAVEABLE_H_ */
