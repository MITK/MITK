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


#ifndef CHERRYIINTRODESCRIPTOR_H_
#define CHERRYIINTRODESCRIPTOR_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "../../intro/cherryIIntroPart.h"

#include <cherryPlatformException.h>
#include <cherryImageDescriptor.h>

namespace cherry {

/**
 * Describes an introduction extension.
 *
 * @since 3.0
 */
struct IIntroDescriptor : public Object {

  cherryInterfaceMacro(IIntroDescriptor, cherry)

  /**
   * Creates an instance of the intro part defined in the descriptor.
   */
  virtual IIntroPart::Pointer CreateIntro() throw(CoreException) = 0;

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

#endif /* CHERRYIINTRODESCRIPTOR_H_ */
