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
#ifndef BERRYIPREFERENCES_H_
#define BERRYIPREFERENCES_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryObject.h"
#include "berryBackingStoreException.h"


namespace berry
{

  /**
   * A node in a hierarchical collection of preference data.
   *
   * <p>
   * This interface allows applications to store and retrieve user and system
   * preference data. This data is stored persistently in an
   * implementation-dependent backing store. Typical implementations include flat
   * files, OS-specific registries, directory servers and SQL databases.
   *
   * <p>
   * For each bundle, there is a separate tree of nodes for each user, and one for
   * system preferences. The precise description of "user" and "system" will vary
   * from one bundle to another. Typical information stored in the user preference
   * tree might include font choice, and color choice for a bundle which interacts
   * with the user via a servlet. Typical information stored in the system
   * preference tree might include installation data, or things like high score
   * information for a game program.
   *
   * <p>
   * Nodes in a preference tree are named in a similar fashion to directories in a
   * hierarchical file system. Every node in a preference tree has a <i>node name
   * </i> (which is not necessarily unique), a unique <i>absolute path name </i>,
   * and a path name <i>relative </i> to each ancestor including itself.
   *
   * <p>
   * The root node has a node name of the empty <code>QString</code> object ("").
   * Every other node has an arbitrary node name, specified at the time it is
   * created. The only restrictions on this name are that it cannot be the empty
   * string, and it cannot contain the slash character ('/').
   *
   * <p>
   * The root node has an absolute path name of <code>"/"</code>. Children of the
   * root node have absolute path names of <code>"/" + </code> <i>&lt;node name&gt;
   * </i>. All other nodes have absolute path names of <i>&lt;parent's absolute
   * path name&gt; </i> <code> + "/" + </code> <i>&lt;node name&gt; </i>. Note that
   * all absolute path names begin with the slash character.
   *
   * <p>
   * A node <i>n </i>'s path name relative to its ancestor <i>a </i> is simply the
   * string that must be appended to <i>a </i>'s absolute path name in order to
   * form <i>n </i>'s absolute path name, with the initial slash character (if
   * present) removed. Note that:
   * <ul>
   * <li>No relative path names begin with the slash character.
   * <li>Every node's path name relative to itself is the empty string.
   * <li>Every node's path name relative to its parent is its node name (except
   * for the root node, which does not have a parent).
   * <li>Every node's path name relative to the root is its absolute path name
   * with the initial slash character removed.
   * </ul>
   *
   * <p>
   * Note finally that:
   * <ul>
   * <li>No path name contains multiple consecutive slash characters.
   * <li>No path name with the exception of the root's absolute path name end in
   * the slash character.
   * <li>Any string that conforms to these two rules is a valid path name.
   * </ul>
   *
   * <p>
   * Each <code>Preference</code> node has zero or more properties associated with
   * it, where a property consists of a name and a value. The bundle writer is
   * free to choose any appropriate names for properties. Their values can be of
   * type <code>QString</code>,<code>long</code>,<code>int</code>,<code>bool</code>,
   * <code>std::vector<char></code>,<code>float</code>, or <code>double</code> but they can
   * always be accessed as if they were <code>QString</code> objects.
   *
   * <p>
   * All node name and property name comparisons are case-sensitive.
   *
   * <p>
   * All of the methods that modify preference data are permitted to operate
   * asynchronously; they may return immediately, and changes will eventually
   * propagate to the persistent backing store, with an implementation-dependent
   * delay. The <code>flush</code> method may be used to synchronously force updates
   * to the backing store.
   *
   * <p>
   * Implementations must automatically attempt to flush to the backing store any
   * pending updates for a bundle's preferences when the bundle is stopped or
   * otherwise ungets the IPreferences Service.
   *
   * <p>
   * The methods in this class may be invoked concurrently by multiple threads in
   * a single Java Virtual Machine (JVM) without the need for external
   * synchronization, and the results will be equivalent to some serial execution.
   * If this class is used concurrently <i>by multiple JVMs </i> that store their
   * preference data in the same backing store, the data store will not be
   * corrupted, but no other guarantees are made concerning the consistency of the
   * preference data.
   *
   *
   * @version $Revision$
   */
  struct org_blueberry_core_runtime_EXPORT IPreferences : virtual public Object
  {
    berryObjectMacro(berry::IPreferences)

    ~IPreferences() override;

    /**
     * Associates the specified value with the specified key in this node.
     *
     * @param key key with which the specified value is to be associated.
     * @param value value to be associated with the specified key.
     * @throws NullPointerException if <code>key</code> or <code>value</code> is
     *         <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     */
    virtual void Put(const QString& key, const QString& value) = 0;

    /**
     * Returns the value associated with the specified <code>key</code> in this
     * node. Returns the specified default if there is no value associated with
     * the <code>key</code>, or the backing store is inaccessible.
     *
     * @param key key whose associated value is to be returned.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the backing store is
     *        inaccessible.
     * @return the value associated with <code>key</code>, or <code>def</code> if
     *         no value is associated with <code>key</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @throws NullPointerException if <code>key</code> is <code>null</code>. (A
     *         <code>null</code> default <i>is </i> permitted.)
     */
    virtual QString Get(const QString& key, const QString& def) const = 0;

    /**
     * Removes the value associated with the specified <code>key</code> in this
     * node, if any.
     *
     * @param key key whose mapping is to be removed from this node.
     * @see #get(const QString&,const QString&)
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     */
    virtual void Remove(const QString& key) = 0;

    /**
     * Removes all of the properties (key-value associations) in this node. This
     * call has no effect on any descendants of this node.
     *
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #remove(const QString&)
     */
    virtual void Clear() = 0;

    /**
     * Associates a <code>QString</code> object representing the specified
     * <code>int</code> value with the specified <code>key</code> in this node. The
     * associated string is the one that would be returned if the <code>int</code>
     * value were passed to <code>Integer.toString(int)</code>. This method is
     * intended for use in conjunction with {@link #getInt}method.
     *
     * <p>
     * Implementor's note: it is <i>not </i> necessary that the property value
     * be represented by a <code>QString</code> object in the backing store. If the
     * backing store supports integer values, it is not unreasonable to use
     * them. This implementation detail is not visible through the
     * <code>IPreferences</code> API, which allows the value to be read as an
     * <code>int</code> (with <code>getInt</code> or a <code>QString</code> (with
     * <code>get</code>) type.
     *
     * @param key key with which the string form of value is to be associated.
     * @param value <code>value</code> whose string form is to be associated with
     *        <code>key</code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #getInt(const QString&,int)
     */
    virtual void PutInt(const QString& key, int value) = 0;

    /**
     * Returns the <code>int</code> value represented by the <code>QString</code>
     * object associated with the specified <code>key</code> in this node. The
     * <code>QString</code> object is converted to an <code>int</code> as by
     * <code>Integer.parseInt(QString)</code>. Returns the specified default if
     * there is no value associated with the <code>key</code>, the backing store
     * is inaccessible, or if <code>Integer.parseInt(QString)</code> would throw a
     * <code>NumberFormatException</code> if the associated <code>value</code> were
     * passed. This method is intended for use in conjunction with the
     * {@link #putInt}method.
     *
     * @param key key whose associated value is to be returned as an
     *        <code>int</code>.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the associated value
     *        cannot be interpreted as an <code>int</code> or the backing store is
     *        inaccessible.
     * @return the <code>int</code> value represented by the <code>QString</code>
     *         object associated with <code>key</code> in this node, or
     *         <code>def</code> if the associated value does not exist or cannot
     *         be interpreted as an <code>int</code> type.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #putInt(const QString&,int)
     * @see #get(const QString&,const QString&)
     */
    virtual int GetInt(const QString& key, int def) const = 0;

    /**
     * Associates a <code>QString</code> object representing the specified
     * <code>long</code> value with the specified <code>key</code> in this node. The
     * associated <code>QString</code> object is the one that would be returned if
     * the <code>long</code> value were passed to <code>Long.toString(long)</code>.
     * This method is intended for use in conjunction with the {@link #getLong}
     * method.
     *
     * <p>
     * Implementor's note: it is <i>not </i> necessary that the <code>value</code>
     * be represented by a <code>QString</code> type in the backing store. If the
     * backing store supports <code>long</code> values, it is not unreasonable to
     * use them. This implementation detail is not visible through the <code>
     * IPreferences</code> API, which allows the value to be read as a
     * <code>long</code> (with <code>getLong</code> or a <code>QString</code> (with
     * <code>get</code>) type.
     *
     * @param key <code>key</code> with which the string form of <code>value</code>
     *        is to be associated.
     * @param value <code>value</code> whose string form is to be associated with
     *        <code>key</code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #getLong(const QString&,long)
     */
    virtual void PutLong(const QString& key, long value) = 0;

    /**
     * Returns the <code>long</code> value represented by the <code>QString</code>
     * object associated with the specified <code>key</code> in this node. The
     * <code>QString</code> object is converted to a <code>long</code> as by
     * <code>Long.parseLong(QString)</code>. Returns the specified default if
     * there is no value associated with the <code>key</code>, the backing store
     * is inaccessible, or if <code>Long.parseLong(QString)</code> would throw a
     * <code>NumberFormatException</code> if the associated <code>value</code> were
     * passed. This method is intended for use in conjunction with the
     * {@link #putLong}method.
     *
     * @param key <code>key</code> whose associated value is to be returned as a
     *        <code>long</code> value.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the associated value
     *        cannot be interpreted as a <code>long</code> type or the backing
     *        store is inaccessible.
     * @return the <code>long</code> value represented by the <code>QString</code>
     *         object associated with <code>key</code> in this node, or
     *         <code>def</code> if the associated value does not exist or cannot
     *         be interpreted as a <code>long</code> type.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #putLong(const QString&,long)
     * @see #get(const QString&,const QString&)
     */
    virtual long GetLong(const QString& key, long def) const = 0;

    /**
     * Associates a <code>QString</code> object representing the specified
     * <code>bool</code> value with the specified key in this node. The
     * associated string is "true" if the value is <code>true</code>, and "false"
     * if it is <code>false</code>. This method is intended for use in
     * conjunction with the {@link #getBool}method.
     *
     * <p>
     * Implementor's note: it is <i>not </i> necessary that the value be
     * represented by a string in the backing store. If the backing store
     * supports <code>bool</code> values, it is not unreasonable to use them.
     * This implementation detail is not visible through the <code>IPreferences
     * </code> API, which allows the value to be read as a <code>bool</code>
     * (with <code>getBool</code>) or a <code>QString</code> (with <code>get</code>)
     * type.
     *
     * @param key <code>key</code> with which the string form of value is to be
     *        associated.
     * @param value value whose string form is to be associated with
     *        <code>key</code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #getBool(const QString&,bool)
     * @see #get(const QString&,const QString&)
     */
    virtual void PutBool(const QString& key, bool value) = 0;

    /**
     * Returns the <code>bool</code> value represented by the <code>QString</code>
     * object associated with the specified <code>key</code> in this node. Valid
     * strings are "true", which represents <code>true</code>, and "false", which
     * represents <code>false</code>. Case is ignored, so, for example, "TRUE"
     * and "False" are also valid. This method is intended for use in
     * conjunction with the {@link #putBool}method.
     *
     * <p>
     * Returns the specified default if there is no value associated with the
     * <code>key</code>, the backing store is inaccessible, or if the associated
     * value is something other than "true" or "false", ignoring case.
     *
     * @param key <code>key</code> whose associated value is to be returned as a
     *        <code>bool</code>.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the associated value
     *        cannot be interpreted as a <code>bool</code> or the backing store
     *        is inaccessible.
     * @return the <code>bool</code> value represented by the <code>std::string</code>
     *         object associated with <code>key</code> in this node, or
     *         <code>null</code> if the associated value does not exist or cannot
     *         be interpreted as a <code>bool</code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #get(const QString&,const QString&)
     * @see #putBool(const QString&,bool)
     */
    virtual bool GetBool(const QString& key, bool def) const = 0;

    /**
     * Associates a <code>QString</code> object representing the specified
     * <code>float</code> value with the specified <code>key</code> in this node.
     * The associated <code>QString</code> object is the one that would be returned
     * if the <code>float</code> value were passed to
     * <code>Float.toString(float)</code>. This method is intended for use in
     * conjunction with the {@link #getFloat}method.
     *
     * <p>
     * Implementor's note: it is <i>not </i> necessary that the value be
     * represented by a string in the backing store. If the backing store
     * supports <code>float</code> values, it is not unreasonable to use them.
     * This implementation detail is not visible through the <code>IPreferences
     * </code> API, which allows the value to be read as a <code>float</code> (with
     * <code>getFloat</code>) or a <code>QString</code> (with <code>get</code>) type.
     *
     * @param key <code>key</code> with which the string form of value is to be
     *        associated.
     * @param value value whose string form is to be associated with
     *        <code>key</code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #getFloat(const QString&,float)
     */
    virtual void PutFloat(const QString& key, float value) = 0;

    /**
     * Returns the float <code>value</code> represented by the <code>QString</code>
     * object associated with the specified <code>key</code> in this node. The
     * <code>QString</code> object is converted to a <code>float</code> value as by
     * <code>Float.parseFloat(QString)</code>. Returns the specified default if
     * there is no value associated with the <code>key</code>, the backing store
     * is inaccessible, or if <code>Float.parseFloat(QString)</code> would throw a
     * <code>NumberFormatException</code> if the associated value were passed.
     * This method is intended for use in conjunction with the {@link #putFloat}
     * method.
     *
     * @param key <code>key</code> whose associated value is to be returned as a
     *        <code>float</code> value.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the associated value
     *        cannot be interpreted as a <code>float</code> type or the backing
     *        store is inaccessible.
     * @return the <code>float</code> value represented by the string associated
     *         with <code>key</code> in this node, or <code>def</code> if the
     *         associated value does not exist or cannot be interpreted as a
     *         <code>float</code> type.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @see #putFloat(const QString&,float)
     * @see #get(const QString&,const QString&)
     */
    virtual float GetFloat(const QString& key, float def) const = 0;

    /**
     * Associates a <code>QString</code> object representing the specified
     * <code>double</code> value with the specified <code>key</code> in this node.
     * The associated <code>QString</code> object is the one that would be returned
     * if the <code>double</code> value were passed to
     * <code>Double.toString(double)</code>. This method is intended for use in
     * conjunction with the {@link #getDouble}method
     *
     * <p>
     * Implementor's note: it is <i>not </i> necessary that the value be
     * represented by a string in the backing store. If the backing store
     * supports <code>double</code> values, it is not unreasonable to use them.
     * This implementation detail is not visible through the <code>IPreferences
     * </code> API, which allows the value to be read as a <code>double</code> (with
     * <code>getDouble</code>) or a <code>QString</code> (with <code>get</code>)
     * type.
     *
     * @param key <code>key</code> with which the string form of value is to be
     *        associated.
     * @param value value whose string form is to be associated with
     *        <code>key</code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #getDouble(const QString&,double)
     */
    virtual void PutDouble(const QString& key, double value) = 0;

    /**
     * Returns the <code>double</code> value represented by the <code>QString</code>
     * object associated with the specified <code>key</code> in this node. The
     * <code>QString</code> object is converted to a <code>double</code> value as by
     * <code>Double.parseDouble(QString)</code>. Returns the specified default if
     * there is no value associated with the <code>key</code>, the backing store
     * is inaccessible, or if <code>Double.parseDouble(QString)</code> would throw
     * a <code>NumberFormatException</code> if the associated value were passed.
     * This method is intended for use in conjunction with the
     * {@link #putDouble}method.
     *
     * @param key <code>key</code> whose associated value is to be returned as a
     *        <code>double</code> value.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the associated value
     *        cannot be interpreted as a <code>double</code> type or the backing
     *        store is inaccessible.
     * @return the <code>double</code> value represented by the <code>QString</code>
     *         object associated with <code>key</code> in this node, or
     *         <code>def</code> if the associated value does not exist or cannot
     *         be interpreted as a <code>double</code> type.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the the {@link #removeNode()}method.
     * @throws NullPointerException if <code>key</code> is <code>null</code>.
     * @see #putDouble(const QString&,double)
     * @see #get(const QString&,const QString&)
     */
    virtual double GetDouble(const QString& key, double def) const = 0;

    /**
     * Associates a <code>QByteArray</code> object representing the specified
     * <code>QByteArray</code> with the specified <code>key</code> in this node. The
     * associated <code>QByteArray</code> object is stored in <i>Base64</i> encoding.
     * This method is intended for use in conjunction with the
     * {@link #getByteArray}method.
     *
     * @param key <code>key</code> with which the string form of <code>value</code>
     *        is to be associated.
     * @param value <code>value</code> whose string form is to be associated with
     *        <code>key</code>.
     * @throws NullPointerException if <code>key</code> or <code>value</code> is
     *         <code>null</code>.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #GetByteArray(const QString&,const QByteArray&)
     * @see #Get(const QString&,const QString&)
     */
    virtual void PutByteArray(const QString& key, const QByteArray& value) = 0;

    /**
     * Returns the <code>QByteArray</code> value represented by the <code>QString</code>
     * object associated with the specified <code>key</code> in this node. Valid
     * <code>QString</code> objects are <i>Base64 </i> encoded binary data, as
     * defined in <a href="http://www.ietf.org/rfc/rfc2045.txt">RFC 2045 </a>,
     * Section 6.8, with one minor change: the string must consist solely of
     * characters from the <i>Base64 Alphabet </i>; no newline characters or
     * extraneous characters are permitted. This method is intended for use in
     * conjunction with the {@link #putByteArray}method.
     *
     * <p>
     * Returns the specified default if there is no value associated with the
     * <code>key</code>, the backing store is inaccessible, or if the associated
     * value is not a valid Base64 encoded byte array (as defined above).
     *
     * @param key <code>key</code> whose associated value is to be returned as a
     *        <code>std::vector<char></code> object.
     * @param def the value to be returned in the event that this node has no
     *        value associated with <code>key</code> or the associated value
     *        cannot be interpreted as a <code>std::vector<char></code> type, or the backing
     *        store is inaccessible.
     * @return the <code>std::vector<char></code> value represented by the <code>QString</code>
     *         object associated with <code>key</code> in this node, or
     *         <code>def</code> if the associated value does not exist or cannot
     *         be interpreted as a <code>std::vector<char></code>.
     * @throws NullPointerException if <code>key</code> is <code>null</code>. (A
     *         <code>null</code> value for <code>def</code> <i>is </i> permitted.)
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #get(const QString&,const QString&)
     * @see #putByteArray(const QString&,std::vector<char>)
     */
    virtual QByteArray GetByteArray(const QString& key, const QByteArray& def) const = 0;

    /**
     * Returns all of the keys that have an associated value in this node. (The
     * returned array will be of size zero if this node has no preferences and
     * not <code>null</code>!)
     *
     * @return an array of the keys that have an associated value in this node.
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     */
    virtual QStringList Keys() const = 0;

    /**
     * Returns the names of the children of this node. (The returned array will
     * be of size zero if this node has no children and not <code>null</code>!)
     *
     * @return the names of the children of this node.
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     */
    virtual QStringList ChildrenNames() const = 0;

    /**
     * Returns the parent of this node, or <code>null</code> if this is the root.
     *
     * @return the parent of this node.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     */
    virtual IPreferences::Pointer Parent() const = 0;

    /**
     * Returns a named <code>IPreferences</code> object (node), creating it and any
     * of its ancestors if they do not already exist. Accepts a relative or
     * absolute pathname. Absolute pathnames (which begin with <code>'/'</code>)
     * are interpreted relative to the root of this node. Relative pathnames
     * (which begin with any character other than <code>'/'</code>) are
     * interpreted relative to this node itself. The empty string (<code>""</code>)
     * is a valid relative pathname, referring to this node itself.
     *
     * <p>
     * If the returned node did not exist prior to this call, this node and any
     * ancestors that were created by this call are not guaranteed to become
     * persistent until the <code>flush</code> method is called on the returned
     * node (or one of its descendants).
     *
     * @param pathName the path name of the <code>IPreferences</code> object to
     *        return.
     * @return the specified <code>IPreferences</code> object.
     * @throws IllegalArgumentException if the path name is invalid.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @throws NullPointerException if path name is <code>null</code>.
     * @see #flush()
     */
    virtual IPreferences::Pointer Node(const QString& pathName) = 0;

    /**
     * Returns true if the named node exists. Accepts a relative or absolute
     * pathname. Absolute pathnames (which begin with <code>'/'</code>) are
     * interpreted relative to the root of this node. Relative pathnames (which
     * begin with any character other than <code>'/'</code>) are interpreted
     * relative to this node itself. The pathname <code>""</code> is valid, and
     * refers to this node itself.
     *
     * <p>
     * If this node (or an ancestor) has already been removed with the
     * {@link #removeNode()}method, it <i>is </i> legal to invoke this method,
     * but only with the pathname <code>""</code>; the invocation will return
     * <code>false</code>. Thus, the idiom <code>p.nodeExists("")</code> may be
     * used to test whether <code>p</code> has been removed.
     *
     * @param pathName the path name of the node whose existence is to be
     *        checked.
     * @return true if the specified node exists.
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method and
     *         <code>pathname</code> is not the empty string (<code>""</code>).
     * @throws IllegalArgumentException if the path name is invalid (i.e., it
     *         contains multiple consecutive slash characters, or ends with a
     *         slash character and is more than one character long).
     */
    virtual bool NodeExists(const QString& pathName) const = 0;

    /**
     * Removes this node and all of its descendants, invalidating any properties
     * contained in the removed nodes. Once a node has been removed, attempting
     * any method other than <code>name()</code>,<code>absolutePath()</code> or
     * <code>nodeExists("")</code> on the corresponding <code>IPreferences</code>
     * instance will fail with an <code>IllegalStateException</code>. (The
     * methods defined on <code>Object</code> can still be invoked on a node after
     * it has been removed; they will not throw <code>IllegalStateException</code>.)
     *
     * <p>
     * The removal is not guaranteed to be persistent until the <code>flush</code>
     * method is called on the parent of this node.
     *
     * @throws IllegalStateException if this node (or an ancestor) has already
     *         been removed with the {@link #removeNode()}method.
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @see #flush()
     */
    virtual void RemoveNode() = 0;

    /**
     * Returns this node's name, relative to its parent.
     *
     * @return this node's name, relative to its parent.
     */
    virtual QString Name() const = 0;

    /**
     * Returns this node's absolute path name. Note that:
     * <ul>
     * <li>Root node - The path name of the root node is <code>"/"</code>.
     * <li>Slash at end - Path names other than that of the root node may not
     * end in slash (<code>'/'</code>).
     * <li>Unusual names -<code>"."</code> and <code>".."</code> have <i>no </i>
     * special significance in path names.
     * <li>Illegal names - The only illegal path names are those that contain
     * multiple consecutive slashes, or that end in slash and are not the root.
     * </ul>
     *
     * @return this node's absolute path name.
     */
    virtual QString AbsolutePath() const = 0;

    /**
     * Forces any changes in the contents of this node and its descendants to
     * the persistent store.
     *
     * <p>
     * Once this method returns successfully, it is safe to assume that all
     * changes made in the subtree rooted at this node prior to the method
     * invocation have become permanent.
     *
     * <p>
     * Implementations are free to flush changes into the persistent store at
     * any time. They do not need to wait for this method to be called.
     *
     * <p>
     * When a flush occurs on a newly created node, it is made persistent, as
     * are any ancestors (and descendants) that have yet to be made persistent.
     * Note however that any properties value changes in ancestors are <i>not
     * </i> guaranteed to be made persistent.
     *
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #sync()
     */
    virtual void Flush() = 0;

    /**
     * Ensures that future reads from this node and its descendants reflect any
     * changes that were committed to the persistent store (from any VM) prior
     * to the <code>sync</code> invocation. As a side-effect, forces any changes
     * in the contents of this node and its descendants to the persistent store,
     * as if the <code>flush</code> method had been invoked on this node.
     *
     * @throws BackingStoreException if this operation cannot be completed due
     *         to a failure in the backing store, or inability to communicate
     *         with it.
     * @throws IllegalStateException if this node (or an ancestor) has been
     *         removed with the {@link #removeNode()}method.
     * @see #flush()
     */
    virtual void Sync() = 0;

    /**
    * Sets a flag to the parameter <code>block</code> which can be used to
    * block berry messages in order to prevent callback functions to be called.
    *
    * @param block boolean to set the flag
    */
    virtual void BlockSignals(bool block) = 0;
  };

}  // namespace berry

#endif /*BERRYIPREFERENCES_H_*/
