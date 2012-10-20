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


#ifndef BERRYXMLMEMENTO_H_
#define BERRYXMLMEMENTO_H_

#include <berryMacros.h>
#include "berryIMemento.h"
#include "berryUIException.h"

#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/SAX/XMLReader.h"


namespace berry {

/**
 * This class represents the default implementation of the
 * <code>IMemento</code> interface.
 * <p>
 * This class is not intended to be extended by clients.
 * </p>
 *
 * @see IMemento
 */
class BERRY_UI_QT XMLMemento : public IMemento
{

  public:
    berryObjectMacro(XMLMemento)

    /**
     * Defines a std::ostream as XML output stream
     */
    typedef std::ostream XMLByteOutputStream;

    /**
     * Defines a std::istream as XML input stream
     */
    typedef std::istream XMLByteInputStream;

    /**
    * Creates a memento for the specified document and element.
    * <p>
    * Clients should use <code>CreateReadRoot</code> and
    * <code>CreateWriteRoot</code> to create the initial
    * memento on a document.
    * </p>
    *
    * @param document the document for the memento
    * @param element the element node for the memento
    */
    XMLMemento(Poco::XML::Document* document, Poco::XML::Element* elem);

    ~XMLMemento();

    /**
     * Creates a <code>Document</code> from the <code>Reader</code>
     * and returns a memento on the first <code>Element</code> for reading
     * the document.
     *
     * @param reader the <code>Reader</code> used to create the memento's document
     * @return a memento on the first <code>Element</code> for reading the document
     * @throws WorkbenchException if IO problems, invalid format, or no element.
    */
    static XMLMemento::Pointer CreateReadRoot(berry::XMLMemento::XMLByteInputStream& reader);

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
    static XMLMemento::Pointer CreateReadRoot(berry::XMLMemento::XMLByteInputStream& reader, const QString& baseDir);

    /**
     * Returns a root memento for writing a document.
     *
     * @param type the element node type to create on the document
     * @return the root memento for writing a document
    */
    static XMLMemento::Pointer CreateWriteRoot(const QString& type);

    /**
     * Copies another Memento into this memento
     *
     * @param child the new child memento
     * @return the new child memento
    */
    IMemento::Pointer CopyChild(IMemento::Pointer child);

    /**
     * Creates a new child of this memento with the given type
     *
     * @param type the type
     * @return a new child memento with the given type
    */
    virtual IMemento::Pointer CreateChild(const QString& type);

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
    virtual IMemento::Pointer CreateChild(const QString& type, const QString& id);

    /**
     * Returns the first child with the given type id.
     *
     * @param type the type id
     * @return the first child with the given type
     */
    virtual IMemento::Pointer GetChild(const QString& type) const;

    /**
     * Returns all children with the given type id.
     *
     * @param type the type id
     * @return an array of children with the given type
     */
    virtual QList< IMemento::Pointer > GetChildren(const QString& type) const;

    /**
     * Returns the Type of this memento
    */
    virtual QString GetType() const;

    /**
     * Returns the ID of this memento
     */
    virtual QString GetID() const;

    /**
     * @see IMemento#GetInteger
     */
    virtual bool GetInteger(const QString& key, int& value) const;

    /**
     * @see IMemento#GetFloat
     */
    virtual bool GetFloat(const QString& key, double& value) const;

    /**
     * @see IMemento#GetString
     */
    virtual bool GetString(const QString& key, QString& value) const;

    /**
     * @see IMemento#GetString
     */
    virtual bool GetBoolean(const QString& key, bool& value) const;

    /**
     * Returns the data of the Text node of the memento. Each memento is allowed
     * only one Text node.
     *
     * @return the data of the Text node of the memento, or <code>null</code>
     * if the memento has no Text node.
     */
    virtual QString GetTextData() const;

    /**
     * Returns an array of all the attribute keys of the memento. This will not
     * be <code>null</code>. If there are no keys, an array of length zero will
     * be returned.
     * @return an vector with all the attribute keys of the memento
     */
    virtual QList< QString > GetAttributeKeys() const;

    /**
     * Puts a float in this memento
     *
     * @param key the key
     * @param value the value
     */
    virtual void PutFloat(const QString& key, double value);

    /**
     * Puts a integer in this memento
     *
     * @param key the key
     * @param value the value
     */
    virtual void PutInteger(const QString& key, int value);

    /**
     * Puts another memento in this memento as a child
     *
     * @param key the key
     * @param value the value
     */
    virtual void PutMemento(IMemento::Pointer memento);

    /**
     * Puts a string in this memento
     *
     * @param key the key
     * @param value the value
     */
    virtual void PutString(const QString& key, const QString& value);

    /**
     * Puts a boolean in this memento
     *
     * @param key the key
     * @param value the value
     */
    virtual void PutBoolean(const QString& key, bool value);

    /**
     * Puts a text in this memento
     *
     * @param data the text
     */
    virtual void PutTextData(const QString& data);

    /**
     * Saves this memento's document current values to the
     * specified writer.
     *
     * @param writer the writer used to save the memento's document
     * @throws IOException if there is a problem serializing the document to the stream.
     */
    void Save(XMLByteOutputStream& writer); //TODO: throw(IOException)

    /**
     * Returns the element of the memento
     *
     * @return the xml element
     */
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
}//namespace berry
#endif /* BERRYXMLMEMENTO_H_ */
