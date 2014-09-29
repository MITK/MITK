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

#ifndef BERRYVIEWDESCRIPTOR_H_
#define BERRYVIEWDESCRIPTOR_H_

#include "berryIViewPart.h"
#include "berryIViewDescriptor.h"

#include "service/berryIConfigurationElement.h"

#include <string>
#include <vector>

namespace berry
{
struct IHandlerActivation;

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class ViewDescriptor : public IViewDescriptor
{
private:

  std::string id;
  mutable SmartPointer<ImageDescriptor> imageDescriptor;
  IConfigurationElement::Pointer configElement;
  std::vector<std::string> categoryPath;

  /**
   * The activation token returned when activating the show view handler with
   * the workbench.
   */
  SmartPointer<IHandlerActivation> handlerActivation;

public:

  berryObjectMacro(ViewDescriptor);

  /**
   * Create a new <code>ViewDescriptor</code> for an extension.
   *
   * @param e the configuration element
   * @throws CoreException thrown if there are errors in the configuration
   */
  ViewDescriptor(IConfigurationElement::Pointer e);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#createView()
   */
  IViewPart::Pointer CreateView();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#getCategoryPath()
   */
  const std::vector<std::string>& GetCategoryPath() const;

  /**
   * Return the configuration element for this descriptor.
   *
   * @return the configuration element
   */
  IConfigurationElement::Pointer GetConfigurationElement() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#getDescription()
   */
  std::string GetDescription() const;

  std::vector< std::string> GetKeywordReferences() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPartDescriptor#getId()
   */
  std::string GetId() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPartDescriptor#getImageDescriptor()
   */
  SmartPointer<ImageDescriptor> GetImageDescriptor() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPartDescriptor#getLabel()
   */
  std::string GetLabel() const;

  /**
   * Return the accelerator attribute.
   *
   * @return the accelerator attribute
   */
  std::string GetAccelerator() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#getAllowMultiple()
   */
  bool GetAllowMultiple() const;

  /* (non-Javadoc)
   * @see org.eclipse.ui.views.IViewDescriptor#getRestorable()
   */
  bool IsRestorable() const;

  bool operator==(const Object*) const;

  /**
   * Activates a show view handler for this descriptor. This handler can later
   * be deactivated by calling {@link ViewDescriptor#deactivateHandler()}.
   * This method will only activate the handler if it is not currently active.
   *
   */
  void ActivateHandler();

  /**
   * Deactivates the show view handler for this descriptor. This handler was
   * previously activated by calling {@link ViewDescriptor#activateHandler()}.
   * This method will only deactivative the handler if it is currently active.
   *
   */
  void DeactivateHandler();

protected:

  /* (non-Javadoc)
   * @see IAdaptable#GetAdapterImpl(const std::type_info&)
   */
  Poco::Any GetAdapter(const std::string& adapter);

private:
  /**
   * load a view descriptor from the registry.
   */
  void LoadFromExtension();
};
}

#endif /*BERRYVIEWDESCRIPTOR_H_*/
