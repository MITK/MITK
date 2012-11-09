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

#include <org_blueberry_ui_Export.h>

#include <berryMacros.h>
#include <berryObject.h>

#include "berryImageDescriptor.h"

#include <string>

namespace berry
{

/**
 * \ingroup org_blueberry_ui
 *
 * Description of a workbench part. The part descriptor contains
 * the information needed to create part instances.
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct BERRY_UI IWorkbenchPartDescriptor : public Object
{

  berryInterfaceMacro(IWorkbenchPartDescriptor, berry);

  /**
   * Returns the part id.
   *
   * @return the id of the part
   */
  virtual std::string GetId() const = 0;

  /**
   * Returns the descriptor of the image for this part.
   *
   * @return the descriptor of the image to display next to this part
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor() const = 0;

  /**
   * Returns the label to show for this part.
   *
   * @return the part label
   */
  virtual std::string GetLabel() const = 0;

  virtual ~IWorkbenchPartDescriptor();
};

} // namespace berry

#endif /*BERRYIWORKBENCHPARTDESCRIPTOR_H_*/
