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

#ifndef CHERRYSAVEABLEMOCKVIEWPART_H_
#define CHERRYSAVEABLEMOCKVIEWPART_H_

#include "cherryMockViewPart.h"

#include <cherryISaveablePart.h>

namespace cherry
{

/**
 * Mock view part that implements ISaveablePart.
 * Used for testing hideView and other view lifecycle on saveable views.
 *
 * @since 3.0.1
 */
class SaveableMockViewPart: public MockViewPart, public ISaveablePart
{

public:

  cherryObjectMacro(SaveableMockViewPart)

  static const std::string ID; // = "org.opencherry.ui.tests.api.SaveableMockViewPart";

  SaveableMockViewPart();

  void CreatePartControl(void* parent);

  /* (non-Javadoc)
   * @see org.eclipse.ui.ISaveablePart#doSave(org.eclipse.core.runtime.IProgressMonitor)
   */
  void DoSave(/*IProgressMonitor monitor*/);

  /* (non-Javadoc)
   * @see org.eclipse.ui.ISaveablePart#doSaveAs()
   */
  void DoSaveAs();

  /* (non-Javadoc)
   * @see org.eclipse.ui.ISaveablePart#isDirty()
   */
  bool IsDirty() const;

  /* (non-Javadoc)
   * @see org.eclipse.ui.ISaveablePart#isSaveAsAllowed()
   */
  bool IsSaveAsAllowed() const;

  /* (non-Javadoc)
   * @see org.eclipse.ui.ISaveablePart#isSaveOnCloseNeeded()
   */
  bool IsSaveOnCloseNeeded() const;

  void SetDirty(bool isDirty);

  void SetSaveAsAllowed(bool isSaveAsAllowed);

  void SetSaveNeeded(bool isSaveOnCloseNeeded);

private:

  bool isDirty;

  bool saveAsAllowed;

  bool saveNeeded;

};

}

#endif /* CHERRYSAVEABLEMOCKVIEWPART_H_ */
