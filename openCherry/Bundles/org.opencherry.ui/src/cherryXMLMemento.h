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


#ifndef CHERRYXMLMEMENTO_H_
#define CHERRYXMLMEMENTO_H_

#include <cherryMacros.h>
#include "cherryIMemento.h"
#include "cherryUIException.h"

#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/SAX/XMLReader.h"

#include <iostream>
namespace cherry {

/**
 * This class represents the default implementation of the
 * <code>IMemento</code> interface.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @see IMemento
 */
  class CHERRY_UI XMLMemento : public IMemento {



    /**
    * Creates a <code>Document</code> from the <code>Reader</code>
    * and returns a memento on the first <code>Element</code> for reading
    * the document.
    * <p>
    * Same as calling createReadRoot(reader, null)
    * </p>
    *
    * @param reader the <code>Reader</code> used to create the memento's document
    * @return a memento on the first <code>Element</code> for reading the document
    * @throws WorkbenchException if IO problems, invalid format, or no element.
    */
  public:
    cherryObjectMacro(XMLMemento);
    cherryNewMacro2Param(XMLMemento, Poco::XML::Document*, Poco::XML::Element*);
    //TODO Docu
    typedef std::ostream XMLByteOutputStream;
    typedef std::istream XMLByteInputStream;

    /**
    * Creates a memento for the specified document and element.
    * <p>
    * Clients should use <code>createReadRoot</code> and
    * <code>createWriteRoot</code> to create the initial
    * memento on a document.
    * </p>
    *
    * @param document the document for the memento
    * @param element the element node for the memento
    */
    XMLMemento(Poco::XML::Document* document, Poco::XML::Element* elem);

    //TODO Docu
    XMLMemento();
    static XMLMemento::Pointer CreateReadRoot(cherry::XMLMemento::XMLByteInputStream& reader) throw(WorkbenchException);

    /**
    * Creates a <code>Document</code> from the <code>Reader</code>
    * and returns a memento on the first <code>Element</code> for reading
    * the document.
    *
    * @param reader the <code>Reader</code> used to create the memento's document
    * @param baseDir the directory used to resolve relative file names
    *    in the XML document. This directory must exist and include the
    *    trailing separator. The directory format, including the separators,
    *    must be valid for the platform. Can be <code>null</code> if not
    *    needed.
    * @return a memento on the first <code>Element</code> for reading the document
    * @throws WorkbenchException if IO problems, invalid format, or no element.
    */
    static XMLMemento::Pointer CreateReadRoot(cherry::XMLMemento::XMLByteInputStream& reader, const std::string& baseDir) throw(WorkbenchException);

    /**
    * Returns a root memento for writing a document.
    *
    * @param type the element node type to create on the document
    * @return the root memento for writing a document
    */
    static XMLMemento::Pointer CreateWriteRoot(const std::string& type);



      /* (non-Javadoc)
      * Method declared in IMemento.
      */
      //TODO: not defined in IMemento!
      IMemento::Pointer CopyChild(IMemento::Pointer child);


      /* (non-Javadoc)
      * Method declared in IMemento.
      */
    virtual IMemento::Pointer CreateChild(const std::string& type);

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
    virtual IMemento::Pointer CreateChild(const std::string& type, const std::string& id);

    /**
     * Returns the first child with the given type id.
     *
     * @param type the type id
     * @return the first child with the given type
     */
    virtual IMemento::Pointer GetChild(const std::string& type) const;

    /**
     * Returns all children with the given type id.
     *
     * @param type the type id
     * @return an array of children with the given type
     */
    virtual std::vector< IMemento::Pointer > GetChildren(const std::string& type) const;


    /**
     * Returns the floating point value of the given key.
     *
     * @param key the key
     * @return the value, or <code>null</code> if the key was not found or was found
     *   but was not a floating point number
     */
    virtual float GetFloat(const std::string& key) const;

    //TODO: docu
    virtual const std::string& GetType() const;

    //TODO: docu
    virtual const std::string& GetID() const;
    /**
     * Returns the integer value of the given key.
     *
     * @param key the key
     * @return the value, or <code>null</code> if the key was not found or was found
     *   but was not an integer
     */
    virtual int GetInteger(const std::string& key) const;

    /**
     * Returns the string value of the given key.
     *
     * @param key the key
     * @return the value, or <code>null</code> if the key was not found
     */
    virtual const std::string& GetString(const std::string& key) const;

    /**
	 * Returns the boolean value of the given key.
	 * 
	 * @param key the key
	 * @return the value, or <code>null</code> if the key was not found
     * @since 3.4
	 */
	  virtual bool GetBoolean(const std::string& key) const;

	/**
     * Returns the data of the Text node of the memento. Each memento is allowed
     * only one Text node.
     * 
     * @return the data of the Text node of the memento, or <code>null</code>
     * if the memento has no Text node.
     * @since 2.0
     */
    virtual const std::string& GetTextData() const;
	
    /**
     * Returns an array of all the attribute keys of the memento. This will not
     * be <code>null</code>. If there are no keys, an array of length zero will
     * be returned. 
     * @return an array with all the attribute keys of the memento
     * @since 3.4
     */
    virtual std::vector< std::string > GetAttributeKeys() const;


      /* (non-Javadoc)
      * Method declared in IMemento.
      */
      virtual void PutFloat(const std::string& key, float value);

     /* (non-Javadoc)
     * Method declared in IMemento.
     */
     virtual void PutInteger(const std::string& key, int value);

     /* (non-Javadoc)
     * Method declared in IMemento.
     */
     virtual void PutMemento(IMemento::Pointer memento);

     /* (non-Javadoc)
     * Method declared in IMemento.
     */
     virtual void PutString(const std::string& key, const std::string& value);

     /* (non-Javadoc)
     * Method declared in IMemento.
     */
     virtual void PutBoolean(const std::string& key, bool value);

     /* (non-Javadoc)
     * Method declared in IMemento.
     */
     virtual void PutTextData(const std::string& data);

      /**
      * Saves this memento's document current values to the
      * specified writer.
      *
      * @param writer the writer used to save the memento's document
      * @throws IOException if there is a problem serializing the document to the stream.
      */

     //TODO: throw(IOException)
      //void Save(Writer writer) throw(IOException);
     void Save(XMLByteOutputStream& writer);

     virtual Poco::XML::Element* GetElement() const;

private:         
  /**
  * Returns the Text node of the memento. Each memento is allowed only
  * one Text node.
  *
  * @return the Text node of the memento, or <code>null</code> if
  * the memento has no Text node.
  */
  Poco::XML::Text* GetTextNode() const; 

  /**
  * Places the element's attributes into the document.
  * @param copyText true if the first text node should be copied
  */
  void PutElement(Poco::XML::Element* element, bool copyText);


  Poco::XML::Document* factory;
  Poco::XML::Element* element;
};
}//namespace cherry
#endif /* CHERRYXMLMEMENTO_H_ */
