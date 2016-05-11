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
#include "berryIPluginContribution.h"

#include <QStringList>
#include <QIcon>

namespace berry
{

struct IConfigurationElement;
struct IHandlerActivation;

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class ViewDescriptor : public IViewDescriptor, public IPluginContribution
{
private:

  QString id;
  mutable QIcon imageDescriptor;
  IConfigurationElement::Pointer configElement;
  QStringList categoryPath;

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
  ViewDescriptor(const SmartPointer<IConfigurationElement>& e);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#createView()
   */
  IViewPart::Pointer CreateView() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#getCategoryPath()
   */
  QStringList GetCategoryPath() const override;

  /**
   * Return the configuration element for this descriptor.
   *
   * @return the configuration element
   */
  IConfigurationElement::Pointer GetConfigurationElement() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#getDescription()
   */
  QString GetDescription() const override;

  QStringList GetKeywordReferences() const override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPartDescriptor#getId()
   */
  QString GetId() const override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPartDescriptor#getImageDescriptor()
   */
  QIcon GetImageDescriptor() const override;

  /* (non-Javadoc)
  * @see org.blueberry.ui.IWorkbenchPartDescriptor#getCase()
  */
  QString GetCase() const override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.IWorkbenchPartDescriptor#getLabel()
   */
  QString GetLabel() const override;

  /**
   * Return the accelerator attribute.
   *
   * @return the accelerator attribute
   */
  QString GetAccelerator() const;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.registry.IViewDescriptor#getAllowMultiple()
   */
  bool GetAllowMultiple() const override;

  /* (non-Javadoc)
   * @see org.eclipse.ui.views.IViewDescriptor#getRestorable()
   */
  bool IsRestorable() const override;

  bool operator==(const Object*) const override;

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

  /*
   * @see IPluginContribution#GetPluginId()
   */
   QString GetPluginId() const override;

   /*
    * @see IPluginContribution#GetLocalId()
    */
   QString GetLocalId() const override;

protected:

  /* (non-Javadoc)
   * @see IAdaptable#GetAdapterImpl(const std::type_info&)
   */
  Object* GetAdapter(const QString& adapter) const override;

private:
  /**
   * load a view descriptor from the registry.
   */
  void LoadFromExtension();
};

}

#endif /*BERRYVIEWDESCRIPTOR_H_*/
