/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPERSPECTIVEEXTENSIONREADER_H_
#define BERRYPERSPECTIVEEXTENSIONREADER_H_

#include "berryRegistryReader.h"

#include "berryPageLayout.h"

namespace berry
{

struct IExtensionTracker;

/**
 * A strategy to read perspective extension from the registry.
 * A pespective extension is one of a view, viewAction, perspAction,
 * newWizardAction, or actionSet.
 */
class PerspectiveExtensionReader: public RegistryReader
{
private:

  QString targetID;

  PageLayout::Pointer pageLayout;

  QList<QString> includeOnlyTags;

  static const QString VAL_LEFT; // = "left";//$NON-NLS-1$

  static const QString VAL_RIGHT; // = "right";//$NON-NLS-1$

  static const QString VAL_TOP; // = "top";//$NON-NLS-1$

  static const QString VAL_BOTTOM; // = "bottom";//$NON-NLS-1$

  static const QString VAL_STACK; // = "stack";//$NON-NLS-1$

  static const QString VAL_FAST; // = "fast";//$NON-NLS-1$

  static const QString VAL_TRUE; // = "true";//$NON-NLS-1$

  // VAL_FALSE added by dan_rubel@instantiations.com
  // TODO: this logic is backwards... we should be checking for true, but
  // technically this is API now...
  //static const QString VAL_FALSE; // = "false";//$NON-NLS-1$

  IExtensionTracker* tracker;

  /**
   * Returns whether the given tag should be included.
   */
  bool IncludeTag(const QString& tag);

  /**
   * Process an action set.
   */
  bool ProcessActionSet(const IConfigurationElement::Pointer& element);

  /**
   * Process an extension.
   * Assumption: Extension is for current perspective.
   */
  bool ProcessExtension(const IConfigurationElement::Pointer& element);

  /**
   * Process a perspective shortcut
   */
  bool ProcessPerspectiveShortcut(const IConfigurationElement::Pointer& element);

  /**
   * Process a show in element.
   */
  bool ProcessShowInPart(const IConfigurationElement::Pointer& element);

  // processView(IConfigurationElement) modified by dan_rubel@instantiations.com
  /**
   * Process a view
   */
  bool ProcessView(const IConfigurationElement::Pointer& element);

  /**
   * Process a view shortcut
   */
  bool ProcessViewShortcut(const IConfigurationElement::Pointer& element);

  /**
   * Process a wizard shortcut
   */
  //bool ProcessWizardShortcut(IConfigurationElement::Pointer element);

protected:

  bool ReadElement(const IConfigurationElement::Pointer& element) override;

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
  void ExtendLayout(IExtensionTracker* extensionTracker,
                    const QString& id,
                    PageLayout::Pointer out);

  /**
   * Sets the tags to include.  All others are ignored.
   *
   * @param tags the tags to include
   */
  void SetIncludeOnlyTags(const QList<QString>& tags);
};

}

#endif /* BERRYPERSPECTIVEEXTENSIONREADER_H_ */
