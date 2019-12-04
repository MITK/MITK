/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPerspectiveExtensionReader.h"

#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchRegistryConstants.h"
#include "berryDirtyPerspectiveMarker.h"

#include <berryIExtension.h>
#include <berryIExtensionTracker.h>

namespace berry
{

const QString PerspectiveExtensionReader::VAL_LEFT = "left";
const QString PerspectiveExtensionReader::VAL_RIGHT = "right";
const QString PerspectiveExtensionReader::VAL_TOP = "top";
const QString PerspectiveExtensionReader::VAL_BOTTOM = "bottom";
const QString PerspectiveExtensionReader::VAL_STACK = "stack";
const QString PerspectiveExtensionReader::VAL_FAST = "fast";
const QString PerspectiveExtensionReader::VAL_TRUE = "true";
//const QString PerspectiveExtensionReader::VAL_FALSE = "false";

bool PerspectiveExtensionReader::IncludeTag(const QString& tag)
{
  return includeOnlyTags.empty() ||
    std::find(includeOnlyTags.begin(), includeOnlyTags.end(), tag) != includeOnlyTags.end();
}

bool PerspectiveExtensionReader::ProcessActionSet(
    const IConfigurationElement::Pointer& element)
{
  QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (!id.isEmpty())
  {
    //TODO PerspectiveExtensionReader action set
    //pageLayout->AddActionSet(id);
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessExtension(
    const IConfigurationElement::Pointer& element)
{
  foreach (IConfigurationElement::Pointer child, element->GetChildren())
  {
    QString type = child->GetName();
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
        WorkbenchPlugin::Log("Unable to process element: " + type + " in perspective extension: " +
                             element->GetDeclaringExtension()->GetUniqueIdentifier());
      }
    }
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessPerspectiveShortcut(
    const IConfigurationElement::Pointer& element)
{
  QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (!id.isEmpty())
  {
    pageLayout->AddPerspectiveShortcut(id);
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessShowInPart(
    const IConfigurationElement::Pointer& element)
{
  QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (!id.isEmpty())
  {
    pageLayout->AddShowInPart(id);
  }
  return true;
}

bool PerspectiveExtensionReader::ProcessView(
    const IConfigurationElement::Pointer& element)
{
  // Get id, relative, and relationship.
  QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (id.isEmpty())
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_ID);
    return false;
  }

  QString relative = element->GetAttribute(WorkbenchRegistryConstants::ATT_RELATIVE);

  QString relationship = element->GetAttribute(WorkbenchRegistryConstants::ATT_RELATIONSHIP);
  if (relationship.isEmpty())
  {
    this->LogMissingAttribute(element, WorkbenchRegistryConstants::ATT_RELATIONSHIP);
    return false;
  }

  if (VAL_FAST != relationship && relative.isEmpty())
    {
      this->LogError(element, "Attribute '" + WorkbenchRegistryConstants::ATT_RELATIVE
          + "' not defined.  This attribute is required when "
          + WorkbenchRegistryConstants::ATT_RELATIONSHIP + "=\"" + relationship
          + "\"."); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$ //$NON-NLS-4$
      return false;
    }

    // Get relationship details.
    bool stack = false;
    //bool fast = false;
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
//    else if (relationship == VAL_FAST)
//    {
//      fast = true;
//    }
    else
    {
      return false;
    }



  float ratio = 0.5f;
  QString ratioString = element->GetAttribute(WorkbenchRegistryConstants::ATT_RATIO);
  if (!ratioString.isEmpty())
  {
    bool ok = false;
    ratio = ratioString.toFloat(&ok);
    if (!ok) return false;

    // If the ratio is outside the allowable range, mark it as invalid.
    if (ratio < IPageLayout::RATIO_MIN || ratio > IPageLayout::RATIO_MAX)
    {
      ratio = IPageLayout::INVALID_RATIO;
    }
  }
  else
  {
    // The ratio has not been specified.
    ratio = IPageLayout::nullptr_RATIO;
  }


  QString strVisible = element->GetAttribute(WorkbenchRegistryConstants::ATT_VISIBLE);
  bool visible = (VAL_TRUE == strVisible);

  QString closeable = element->GetAttribute(WorkbenchRegistryConstants::ATT_CLOSEABLE);

  QString moveable = element->GetAttribute(WorkbenchRegistryConstants::ATT_MOVEABLE);

  QString standalone = element->GetAttribute(WorkbenchRegistryConstants::ATT_STANDALONE);

  QString showTitle = element->GetAttribute(WorkbenchRegistryConstants::ATT_SHOW_TITLE);

  QString minVal = element->GetAttribute(WorkbenchRegistryConstants::ATT_MINIMIZED);
  // Default to 'false'
  bool minimized = (VAL_TRUE == minVal);


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
//    if (ratio == IPageLayout::nullptr_RATIO)
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
    if (ratio == IPageLayout::nullptr_RATIO || ratio == IPageLayout::INVALID_RATIO)
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
    if (!closeable.isEmpty())
    {
      viewLayout->SetCloseable(VAL_TRUE == closeable);
    }
    if (!moveable.isEmpty())
    {
      viewLayout->SetMoveable(VAL_TRUE == moveable);
    }
  }

  return true;
}

bool PerspectiveExtensionReader::ProcessViewShortcut(
    const IConfigurationElement::Pointer& element)
{
  QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_ID);
  if (!id.isEmpty())
  {
    pageLayout->AddShowViewShortcut(id);
  }
  return true;
}

//bool PerspectiveExtensionReader::ProcessWizardShortcut(
//    IConfigurationElement::Pointer element)
//{
//  QString id = element.getAttribute(IWorkbenchRegistryConstants.ATT_ID);
//  if (id != null)
//  {
//    pageLayout.addNewWizardShortcut(id);
//  }
//  return true;
//}

bool PerspectiveExtensionReader::ReadElement(
    const IConfigurationElement::Pointer& element)
{
  QString type = element->GetName();
  if (type == WorkbenchRegistryConstants::TAG_PERSPECTIVE_EXTENSION)
  {
    QString id = element->GetAttribute(WorkbenchRegistryConstants::ATT_TARGET_ID);
    if (targetID == id || "*" == id)
    {
      if (tracker != nullptr)
      {
        Object::Pointer obj(new DirtyPerspectiveMarker(id));
        tracker->RegisterObject(element->GetDeclaringExtension(),
                                obj, IExtensionTracker::REF_STRONG);
      }
      return this->ProcessExtension(element);
    }
    return true;
  }
  return false;
}

PerspectiveExtensionReader::PerspectiveExtensionReader()
  : tracker(nullptr)
{
  // do nothing
}

void PerspectiveExtensionReader::ExtendLayout(IExtensionTracker* extensionTracker,
                                              const QString& id,
                                              PageLayout::Pointer out)
{
  tracker = extensionTracker;
  targetID = id;
  pageLayout = out;
  this->ReadRegistry(Platform::GetExtensionRegistry(),
                     PlatformUI::PLUGIN_ID(),
                     WorkbenchRegistryConstants::PL_PERSPECTIVE_EXTENSIONS);
}

void PerspectiveExtensionReader::SetIncludeOnlyTags(
    const QList<QString>& tags)
{
  includeOnlyTags = tags;
}

}
