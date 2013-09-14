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


#ifndef BERRYERRORVIEWPART_H_
#define BERRYERRORVIEWPART_H_

#include "tweaklets/berryWorkbenchPageTweaklet.h"

#include "berryViewPart.h"

namespace berry {

/**
 * This part is shown instead the views with errors.
 *
 * @since 3.3
 */
class ErrorViewPart : public ViewPart {

public:

  berryObjectMacro(ErrorViewPart);

  /**
   * Creates instance of the class
   */
  ErrorViewPart();

  /**
   * Creates instance of the class
   *
   * @param error the status
   */
  ErrorViewPart(const std::string& title, const std::string& error);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.part.WorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
   */
  void CreatePartControl(void* parent);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.part.ViewPart#setPartName(java.lang.String)
   */
  void SetPartName(const std::string& newName);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.part.WorkbenchPart#setFocus()
   */
  void SetFocus();

  private:

  //IStatus error;
  std::string title;
  std::string error;
  Object::Pointer statusPart;
};


}

#endif /* BERRYERRORVIEWPART_H_ */
