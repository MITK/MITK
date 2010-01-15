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

#ifndef CHERRYINTRODESCRIPTOR_H_
#define CHERRYINTRODESCRIPTOR_H_

#include "cherryIIntroDescriptor.h"
#include "../../intro/cherryIntroContentDetector.h"
#include "../../intro/cherryIIntroPart.h"

#include <cherryIConfigurationElement.h>
#include <cherryImageDescriptor.h>

namespace cherry
{

/**
 * Describes an introduction extension.
 *
 */
class IntroDescriptor: public IIntroDescriptor
{

private:

  IConfigurationElement::Pointer element;

  mutable ImageDescriptor::Pointer imageDescriptor;

public:

  cherryObjectMacro(IntroDescriptor)

  /**
   * Create a new IntroDescriptor for an extension.
   */
  IntroDescriptor(IConfigurationElement::Pointer configElement)
      throw (CoreException);

  /*
   * @see IIntroDescriptor#CreateIntro()
   */
  IIntroPart::Pointer CreateIntro() throw (CoreException);

  IntroContentDetector::Pointer GetIntroContentDetector() throw (CoreException);

  int GetRole() const;

  /*
   * @see IIntroDescriptor#GetId()
   */
  std::string GetId() const;

  std::string GetPluginId() const;

  /*
   * @see IIntroDescriptor#GetImageDescriptor()
   */
  ImageDescriptor::Pointer GetImageDescriptor() const;

  /**
   * Returns the configuration element.
   *
   * @return the configuration element
   */
  IConfigurationElement::Pointer GetConfigurationElement() const;

  /*
   * @see IIntroDescriptor#GetLabelOverride()
   */
  std::string GetLabelOverride() const;

};

}

#endif /* CHERRYINTRODESCRIPTOR_H_ */
