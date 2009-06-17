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

#ifndef CHERRYIMEMENTO_H_
#define CHERRYIMEMENTO_H_

#include <cherryMacros.h>
#include <cherryObject.h>

#include "cherryUiDll.h"

#include <string>

namespace cherry {

/**
 * \ingroup org_opencherry_ui
 *
 **/
struct CHERRY_UI IMemento : public Object
{
  cherryInterfaceMacro(IMemento, cherry)

public:

      /**
     * Creates a new child of this memento with the given type.
     * <p>
     * The <code>getChild</code> and <code>getChildren</code> methods
     * are used to retrieve children of a given type.
     * </p>
     *
     * @param type the type
     * @return a new child memento
     * @see #getChild
     * @see #getChildren
     */
    virtual IMemento::Pointer CreateChild(const std::string& type) = 0;

    /**
     * Creates a new child of this memento with the given type and id.
     * The id is stored in the child memento (using a special reserved
     * key, <code>TAG_ID</code>) and can be retrieved using <code>getId</code>.
     * <p>
     * The <code>getChild</code> and <code>getChildren</code> methods
     * are used to retrieve children of a given type.
     * </p>
     *
     * @param type the type
     * @param id the child id
     * @return a new child memento with the given type and id
     * @see #getID
     */
    virtual IMemento::Pointer CreateChild(const std::string& type, const std::string& id) = 0;

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
    virtual std::vector< IMemento::Pointer > GetChildren(const std::string& type) const = 0;


    /**
     * Returns the floating point value of the given key.
     *
     * @param key the key
     * @return the value, or <code>null</code> if the key was not found or was found
     *   but was not a floating point number
     */
    virtual float GetFloat(const std::string& key) const = 0;

    /**
     * Returns the integer value of the given key.
     *
     * @param key the key
     * @return the value, or <code>null</code> if the key was not found or was found
     *   but was not an integer
     */
    virtual int GetInteger(const std::string& key) const = 0;

    /**
     * Returns the string value of the given key.
     *
     * @param key the key
     * @return the value, or <code>null</code> if the key was not found
     */
    virtual const std::string& GetString(const std::string& key) const = 0;

    /**
	 * Returns the boolean value of the given key.
	 * 
	 * @param key the key
	 * @return the value, or <code>null</code> if the key was not found
     * @since 3.4
	 */
	  virtual bool GetBoolean(const std::string& key) const = 0;

	/**
     * Returns the data of the Text node of the memento. Each memento is allowed
     * only one Text node.
     * 
     * @return the data of the Text node of the memento, or <code>null</code>
     * if the memento has no Text node.
     * @since 2.0
     */
    virtual const std::string& GetTextData() const = 0;
	
    /**
     * Returns an array of all the attribute keys of the memento. This will not
     * be <code>null</code>. If there are no keys, an array of length zero will
     * be returned. 
     * @return an array with all the attribute keys of the memento
     * @since 3.4
     */
    virtual std::vector< std::string > GetAttributeKeys() const = 0;



      /**
     * Sets the value of the given key to the given floating point number.
     *
     * @param key the key
     * @param value the value
     */
    virtual void PutFloat(const std::string& key, float value) = 0;

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
     * @since 3.4
	 */
	virtual void PutBoolean(const std::string& key, bool value) = 0;

    /**
     * Sets the memento's Text node to contain the given data. Creates the Text node if
     * none exists. If a Text node does exist, it's current contents are replaced. 
     * Each memento is allowed only one text node.
     * 
     * @param data the data to be placed on the Text node
     * @since 2.0
     */
    virtual void PutTextData(const std::string& data) = 0;



  virtual ~IMemento() {};
};

} // namespace cherry

#endif /*CHERRYIMEMENTO_H_*/
