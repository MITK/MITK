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

#ifndef BERRYIMEMENTO_H_
#define BERRYIMEMENTO_H_

#include <berryMacros.h>
#include <berryObject.h>

#include "berryUiDll.h"

#include <string>

namespace berry
{

/**
 * \ingroup org_blueberry_ui
 *
 * Interface to a memento used for saving the important state of an object
 * in a form that can be persisted in the file system.
 * <p>
 * Mementos were designed with the following requirements in mind:
 * <ol>
 *  <li>Certain objects need to be saved and restored across platform sessions.
 *    </li>
 *  <li>When an object is restored, an appropriate class for an object might not
 *    be available. It must be possible to skip an object in this case.</li>
 *  <li>When an object is restored, the appropriate class for the object may be
 *    different from the one when the object was originally saved. If so, the
 *    new class should still be able to read the old form of the data.</li>
 * </ol>
 * </p>
 * <p>
 * Mementos meet these requirements by providing support for storing a
 * mapping of arbitrary string keys to primitive values, and by allowing
 * mementos to have other mementos as children (arranged into a tree).
 * A robust external storage format based on XML is used.
 * </p><p>
 * The key for an attribute may be any alpha numeric value.  However, the
 * value of <code>TAG_ID</code> is reserved for internal use.
 * </p><p>
 * This interface is not intended to be implemented or extended by clients.
 * </p>
 *
 * @see IPersistableElement
 * @see IElementFactory
 * @noimplement This interface is not intended to be implemented by clients.
 *
 **/
struct BERRY_UI IMemento: public Object
{
berryInterfaceMacro(IMemento, berry)

  /**
   * Special reserved key used to store the memento id
   * (value <code>"IMemento.internal.id"</code>).
   *
   * @see #getID()
   */
  static const std::string TAG_ID; // = "IMemento.internal.id";

  /**
   * Creates a new child of this memento with the given type.
   * <p>
   * The <code>GetChild</code> and <code>GetChildren</code> methods
   * are used to retrieve children of a given type.
   * </p>
   *
   * @param type the type
   * @return a new child memento
   * @see #GetChild
   * @see #GetChildren
   */
  virtual IMemento::Pointer CreateChild(const std::string& type) = 0;

  /**
   * Creates a new child of this memento with the given type and id.
   * The id is stored in the child memento (using a special reserved
   * key, <code>TAG_ID</code>) and can be retrieved using <code>GetID</code>.
   * <p>
   * The <code>GetChild</code> and <code>GetChildren</code> methods
   * are used to retrieve children of a given type.
   * </p>
   *
   * @param type the type
   * @param id the child id
   * @return a new child memento with the given type and id
   * @see #GetID
   */
  virtual IMemento::Pointer CreateChild(const std::string& type,
      const std::string& id) = 0;

  /**
   * Returns the first child with the given type id.
   *
   * @param type the type id
   * @return the first child with the given type
   */
  virtual IMemento::Pointer GetChild(const std::string& type) const = 0;

  /**
   * Returns all children with the given type id.
   *
   * @param type the type id
   * @return an array of children with the given type
   */
  virtual std::vector<IMemento::Pointer>
      GetChildren(const std::string& type) const = 0;

  /**
   * Gets the floating point value of the given key.
   *
   * @param key the key
   * @param value the value of the given key
   * @return false if the key was not found or was found
   *   but was not a floating point number, else true
   */
  virtual bool GetFloat(const std::string& key, double& value) const = 0;

  /**
   * Gets the integer value of the given key.
   *
   * @param key the key
   * @param value the value of the given key
   * @return false if the key was not found or was found
   *   but was not an integer, else true
   */
  virtual bool GetInteger(const std::string& key, int& value) const = 0;

  /**
   * Gets the string value of the given key.
   *
   * @param key the key
   * @param value the value of the given key
   * @return false if the key was not found, else true
   */
  virtual bool GetString(const std::string& key, std::string& value) const = 0;

  /**
   * Gets the boolean value of the given key.
   *
   * @param key the key
   * @param value the value of the given key
   * @return false if the key was not found, else true
   */
  virtual bool GetBoolean(const std::string& key, bool& value) const = 0;

  /**
   * Returns the data of the Text node of the memento. Each memento is allowed
   * only one Text node.
   *
   * @return the data of the Text node of the memento, or <code>null</code>
   * if the memento has no Text node.
   */
  virtual const std::string& GetTextData() const = 0;

  /**
   * Returns an array of all the attribute keys of the memento. This will not
   * be <code>null</code>. If there are no keys, an array of length zero will
   * be returned.
   * @return an array with all the attribute keys of the memento
   */
  virtual std::vector<std::string> GetAttributeKeys() const = 0;

  /**
   * Returns the type for this memento.
   *
   * @return the memento type
   * @see #CreateChild(const std::string&)
   * @see #CreateChild(const std::string&, const std::string&)
   */
  virtual std::string GetType() const = 0;

  /**
   * Returns the id for this memento.
   *
   * @return the memento id, or <code>""</code> if none
   * @see #CreateChild(const std::string&, const std::string&)
   */
  virtual std::string GetID() const = 0;

  /**
   * Sets the value of the given key to the given floating point number.
   *
   * @param key the key
   * @param value the value
   */
  virtual void PutFloat(const std::string& key, double value) = 0;

  /**
   * Sets the value of the given key to the given integer.
   *
   * @param key the key
   * @param value the value
   */
  virtual void PutInteger(const std::string& key, int value) = 0;

  /**
   * Copy the attributes and children from  <code>memento</code>
   * to the receiver.
   *
   * @param memento the IMemento to be copied.
   */
  virtual void PutMemento(IMemento::Pointer memento) = 0;

  /**
   * Sets the value of the given key to the given const std::string&.
   *
   * @param key the key
   * @param value the value
   */
  virtual void PutString(const std::string& key, const std::string& value) = 0;

  /**
   * Sets the value of the given key to the given boolean value.
   *
   * @param key the key
   * @param value the value
   */
  virtual void PutBoolean(const std::string& key, bool value) = 0;

  /**
   * Sets the memento's Text node to contain the given data. Creates the Text node if
   * none exists. If a Text node does exist, it's current contents are replaced.
   * Each memento is allowed only one text node.
   *
   * @param data the data to be placed on the Text node
   */
  virtual void PutTextData(const std::string& data) = 0;

  virtual ~IMemento()
  {
  }
  ;
};

} // namespace berry

#endif /*BERRYIMEMENTO_H_*/
