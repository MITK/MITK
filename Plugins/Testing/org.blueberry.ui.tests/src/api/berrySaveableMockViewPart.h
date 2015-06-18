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

#ifndef BERRYSAVEABLEMOCKVIEWPART_H_
#define BERRYSAVEABLEMOCKVIEWPART_H_

#include "berryMockViewPart.h"

#include <berryISaveablePart.h>

namespace berry
{

/**
 * Mock view part that implements ISaveablePart.
 * Used for testing hideView and other view lifecycle on saveable views.
 *
 * @since 3.0.1
 */
class SaveableMockViewPart: public MockViewPart, public ISaveablePart
{
  Q_OBJECT
  Q_INTERFACES(berry::ISaveablePart)

public:

  berryObjectMacro(SaveableMockViewPart);

  static const std::string ID; // = "org.blueberry.ui.tests.api.SaveableMockViewPart";

  SaveableMockViewPart();
  SaveableMockViewPart(const SaveableMockViewPart& other);

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

#endif /* BERRYSAVEABLEMOCKVIEWPART_H_ */
