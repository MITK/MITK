/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  bool ReadElement(const SmartPointer<IConfigurationElement>& element) override;


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
                   const SmartPointer<IConfigurationElement>& element);
};

}

#endif /*BERRYEDITORREGISTRYREADER_H_*/
