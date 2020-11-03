/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSTICKYVIEWDESCRIPTOR_H_
#define BERRYSTICKYVIEWDESCRIPTOR_H_

#include "berryIStickyViewDescriptor.h"

#include <berryIConfigurationElement.h>

namespace berry
{

class StickyViewDescriptor: public IStickyViewDescriptor
{

private:

  IConfigurationElement::Pointer configurationElement;

  QString id;

public:

  berryObjectMacro(StickyViewDescriptor);

  /**
   * Folder constant for right sticky views.
   */
  static const QString STICKY_FOLDER_RIGHT; // = "stickyFolderRight";

  /**
   * Folder constant for left sticky views.
   */
  static const QString STICKY_FOLDER_LEFT; // = "stickyFolderLeft";

  /**
   * Folder constant for top sticky views.
   */
  static const QString STICKY_FOLDER_TOP; // = "stickyFolderTop";

  /**
   * Folder constant for bottom sticky views.
   */
  static const QString STICKY_FOLDER_BOTTOM; // = "stickyFolderBottom";

  /**
   * @param element
   * @throws CoreException
   */
  StickyViewDescriptor(IConfigurationElement::Pointer element);

  /**
   * Return the configuration element.
   *
   * @return the configuration element
   */
  IConfigurationElement::Pointer GetConfigurationElement() const;

  /*
   * @see IStickyViewDescriptor#GetLocation()
   */
  int GetLocation() const override;

  /*
   * @see IStickyViewDescriptor#GetId()
   */
  QString GetId() const override;

  /*
   * @see IStickyViewDescriptor#IsFixed()
   */
  bool IsCloseable() const override;

  /*
   * @see IStickyViewDescriptor#IsMoveable()
   */
  bool IsMoveable() const override;
};

}

#endif /* BERRYSTICKYVIEWDESCRIPTOR_H_ */
