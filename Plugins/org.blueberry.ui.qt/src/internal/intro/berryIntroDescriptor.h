/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYINTRODESCRIPTOR_H_
#define BERRYINTRODESCRIPTOR_H_

#include "berryIIntroDescriptor.h"
#include "intro/berryIntroContentDetector.h"
#include "intro/berryIIntroPart.h"

#include <berryIConfigurationElement.h>

#include <QIcon>

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

  mutable QIcon imageDescriptor;

public:

  berryObjectMacro(IntroDescriptor);

  /**
   * Create a new IntroDescriptor for an extension.
   */
  IntroDescriptor(IConfigurationElement::Pointer configElement);

  /*
   * @see IIntroDescriptor#CreateIntro()
   */
  IIntroPart::Pointer CreateIntro() override;

  IntroContentDetector::Pointer GetIntroContentDetector();

  int GetRole() const override;

  /*
   * @see IIntroDescriptor#GetId()
   */
  QString GetId() const override;

  QString GetPluginId() const;

  /*
   * @see IIntroDescriptor#GetImageDescriptor()
   */
  QIcon GetImageDescriptor() const override;

  /**
   * Returns the configuration element.
   *
   * @return the configuration element
   */
  IConfigurationElement::Pointer GetConfigurationElement() const;

  /*
   * @see IIntroDescriptor#GetLabelOverride()
   */
  QString GetLabelOverride() const override;

};

}

#endif /* BERRYINTRODESCRIPTOR_H_ */
