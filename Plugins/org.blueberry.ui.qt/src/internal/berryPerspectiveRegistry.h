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


#ifndef BERRYPERSPECTIVEREGISTRY_H_
#define BERRYPERSPECTIVEREGISTRY_H_

#include "berryIPerspectiveRegistry.h"

#include "berryIExtensionChangeHandler.h"
#include "berryPerspectiveDescriptor.h"

#include <list>

namespace berry {

class XMLMemento;

/**
 * Perspective registry.
 */
class PerspectiveRegistry : public IPerspectiveRegistry,
     public IExtensionChangeHandler
{

  friend class PerspectiveDescriptor;

private:

  QString defaultPerspID;

  static const QString EXT; // = "_persp.xml";

  static const QString ID_DEF_PERSP; // = "PerspectiveRegistry.DEFAULT_PERSP";

  static const QString PERSP; // = "_persp";

  static const char SPACE_DELIMITER; // = ' ';

  QList<PerspectiveDescriptor::Pointer> perspectives;

  // keep track of the perspectives the user has selected to remove or revert
  QList<QString> perspToRemove;

  class PreferenceChangeListener;
  QScopedPointer<PreferenceChangeListener> preferenceListener;

  virtual void AddExtension(IExtensionTracker* tracker, const SmartPointer<IExtension>& extension) override;

  virtual void RemoveExtension(const SmartPointer<IExtension>& extension,
                               const QList<SmartPointer<Object> >& objects) override;

public:

  /**
   * Construct a new registry.
   */
  PerspectiveRegistry();

  /**
   * Adds a perspective. This is typically used by the reader.
   *
   * @param desc
   */
  void AddPerspective(PerspectiveDescriptor::Pointer desc);

  /**
   * Create a new perspective.
   *
   * @param label
   *            the name of the new descriptor
   * @param originalDescriptor
   *            the descriptor on which to base the new descriptor
   * @return a new perspective descriptor or <code>null</code> if the
   *         creation failed.
   */
  IPerspectiveDescriptor::Pointer CreatePerspective(const QString& label,
                                                    IPerspectiveDescriptor::Pointer originalDescriptor);

  /**
   * Reverts a list of perspectives back to the plugin definition
   *
   * @param perspToRevert
   */
  void RevertPerspectives(const QList<PerspectiveDescriptor::Pointer>& perspToRevert);

  /**
   * Deletes a list of perspectives
   *
   * @param perspToDelete
   */
  void DeletePerspectives(const QList<PerspectiveDescriptor::Pointer>& perspToDelete);

  /**
   * Delete a perspective. Has no effect if the perspective is defined in an
   * extension.
   *
   * @param in
   */
  void DeletePerspective(IPerspectiveDescriptor::Pointer in) override;

  /**
   * Loads the registry.
   */
  void Load();

  /**
   * Saves a custom perspective definition to the preference store.
   *
   * @param desc
   *            the perspective
   * @param memento
   *            the memento to save to
   * @throws IOException
   */
  void SaveCustomPersp(PerspectiveDescriptor::Pointer desc, XMLMemento* memento);

  /**
   * Gets the Custom perspective definition from the preference store.
   *
   * @param id
   *            the id of the perspective to find
   * @return IMemento a memento containing the perspective description
   *
   * @throws WorkbenchException
   * @throws IOException
   */
  IMemento::Pointer GetCustomPersp(const QString& id);

  /**
   * Return <code>true</code> if a label is valid. This checks only the
   * given label in isolation. It does not check whether the given label is
   * used by any existing perspectives.
   *
   * @param label
   *            the label to test
   * @return whether the label is valid
   */
  bool ValidateLabel(const QString& label);

  /**
   * Dispose the receiver.
   */
  ~PerspectiveRegistry();

  // ---------- IPerspectiveRegistry methods ------------

  virtual IPerspectiveDescriptor::Pointer FindPerspectiveWithId(const QString& id) override;
  virtual IPerspectiveDescriptor::Pointer FindPerspectiveWithLabel(const QString& label) override;

  virtual QString GetDefaultPerspective() override;

  virtual QList<IPerspectiveDescriptor::Pointer> GetPerspectives() override;

  virtual void SetDefaultPerspective(const QString& id) override;

  virtual IPerspectiveDescriptor::Pointer ClonePerspective(const QString& id, const QString& label,
      IPerspectiveDescriptor::Pointer originalDescriptor) override;

  virtual void RevertPerspective(IPerspectiveDescriptor::Pointer perspToRevert) override;

protected:

  /**
   * Removes the custom definition of a perspective from the preference store
   *
   * @param desc
   */
  void DeleteCustomDefinition(PerspectiveDescriptor::Pointer desc);

  /**
   * Method hasCustomDefinition.
   *
   * @param desc
   */
  bool HasCustomDefinition(PerspectiveDescriptor::ConstPointer desc) const;

private:

  /**
   * Initialize the preference change listener.
   */
  void InitializePreferenceChangeListener();

  /**
   * @param desc
   */
  void Add(PerspectiveDescriptor::Pointer desc);

  /**
   * Delete a perspective. This will remove perspectives defined in
   * extensions.
   *
   * @param desc
   *            the perspective to delete
   */
  void InternalDeletePerspective(PerspectiveDescriptor::Pointer desc);

  /**
   * Read children from the file system.
   */
  void LoadCustom();

  /**
   * @param status
   */
  void UnableToLoadPerspective(const QString& status);

  /**
   * Read children from the plugin registry.
   */
  void LoadPredefined();

  /**
   * Verifies the id of the default perspective. If the default perspective is
   * invalid use the workbench default.
   */
  void VerifyDefaultPerspective();

};

}

#endif /* BERRYPERSPECTIVEREGISTRY_H_ */
