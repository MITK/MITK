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

#ifndef BERRYHANDLEOBJECT_H_
#define BERRYHANDLEOBJECT_H_

#include <berryMacros.h>
#include <berryObject.h>
#include <org_blueberry_core_commands_Export.h>

#include <Poco/Hash.h>

namespace berry {

/**
 * <p>
 * An object that can exist in one of two states: defined and undefined. This is
 * used by APIs that want to give a handle to an object, even though the object
 * does not fully exist yet. This way, users can attach listeners to objects
 * before they come into existence. It also protects the API from users that do
 * not release references when they should.
 * </p>
 * <p>
 * To enforce good coding practice, all handle objects must implement
 * <code>equals</code> and <code>toString</code>. Please use
 * <code>string</code> to cache the result for <code>toString</code> once
 * calculated.
 * </p>
 * <p>
 * All handle objects are referred to using a single identifier. This identifier
 * is a instance of <code>String</code>. It is important that this identifier
 * remain unique within whatever context that handle object is being used. For
 * example, there should only ever be one instance of <code>Command</code>
 * with a given identifier.
 * </p>
 *
 * @since 3.1
 */
class BERRY_COMMANDS HandleObject : public virtual Object { // extends EventManager implements IIdentifiable {

public:

  berryObjectMacro(HandleObject);

private:

  /**
   * The constant integer hash code value meaning the hash code has not yet
   * been computed.
   */
  static const std::size_t HASH_CODE_NOT_COMPUTED; // = 0;

  /**
   * A factor for computing the hash code for all schemes.
   */
  static const std::size_t HASH_FACTOR; // = 89;

  /**
   * The seed for the hash code for all schemes.
   */
  static const std::size_t HASH_INITIAL;

  /**
   * The hash code for this object. This value is computed lazily, and marked
   * as invalid when one of the values on which it is based changes.
   */
  mutable std::size_t hashCode; // = HASH_CODE_NOT_COMPUTED;


protected:

  /**
   * Whether this object is defined. A defined object is one that has been
   * fully initialized. By default, all objects start as undefined.
   */
   bool defined;

  /**
   * The identifier for this object. This identifier should be unique across
   * all objects of the same type and should never change. This value will
   * never be <code>null</code>.
   */
  const  std::string id;

  /**
   * The string representation of this object. This string is for debugging
   * purposes only, and is not meant to be displayed to the user. This value
   * is computed lazily, and is cleared if one of its dependent values
   * changes.
   */
  mutable std::string str;

  /**
   * Constructs a new instance of <code>HandleObject</code>.
   *
   * @param id
   *            The id of this handle; must not be <code>null</code>.
   */
  HandleObject(const std::string& id);


public:

    /**
     * Tests whether this object is equal to another object. A handle object is
     * only equal to another handle object with the same id and the same class.
     *
     * @param object
     *            The object with which to compare; may be <code>null</code>.
     * @return <code>true</code> if the objects are equal; <code>false</code>
     *         otherwise.
     */
   bool operator==(const Object* object) const;

   virtual std::string GetId() const;

    /**
     * Computes the hash code for this object based on the id.
     *
     * @return The hash code for this object.
     */
    virtual std::size_t HashCode() const {
        if (hashCode == HASH_CODE_NOT_COMPUTED) {
      hashCode = HASH_INITIAL * HASH_FACTOR + Poco::hash(id);
      if (hashCode == HASH_CODE_NOT_COMPUTED) {
        hashCode++;
      }
    }
    return hashCode;
    }

    /**
     * Whether this instance is defined. A defined instance is one that has been
     * fully initialized. This allows objects to effectively disappear even
     * though other objects may still have references to them.
     *
     * @return <code>true</code> if this object is defined; <code>false</code>
     *         otherwise.
     */
    virtual bool IsDefined() const;

    /**
     * Makes this object becomes undefined. This method should make any defined
     * properties <code>null</code>. It should also send notification to any
     * listeners that these properties have changed.
     */
    virtual void Undefine() = 0;
};

}

#endif /*BERRYHANDLEOBJECT_H_*/
