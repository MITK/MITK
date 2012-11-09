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

#ifndef BERRY_EXTENSION_TYPES_H
#define BERRY_EXTENSION_TYPES_H

#include <org_blueberry_osgi_Export.h>

#include <QObject>

namespace berry {

/**
 * \brief The ExtensionType class manages named types
 * \threadsafe
 *
 * The class associates a type
 * name to a type so that it can be created and destructed
 * dynamically at run-time. Call registerExtensionType() to make
 * the type known.
 *
 * Any class or struct that inherits from QObject and has a public
 * default constructor, and a public destructor can be registered.
 *
 */
class BERRY_OSGI ExtensionType
{

public:

  typedef void (*Destructor)(QObject *);
  typedef QObject* (*Constructor)();

  /**
   * \internal
   *
   * Registers a type with typeName, a destructor, and a constructor.
   * Returns the type's handle, or -1 if the type could not be registered.
   */
  static int registerType(const char* typeName, Destructor destructor,
                          Constructor constructor);

  /**
   * \internal
   *
   * Registers a type as an alias of another type (typedef)
   */
  static int registerTypedef(const char* typeName, int aliasId);

  /**
   * Unregisters a type with typeName.
   *
   * \sa type()
   * \sa typeName()
   */
  static void unregisterType(const char* typeName);

  /**
   * Returns a handle to the type called typeName, or 0 if there is
   * no such type.
   *
   * \sa isRegistered()
   * \sa typeName()
   */
  static int type(const char* typeName);

  /**
   * Returns the type name associated with the given type, or 0 if no
   * matching type was found. The returned pointer must not be deleted.
   *
   * \sa type()
   * \sa isRegistered()
   */
  static const char* typeName(int type);

  /**
   * Returns true if the datatype with ID type is registered;
   * otherwise returns false.
   *
   * \sa type()
   * \sa typeName()
   */
  static bool isRegistered(int type);

  /**
   * Creates a default type.
   *
   * \sa destroy()
   * \sa isRegistered()
   */
  static QObject* construct(int type);

  /**
   * Destroys the data, assuming it is of the type given.
   *
   * \sa construct()
   * \sa isRegistered()
   */
  static void destroy(int type, QObject* data);
};

template <typename T>
void extensionTypeDeleteHelper(T* t)
{
  delete t;
}

template <typename T>
QObject* extensionTypeConstructHelper(const T* /*t*/)
{
  return new T;
}

template <typename T>
struct ExtensionTypeId
{
  enum { Defined = 0 };
};

template <typename T>
struct ExtensionTypeId2
{
  enum { Defined = ExtensionTypeId<T>::Defined };
  static inline int extensiontype_id() { return ExtensionTypeId<T>::extensiontype_id(); }
};

namespace internal {

template <typename T, bool Defined = ExtensionTypeId2<T>::Defined>
struct ExtensionTypeIdHelper
{
  static inline int extensiontype_id()
  { return ExtensionTypeId2<T>::extensiontype_id(); }
};

template <typename T>
struct ExtensionTypeIdHelper<T, false>
{
  static inline int extensiontype_id()
  { return -1; }
};

} // end namespace internal

/**
 * \threadsafe
 *
 * Registers the type name typeName for the type T. Returns
 * the internal ID used by ExtensionType. Any class or struct that has a
 * public default constructor, a public destructor, and a QObject base
 * class can be registered.
 *
 * After a type has been registered, you can create and destroy
 * objects of that type dynamically at run-time.
 */
template <typename T>
int registerExtensionType(const char* typeName
#ifndef DOXYGEN_SKIP
                          , T* dummy = 0
#endif
)
{
  const int typedefOf = dummy ? -1 : internal::ExtensionTypeIdHelper<T>::extensiontype_id();
  if (typedefOf != -1)
    return ExtensionType::registerTypedef(typeName, typedefOf);

  typedef QObject*(*ConstructPtr)(const T*);
  ConstructPtr cptr = extensionTypeConstructHelper<T>;
  typedef void(*DeletePtr)(T*);
  DeletePtr dptr = extensionTypeDeleteHelper<T>;

  return ExtensionType::registerType(typeName, reinterpret_cast<ExtensionType::Destructor>(dptr),
                                     reinterpret_cast<ExtensionType::Constructor>(cptr));
}

} // end namespace berry


#endif // BERRY_EXTENSION_TYPES_H
