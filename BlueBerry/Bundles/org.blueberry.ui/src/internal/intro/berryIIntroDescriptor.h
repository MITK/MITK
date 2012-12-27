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


#ifndef BERRYIINTRODESCRIPTOR_H_
#define BERRYIINTRODESCRIPTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include "intro/berryIIntroPart.h"

#include <berryPlatformException.h>
#include <berryImageDescriptor.h>

namespace berry {

/**
 * Describes an introduction extension.
 *
 * @since 3.0
 */
struct BERRY_UI IIntroDescriptor : public Object {

  berryInterfaceMacro(IIntroDescriptor, berry)

  ~IIntroDescriptor();

  /**
   * Creates an instance of the intro part defined in the descriptor.
   */
  virtual IIntroPart::Pointer CreateIntro() throw(CoreException) = 0;

  /**
   * Returns the role of the intro part (view or editor)
   * @return the role of the part
   */
  virtual int GetRole() const = 0;

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
  virtual ImageDescriptor::Pointer GetImageDescriptor() const = 0;

  /**
   * Return the label override string for this part.
   *
   * @return the label override string or the empty string if one has not
   *         been specified
   */
  virtual std::string GetLabelOverride() const = 0;
};

}

#endif /* BERRYIINTRODESCRIPTOR_H_ */
