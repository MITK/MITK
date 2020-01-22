/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYERRORVIEWPART_H_
#define BERRYERRORVIEWPART_H_

#include "tweaklets/berryWorkbenchPageTweaklet.h"

#include "berryViewPart.h"

namespace berry {

/**
 * This part is shown instead the views with errors.
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
  ErrorViewPart(const QString& title, const QString& error);

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.part.WorkbenchPart#createPartControl(org.eclipse.swt.widgets.Composite)
   */
  void CreatePartControl(QWidget* parent) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.part.ViewPart#setPartName(java.lang.String)
   */
  void SetPartName(const QString& newName) override;

  /*
   * (non-Javadoc)
   *
   * @see org.eclipse.ui.part.WorkbenchPart#setFocus()
   */
  void SetFocus() override;

  private:

  //IStatus error;
  QString title;
  QString error;
  Object::Pointer statusPart;
};


}

#endif /* BERRYERRORVIEWPART_H_ */
