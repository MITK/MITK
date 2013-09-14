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

#ifndef BERRYPERSPECTIVEDESCRIPTOR_H_
#define BERRYPERSPECTIVEDESCRIPTOR_H_

#include <berryIConfigurationElement.h>

#include "berryIPerspectiveDescriptor.h"
#include "berryIPerspectiveFactory.h"
#include "berryIMemento.h"

#include <string>

namespace berry {

/**
 * \ingroup org_blueberry_ui_internal
 *
 * PerspectiveDescriptor.
 * <p>
 * A PerspectiveDesciptor has 3 states:
 * </p>
 * <ol>
 * <li>It <code>isPredefined()</code>, in which case it was defined from an
 * extension point.</li>
 * <li>It <code>isPredefined()</code> and <code>hasCustomFile</code>, in
 * which case the user has customized a predefined perspective.</li>
 * <li>It <code>hasCustomFile</code>, in which case the user created a new
 * perspective.</li>
 * </ol>
 *
 */
class PerspectiveDescriptor : public IPerspectiveDescriptor {

public:
  berryObjectMacro(PerspectiveDescriptor);

private:

  std::string id;

   std::string pluginId;

   std::string originalId;

   std::string label;

   std::string className;

   std::string description;

   bool singleton;

   bool fixed;

   mutable ImageDescriptor::Pointer imageDescriptor;

   IConfigurationElement::Pointer configElement;

  /**
   * Create a new empty descriptor.
   *
   * @param id
   *            the id of the new descriptor
   * @param label
   *            the label of the new descriptor
   * @param originalDescriptor
   *            the descriptor that this descriptor is based on
   */
  public: PerspectiveDescriptor(const std::string& id, const std::string& label,
      PerspectiveDescriptor::Pointer originalDescriptor);

  /**
   * Create a descriptor from a config element.
   *
   * @param id
   *            the id of the element to create
   * @param configElement
   *            the element to base this perspective on
   * @throws CoreException
   *             thrown if there are any missing attributes
   */
  public: PerspectiveDescriptor(const std::string& id, IConfigurationElement::Pointer configElement);

  /**
   * Creates a factory for a predefined perspective. If the perspective is not
   * predefined return <code>null</code>.
   *
   * @return the IPerspectiveFactory or <code>null</code>
   * @throws CoreException
   *             if the object could not be instantiated.
   */
  public: IPerspectiveFactory::Pointer CreateFactory();

  /**
   * Deletes the custom definition for a perspective..
   */
  public: void DeleteCustomDefinition();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IPerspectiveDescriptor#getDescription()
   */
  public: std::string GetDescription() const;

  /**
   * Returns whether or not this perspective is fixed.
   *
   * @return whether or not this perspective is fixed
   */
  public: bool GetFixed() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IPerspectiveDescriptor#getId()
   */
  public: std::string GetId() const;

  public: std::string GetPluginId() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IPerspectiveDescriptor#getImageDescriptor()
   */
  public: ImageDescriptor::Pointer GetImageDescriptor() const;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.IPerspectiveDescriptor#getLabel()
   */
  public: std::string GetLabel() const;
  /**
   * Return the original id of this descriptor.
   *
   * @return the original id of this descriptor
   */
  public: std::string GetOriginalId() const;

  /**
   * Returns <code>true</code> if this perspective has a custom definition.
   *
   * @return whether this perspective has a custom definition
   */
  public: bool HasCustomDefinition() const;

  /**
   * Returns <code>true</code> if this perspective wants to be default.
   *
   * @return whether this perspective wants to be default
   */
  public: bool HasDefaultFlag() const;

  /**
   * Returns <code>true</code> if this perspective is predefined by an
   * extension.
   *
   * @return boolean whether this perspective is predefined by an extension
   */
  public: bool IsPredefined() const;

  /**
   * Returns <code>true</code> if this perspective is a singleton.
   *
   * @return whether this perspective is a singleton
   */
  public: bool IsSingleton() const;

  /**
   * Restore the state of a perspective from a memento.
   *
   * @param memento
   *            the memento to restore from
   * @return the <code>IStatus</code> of the operation
   * @see org.blueberry.ui.IPersistableElement
   */
  public: bool RestoreState(IMemento::Pointer memento);

  /**
   * Revert to the predefined extension template. Does nothing if this
   * descriptor is user defined.
   */
  public: void RevertToPredefined();

  /**
   * Save the state of a perspective to a memento.
   *
   * @param memento
   *            the memento to restore from
   * @return the <code>IStatus</code> of the operation
   * @see org.blueberry.ui.IPersistableElement
   */
  public: bool SaveState(IMemento::Pointer memento);

  /**
   * Return the configuration element used to create this perspective, if one
   * was used.
   *
   * @return the configuration element used to create this perspective
   * @since 3.0
   */
  public: IConfigurationElement::Pointer GetConfigElement() const;

  /**
   * Returns the factory class name for this descriptor.
   *
   * @return the factory class name for this descriptor
   * @since 3.1
   */
  public: std::string GetFactoryClassName() const;

};

}

#endif /*BERRYPERSPECTIVEDESCRIPTOR_H_*/
