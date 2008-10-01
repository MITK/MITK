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

#include "cherryPerspectiveExtensionReader.h"

#include "cherryWorkbenchPlugin.h"
#include "cherryWorkbenchRegistryConstants.h"

#include <Poco/NumberParser.h>

namespace cherry
{

const std::string PerspectiveExtensionReader::VAL_LEFT = "left";//$NON-NLS-1$
const std::string PerspectiveExtensionReader::VAL_RIGHT = "right";//$NON-NLS-1$
const std::string PerspectiveExtensionReader::VAL_TOP = "top";//$NON-NLS-1$
const std::string PerspectiveExtensionReader::VAL_BOTTOM = "bottom";//$NON-NLS-1$
const std::string PerspectiveExtensionReader::VAL_STACK = "stack";//$NON-NLS-1$
const std::string PerspectiveExtensionReader::VAL_FAST = "fast";//$NON-NLS-1$
const std::string PerspectiveExtensionReader::VAL_TRUE = "true";//$NON-NLS-1$
//const std::string PerspectiveExtensionReader::VAL_FALSE = "false";//$NON-NLS-1$

bool PerspectiveExtensionReader::IncludeTag(const std::string& tag)
{
  return includeOnlyTags.empty() ||
    std::find(includeOnlyTags.begin(), includeOnlyTags.end(), tag) != includeOnlyTags.end();
}

bool PerspectiveExtensionReader::ProcessActionSet(
    IConfigurationElement::Pointer element)
{
  std::string id;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id))
  {
    //TODO PerspectiveExtensionReader action set
    //pageLayout->AddActionSet(id);
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessExtension(
    IConfigurationElement::Pointer element)
{
  IConfigurationElement::vector children = element->GetChildren();
  for (unsigned int nX = 0; nX < children.size(); nX++)
  {
    IConfigurationElement::Pointer child = children[nX];
    std::string type = child->GetName();
    if (this->IncludeTag(type))
    {
      bool result = false;
      if (type == WorkbenchRegistryConstants::TAG_ACTION_SET)
      {
        result = this->ProcessActionSet(child);
      }
      else if (type == WorkbenchRegistryConstants::TAG_VIEW)
      {
        result = this->ProcessView(child);
      }
      else if (type == WorkbenchRegistryConstants::TAG_VIEW_SHORTCUT)
      {
        result = this->ProcessViewShortcut(child);
      }
//      else if (type == IorkbenchRegistryConstants::TAG_NEW_WIZARD_SHORTCUT)
//      {
//        result = processWizardShortcut(child);
//      }
      else if (type == WorkbenchRegistryConstants::TAG_PERSP_SHORTCUT)
      {
        result = this->ProcessPerspectiveShortcut(child);
      }
      else if (type == WorkbenchRegistryConstants::TAG_SHOW_IN_PART)
      {
        result = this->ProcessShowInPart(child);
      }
      if (!result)
      {
        WorkbenchPlugin::Log("Unable to process element: " + //$NON-NLS-1$
            type + " in perspective extension: " + //$NON-NLS-1$
            element->GetDeclaringExtension()->GetUniqueIdentifier());
      }
    }
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessPerspectiveShortcut(
    IConfigurationElement::Pointer element)
{
  std::string id;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id))
  {
    pageLayout->AddPerspectiveShortcut(id);
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessShowInPart(
    IConfigurationElement::Pointer element)
{
  std::string id;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id))
  {
    pageLayout->AddShowInPart(id);
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessView(
    IConfigurationElement::Pointer element)
{
  // Get id, relative, and relationship.
  std::string id;
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id))
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_ID);
    return false;
  }

  std::string relative;
  bool hasRelative = element->GetAttribute(WorkbenchRegistryConstants::ATT_RELATIVE, relative);

  std::string relationship;
  if (!element->GetAttribute(WorkbenchRegistryConstants::ATT_RELATIONSHIP, relationship))
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_RELATIONSHIP);
    return false;
  }

  if (VAL_FAST != relationship && !hasRelative)
    {
      this->LogError(element, "Attribute '" + WorkbenchRegistryConstants::ATT_RELATIVE
          + "' not defined.  This attribute is required when "
          + WorkbenchRegistryConstants::ATT_RELATIONSHIP + "=\"" + relationship
          + "\"."); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
      return false;
    }

    // Get relationship details.
    bool stack = false;
    bool fast = false;
    int intRelation = 0;
    if (relationship == VAL_LEFT)
    {
      intRelation = IPageLayout::LEFT;
    }
    else if (relationship == VAL_RIGHT)
    {
      intRelation = IPageLayout::RIGHT;
    }
    else if (relationship == VAL_TOP)
    {
      intRelation = IPageLayout::TOP;
    }
    else if (relationship == VAL_BOTTOM)
    {
      intRelation = IPageLayout::BOTTOM;
    }
    else if (relationship == VAL_STACK)
    {
      stack = true;
    }
    else if (relationship == VAL_FAST)
    {
      fast = true;
    }
    else
    {
      return false;
    }



  float ratio = 0.5f;
  std::string ratioString;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_RATIO, ratioString))
  {
    try
    {
      ratio = (float)Poco::NumberParser::parseFloat(ratioString);
    } catch (Poco::SyntaxException& e)
    {
      return false;
    }
    // If the ratio is outside the allowable range, mark it as invalid.
    if (ratio < IPageLayout::RATIO_MIN || ratio > IPageLayout::RATIO_MAX)
    {
      ratio = IPageLayout::INVALID_RATIO;
    }
  }
  else
  {
    // The ratio has not been specified.
    ratio = IPageLayout::NULL_RATIO;
  }


  std::string strVisible;
  element->GetAttribute(WorkbenchRegistryConstants::ATT_VISIBLE, strVisible);
  bool visible = (VAL_TRUE == strVisible);

  std::string closeable;
  bool hasCloseable = element->GetAttribute(
      WorkbenchRegistryConstants::ATT_CLOSEABLE, closeable);

  std::string moveable;
  bool hasMoveable = element->GetAttribute(
      WorkbenchRegistryConstants::ATT_MOVEABLE, moveable);

  std::string standalone;
  element->GetAttribute(
      WorkbenchRegistryConstants::ATT_STANDALONE, standalone);

  std::string showTitle;
  element->GetAttribute(
      WorkbenchRegistryConstants::ATT_SHOW_TITLE, showTitle);

  // Default to 'false'
  std::string minVal;
  bool minimized = false;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_MINIMIZED, minVal))
    minimized = VAL_TRUE == minVal;


  if (visible)
  {
    // If adding a view (not just a placeholder), remove any existing placeholder.
    // See bug 85948 [Perspectives] Adding register & expressions view by default to debug perspective fails
    pageLayout->RemovePlaceholder(id);
  }

  // If stack ..
  if (stack)
  {
    if (visible)
    {
      pageLayout->StackView(id, relative);
    }
    else
    {
      pageLayout->StackPlaceholder(id, relative);
    }
  }
//  // If the view is a fast view...
//  else if (fast)
//  {
//    if (ratio == IPageLayout::NULL_RATIO)
//    {
//      // The ratio has not been specified.
//      pageLayout->AddFastView(id);
//    }
//    else
//    {
//      pageLayout->AddFastView(id, ratio);
//    }
//  }
  else
  {

    // The view is a regular view.
    // If the ratio is not specified or is invalid, use the default ratio.
    if (ratio == IPageLayout::NULL_RATIO || ratio == IPageLayout::INVALID_RATIO)
    {
      ratio = IPageLayout::DEFAULT_VIEW_RATIO;
    }

    if (visible)
    {
      if (VAL_TRUE == standalone)
      {
        pageLayout->AddStandaloneView(id, VAL_TRUE == showTitle,
            intRelation, ratio, relative);
      }
      else
      {
        pageLayout->AddView(id, intRelation, ratio, relative, minimized);
      }
    }
    else
    {
      // Fix for 99155, CGross (schtoo@schtoo.com)
      // Adding standalone placeholder for standalone views
      if (VAL_TRUE == standalone)
      {
        pageLayout->AddStandaloneViewPlaceholder(id, intRelation, ratio,
            relative, VAL_TRUE == showTitle);
      }
      else
      {
        pageLayout->AddPlaceholder(id, intRelation, ratio, relative);
      }
    }
  }
  IViewLayout::Pointer viewLayout = pageLayout->GetViewLayout(id);
  // may be null if it's been filtered by activity
  if (viewLayout != 0)
  {
    if (hasCloseable)
    {
      viewLayout->SetCloseable(VAL_TRUE == closeable);
    }
    if (hasMoveable)
    {
      viewLayout->SetMoveable(VAL_TRUE == moveable);
    }
  }

  return true;
}

bool PerspectiveExtensionReader::ProcessViewShortcut(
    IConfigurationElement::Pointer element)
{
  std::string id;
  if (element->GetAttribute(WorkbenchRegistryConstants::ATT_ID, id))
  {
    pageLayout->AddShowViewShortcut(id);
  }
  return true;
}

//bool PerspectiveExtensionReader::ProcessWizardShortcut(
//    IConfigurationElement::Pointer element)
//{
//  std::string id = element.getAttribute(IWorkbenchRegistryConstants.ATT_ID);
//  if (id != null)
//  {
//    pageLayout.addNewWizardShortcut(id);
//  }
//  return true;
//}

bool PerspectiveExtensionReader::ReadElement(
    IConfigurationElement::Pointer element)
{
  std::string type = element->GetName();
  if (type == WorkbenchRegistryConstants::TAG_PERSPECTIVE_EXTENSION)
  {
    std::string id;
    element->GetAttribute(WorkbenchRegistryConstants::ATT_TARGET_ID, id);
    if (targetID == id || "*" == id)
    { //$NON-NLS-1$
//      if (tracker != null)
//      {
//        tracker.registerObject(element.getDeclaringExtension(),
//            new DirtyPerspectiveMarker(id), IExtensionTracker.REF_STRONG);
//      }
      return this->ProcessExtension(element);
    }
    return true;
  }
  return false;
}

PerspectiveExtensionReader::PerspectiveExtensionReader()
{
  // do nothing
}

void PerspectiveExtensionReader::ExtendLayout(const std::string& id,
    PageLayout::Pointer out)
{
  //tracker = extensionTracker;
  targetID = id;
  pageLayout = out;
  this->ReadRegistry(PlatformUI::PLUGIN_ID,
      WorkbenchRegistryConstants::PL_PERSPECTIVE_EXTENSIONS);
}

void PerspectiveExtensionReader::SetIncludeOnlyTags(
    const std::vector<std::string>& tags)
{
  includeOnlyTags = tags;
}

}
