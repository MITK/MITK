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

#ifndef BERRYPERSPECTIVEEXTENSIONREADER_H_
#define BERRYPERSPECTIVEEXTENSIONREADER_H_

#include "berryRegistryReader.h"

#include "berryPageLayout.h"

namespace berry
{

/**
 * A strategy to read perspective extension from the registry.
 * A pespective extension is one of a view, viewAction, perspAction,
 * newWizardAction, or actionSet.
 */
class PerspectiveExtensionReader: public RegistryReader
{
private:

  std::string targetID;

  PageLayout::Pointer pageLayout;

  std::vector<std::string> includeOnlyTags;

  static const std::string VAL_LEFT; // = "left";//$NON-NLS-1$

  static const std::string VAL_RIGHT; // = "right";//$NON-NLS-1$

  static const std::string VAL_TOP; // = "top";//$NON-NLS-1$

  static const std::string VAL_BOTTOM; // = "bottom";//$NON-NLS-1$

  static const std::string VAL_STACK; // = "stack";//$NON-NLS-1$

  static const std::string VAL_FAST; // = "fast";//$NON-NLS-1$

  static const std::string VAL_TRUE; // = "true";//$NON-NLS-1$

  // VAL_FALSE added by dan_rubel@instantiations.com
  // TODO: this logic is backwards... we should be checking for true, but
  // technically this is API now...
  //static const std::string VAL_FALSE; // = "false";//$NON-NLS-1$

  // IExtensionTracker tracker;

  /**
   * Returns whether the given tag should be included.
   */
  bool IncludeTag(const std::string& tag);

  /**
   * Process an action set.
   */
  bool ProcessActionSet(IConfigurationElement::Pointer element);

  /**
   * Process an extension.
   * Assumption: Extension is for current perspective.
   */
  bool ProcessExtension(IConfigurationElement::Pointer element);

  /**
   * Process a perspective shortcut
   */
  bool ProcessPerspectiveShortcut(IConfigurationElement::Pointer element);

  /**
   * Process a show in element.
   */
  bool ProcessShowInPart(IConfigurationElement::Pointer element);

  // processView(IConfigurationElement) modified by dan_rubel@instantiations.com
  /**
   * Process a view
   */
  bool ProcessView(IConfigurationElement::Pointer element);

  /**
   * Process a view shortcut
   */
  bool ProcessViewShortcut(IConfigurationElement::Pointer element);

  /**
   * Process a wizard shortcut
   */
  //bool ProcessWizardShortcut(IConfigurationElement::Pointer element);

protected:

  bool ReadElement(IConfigurationElement::Pointer element);

public:

  /**
   * PerspectiveExtensionReader constructor..
   */
  PerspectiveExtensionReader();

  /**
   * Read the view extensions within a registry.
   *
   * @param extensionTracker the tracker
   * @param id the id
   * @param out the layout
   */
  void ExtendLayout(const std::string& id,
      PageLayout::Pointer out);

  /**
   * Sets the tags to include.  All others are ignored.
   *
   * @param tags the tags to include
   */
  void SetIncludeOnlyTags(const std::vector<std::string>& tags);
};

}

#endif /* BERRYPERSPECTIVEEXTENSIONREADER_H_ */
