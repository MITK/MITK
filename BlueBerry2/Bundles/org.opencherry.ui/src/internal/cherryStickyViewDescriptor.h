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

#ifndef CHERRYSTICKYVIEWDESCRIPTOR_H_
#define CHERRYSTICKYVIEWDESCRIPTOR_H_

#include "../cherryIStickyViewDescriptor.h"

#include <cherryIConfigurationElement.h>

namespace cherry
{

class StickyViewDescriptor: public IStickyViewDescriptor
{

private:

  IConfigurationElement::Pointer configurationElement;

  std::string id;

public:

  cherryObjectMacro(StickyViewDescriptor)

  /**
   * Folder constant for right sticky views.
   */
  static const std::string STICKY_FOLDER_RIGHT; // = "stickyFolderRight"; //$NON-NLS-1$

  /**
   * Folder constant for left sticky views.
   */
  static const std::string STICKY_FOLDER_LEFT; // = "stickyFolderLeft"; //$NON-NLS-1$

  /**
   * Folder constant for top sticky views.
   */
  static const std::string STICKY_FOLDER_TOP; // = "stickyFolderTop"; //$NON-NLS-1$

  /**
   * Folder constant for bottom sticky views.
   */
  static const std::string STICKY_FOLDER_BOTTOM; // = "stickyFolderBottom"; //$NON-NLS-1$

  /**
   * @param element
   * @throws CoreException
   */
  StickyViewDescriptor(IConfigurationElement::Pointer element)
      throw (CoreException);

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
  std::string GetId() const;

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

#endif /* CHERRYSTICKYVIEWDESCRIPTOR_H_ */
