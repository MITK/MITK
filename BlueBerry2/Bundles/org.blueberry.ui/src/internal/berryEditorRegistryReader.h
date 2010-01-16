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

#ifndef BERRYEDITORREGISTRYREADER_H_
#define BERRYEDITORREGISTRYREADER_H_

#include "berryRegistryReader.h"

namespace berry
{

class EditorRegistry;

/**
 * \ingroup org_blueberry_ui_internal
 * 
 * This class is used to read resource editor registry descriptors from
 * the platform registry.
 */
class EditorRegistryReader : public RegistryReader
{

private:

  EditorRegistry* editorRegistry;

protected:

  /**
   * Implementation of the abstract method that
   * processes one configuration element.
   */
  bool ReadElement(IConfigurationElement::Pointer element);

  
public:
  
  /**
   * Get the editors that are defined in the registry
   * and add them to the ResourceEditorRegistry
   *
   * Warning:
   * The registry must be passed in because this method is called during the
   * process of setting up the registry and at this time it has not been
   * safely setup with the plugin.
   */
  void AddEditors(EditorRegistry* registry);

  /**
   * @param editorRegistry
   * @param element
   */
  void ReadElement(EditorRegistry* editorRegistry,
      IConfigurationElement::Pointer element);
};

}

#endif /*BERRYEDITORREGISTRYREADER_H_*/
