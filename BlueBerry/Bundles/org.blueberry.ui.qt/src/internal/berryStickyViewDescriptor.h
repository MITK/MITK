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
  int GetLocation() const;

  /*
   * @see IStickyViewDescriptor#GetId()
   */
  QString GetId() const;

  /*
   * @see IStickyViewDescriptor#IsFixed()
   */
  bool IsCloseable() const;

  /*
   * @see IStickyViewDescriptor#IsMoveable()
   */
  bool IsMoveable() const;
};

}

#endif /* BERRYSTICKYVIEWDESCRIPTOR_H_ */
