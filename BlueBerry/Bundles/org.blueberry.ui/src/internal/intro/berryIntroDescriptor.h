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

#ifndef BERRYINTRODESCRIPTOR_H_
#define BERRYINTRODESCRIPTOR_H_

#include "berryIIntroDescriptor.h"
#include "intro/berryIntroContentDetector.h"
#include "intro/berryIIntroPart.h"

#include <berryIConfigurationElement.h>
#include <berryImageDescriptor.h>

namespace berry
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

  berryObjectMacro(IntroDescriptor)

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

#endif /* BERRYINTRODESCRIPTOR_H_ */
