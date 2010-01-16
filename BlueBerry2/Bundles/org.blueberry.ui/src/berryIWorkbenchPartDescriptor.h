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

#ifndef BERRYIWORKBENCHPARTDESCRIPTOR_H_
#define BERRYIWORKBENCHPARTDESCRIPTOR_H_

#include "berryUiDll.h"

#include <osgi/framework/Macros.h>
#include <osgi/framework/Object.h>

#include "berryImageDescriptor.h"

#include <string>

namespace berry
{

using namespace osgi::framework;

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

  osgiInterfaceMacro(berry::IWorkbenchPartDescriptor);

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

  virtual ~IWorkbenchPartDescriptor()
  {
  }
};

} // namespace berry

#endif /*BERRYIWORKBENCHPARTDESCRIPTOR_H_*/
