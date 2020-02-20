/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYIWORKBENCHPARTDESCRIPTOR_H_
#define BERRYIWORKBENCHPARTDESCRIPTOR_H_

#include <org_blueberry_ui_qt_Export.h>

#include <berryMacros.h>
#include <berryObject.h>

namespace berry
{

/**
 * \ingroup org_blueberry_ui_qt
 *
 * Description of a workbench part. The part descriptor contains
 * the information needed to create part instances.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct BERRY_UI_QT IWorkbenchPartDescriptor : public virtual Object
{

  berryObjectMacro(berry::IWorkbenchPartDescriptor);

  /**
   * Returns the part id.
   *
   * @return the id of the part
   */
  virtual QString GetId() const = 0;

  /**
   * Returns the descriptor of the image for this part.
   *
   * @return the descriptor of the image to display next to this part
   */
  virtual QIcon GetImageDescriptor() const = 0;

  /**
   * Returns the label to show for this part.
   *
   * @return the part label
   */
  virtual QString GetLabel() const = 0;

  ~IWorkbenchPartDescriptor() override;
};

} // namespace berry

#endif /*BERRYIWORKBENCHPARTDESCRIPTOR_H_*/
