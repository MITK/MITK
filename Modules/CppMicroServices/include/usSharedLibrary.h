/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USSHAREDLIBRARY_H
#define USSHAREDLIBRARY_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>
#include <usSharedData.h>

#include <string>

namespace us {

class SharedLibraryPrivate;

/**
 * \ingroup MicroServicesUtils
 *
 * \brief The SharedLibrary class loads shared libraries at runtime.
 */
class MITKCPPMICROSERVICES_EXPORT SharedLibrary
{
public:

  /** \brief Default constructor. */
  SharedLibrary();

  /** \brief Copy constructor.
   *
   * \param[in] other The SharedLibrary to copy.
   */
  SharedLibrary(const SharedLibrary& other);

  /**
   * \brief Construct a SharedLibrary object using a library search path and
   * a library base name.
   *
   * \param[in] libPath An absolute path containing the shared library
   * \param[in] name The base name of the shared library, without prefix
   *        and suffix.
   */
  SharedLibrary(const std::string& libPath, const std::string& name);

  /**
   * \brief Construct a SharedLibrary object using an absolute file path to
   * the shared library. Using this constructor effectively disables
   * all setters except SetFilePath().
   *
   * \param[in] absoluteFilePath The absolute path to the shared library.
   */
  SharedLibrary(const std::string& absoluteFilePath);

  /**
   * \brief Destroys this object but does not unload the shared library.
   */
  ~SharedLibrary();

  /** \brief Assignment operator.
   *
   * \param[in] other The SharedLibrary to assign from.
   * \return A reference to this SharedLibrary.
   */
  SharedLibrary& operator=(const SharedLibrary& other);

  /**
   * \brief Loads the shared library pointed to by this SharedLibrary object.
   * On POSIX systems dlopen() is called with the RTLD_LAZY and
   * RTLD_LOCAL flags unless the compiler is gcc 4.4.x or older. Then
   * the RTLD_LAZY and RTLD_GLOBAL flags are used to load the shared library
   * to work around RTTI problems across shared library boundaries.
   *
   * \throws std::logic_error If the library is already loaded.
   * \throws std::runtime_error If loading the library failed.
   */
  void Load();

  /**
   * \brief Loads the shared library pointed to by this SharedLibrary object,
   * using the specified flags on POSIX systems.
   *
   * \param[in] flags The POSIX dlopen flags (e.g. RTLD_LAZY, RTLD_GLOBAL).
   * \throws std::logic_error If the library is already loaded.
   * \throws std::runtime_error If loading the library failed.
   */
  void Load(int flags);

  /**
   * \brief Un-loads the shared library pointed to by this SharedLibrary object.
   *
   * \throws std::runtime_error If an error occurred while un-loading the
   *         shared library.
   */
  void Unload();

  /**
   * \brief Sets the base name of the shared library. Does nothing if the shared
   * library is already loaded or the SharedLibrary(const std::string&)
   * constructor was used.
   *
   * \param[in] name The base name of the shared library, without prefix and
   *        suffix.
   */
  void SetName(const std::string& name);

  /**
   * \brief Gets the base name of the shared library.
   * \return The shared libraries base name.
   */
  std::string GetName() const;

  /**
   * \brief Gets the absolute file path for the shared library with base name
   * \c name, using the search path returned by GetLibraryPath().
   *
   * \param[in] name The shared library base name.
   * \return The absolute file path of the shared library.
   */
  std::string GetFilePath(const std::string& name) const;

  /**
   * \brief Sets the absolute file path of this SharedLibrary object.
   * Using this methods with a non-empty \c absoluteFilePath argument
   * effectively disables all other setters.
   *
   * \param[in] absoluteFilePath The new absolute file path of this SharedLibrary
   *        object.
   */
  void SetFilePath(const std::string& absoluteFilePath);

  /**
   * \brief Gets the absolute file path of this SharedLibrary object.
   *
   * \return The absolute file path of the shared library.
   */
  std::string GetFilePath() const;

  /**
   * \brief Sets a new library search path. Does nothing if the shared
   * library is already loaded or the SharedLibrary(const std::string&)
   * constructor was used.
   *
   * \param[in] path The new shared library search path.
   */
  void SetLibraryPath(const std::string& path);

  /**
   * \brief Gets the library search path of this SharedLibrary object.
   *
   * \return The library search path.
   */
  std::string GetLibraryPath() const;

  /**
   * \brief Sets the suffix for shared library names (e.g. .dll or .so). Does nothing if the
   * shared library is already loaded or the SharedLibrary(const std::string&)
   * constructor was used.
   *
   * \param[in] suffix The shared library name suffix.
   */
  void SetSuffix(const std::string& suffix);

  /**
   * \brief Gets the file name suffix of this SharedLibrary object.
   *
   * \return The file name suffix of the shared library.
   */
  std::string GetSuffix() const;

  /**
   * \brief Sets the file name prefix for shared library names (e.g. lib).
   * Does nothing if the shared library is already loaded or the
   * SharedLibrary(const std::string&) constructor was used.
   *
   * \param[in] prefix The shared library name prefix.
   */
  void SetPrefix(const std::string& prefix);

  /**
   * \brief Gets the file name prefix of this SharedLibrary object.
   *
   * \return The file name prefix of the shared library.
   */
  std::string GetPrefix() const;

  /**
   * \brief Gets the internal handle of this SharedLibrary object.
   *
   * \return \c nullptr if the shared library is not loaded, the operating
   * system specific handle otherwise.
   */
  void* GetHandle() const;

  /**
   * \brief Gets the loaded/unloaded stated of this SharedLibrary object.
   *
   * \return \c true if the shared library is loaded, \c false otherwise.
   */
  bool IsLoaded() const;

private:

  ExplicitlySharedDataPointer<SharedLibraryPrivate> d;

};

}

#endif // USTESTUTILSHAREDLIBRARY_H
