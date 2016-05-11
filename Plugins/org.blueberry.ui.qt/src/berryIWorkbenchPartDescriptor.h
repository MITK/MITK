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

  berryObjectMacro(berry::IWorkbenchPartDescriptor)

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
  * Returns the case perspecitve or 0.
  *
  * @return the part case
  */
  virtual QString GetCase() const = 0;

  /**
   * Returns the label to show for this part.
   *
   * @return the part label
   */
  virtual QString GetLabel() const = 0;

  virtual ~IWorkbenchPartDescriptor();
};

} // namespace berry

#endif /*BERRYIWORKBENCHPARTDESCRIPTOR_H_*/
