/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef USMODULEVERSION_H
#define USMODULEVERSION_H

#include <usGlobalConfig.h>
#include <MitkCppMicroServicesExports.h>

#include <string>

namespace us {

/**
 * \ingroup MicroServices
 *
 * \brief Version identifier for CppMicroServices modules.
 *
 * <p>
 * Version identifiers have four components.
 * <ol>
 * <li>Major version. A non-negative integer.</li>
 * <li>Minor version. A non-negative integer.</li>
 * <li>Micro version. A non-negative integer.</li>
 * <li>Qualifier. A text string. See <code>ModuleVersion(const std::string&)</code> for the
 * format of the qualifier string.</li>
 * </ol>
 *
 * <p>
 * <code>ModuleVersion</code> objects are immutable.
 */
class MITKCPPMICROSERVICES_EXPORT ModuleVersion {

private:

  friend class ModulePrivate;

  unsigned int majorVersion;
  unsigned int minorVersion;
  unsigned int microVersion;
  std::string      qualifier;

  static const char SEPARATOR; //  = "."

  bool undefined;


  /**
   * Called by the ModuleVersion constructors to validate the version components.
   *
   * \return <code>true</code> if the validation was successful, <code>false</code> otherwise.
   */
  void Validate();

  ModuleVersion& operator=(const ModuleVersion& v);

  explicit ModuleVersion(bool undefined = false);

public:

  /**
   * \brief The empty version "0.0.0".
   */
  static ModuleVersion EmptyVersion();

  /**
   * \brief Creates an undefined version identifier, representing either
   * infinity or minus infinity.
   */
  static ModuleVersion UndefinedVersion();

  /**
   * \brief Creates a version identifier from the specified numerical components.
   *
   * <p>
   * The qualifier is set to the empty string.
   *
   * \param[in] majorVersion Major component of the version identifier.
   * \param[in] minorVersion Minor component of the version identifier.
   * \param[in] microVersion Micro component of the version identifier.
   *
   */
  ModuleVersion(unsigned int majorVersion, unsigned int minorVersion, unsigned int microVersion);

  /**
   * \brief Creates a version identifier from the specified components.
   *
   * \param[in] majorVersion Major component of the version identifier.
   * \param[in] minorVersion Minor component of the version identifier.
   * \param[in] microVersion Micro component of the version identifier.
   * \param[in] qualifier Qualifier component of the version identifier.
   */
  ModuleVersion(unsigned int majorVersion, unsigned int minorVersion, unsigned int microVersion, const std::string& qualifier);

  /**
   * \brief Created a version identifier from the specified string.
   *
   * <p>
   * Here is the grammar for version strings.
   *
   * <pre>
   * version ::= majorVersion('.'minorVersion('.'microVersion('.'qualifier)?)?)?
   * majorVersion ::= digit+
   * minorVersion ::= digit+
   * microVersion ::= digit+
   * qualifier ::= (alpha|digit|'_'|'-')+
   * digit ::= [0..9]
   * alpha ::= [a..zA..Z]
   * </pre>
   *
   * There must be no whitespace in version.
   *
   * \param[in] version string representation of the version identifier.
   */
  ModuleVersion(const std::string& version);

  /**
   * \brief Create a version identifier from another.
   *
   * \param[in] version Another version identifier
   */
  ModuleVersion(const ModuleVersion& version);


  /**
   * \brief Parses a version identifier from the specified string.
   *
   * <p>
   * See <code>ModuleVersion(const std::string&)</code> for the format of the version string.
   *
   * \param[in] version string representation of the version identifier. Leading
   *        and trailing whitespace will be ignored.
   * \return A <code>ModuleVersion</code> object representing the version
   *         identifier. If <code>version</code> is the empty string
   *         then <code>EmptyVersion</code> will be
   *         returned.
   */
  static ModuleVersion ParseVersion(const std::string& version);

  /**
   * \brief Returns the undefined state of this version identifier.
   *
   * \return <code>true</code> if this version identifier is undefined,
   *         <code>false</code> otherwise.
   */
  bool IsUndefined() const;

  /**
   * \brief Returns the majorVersion component of this version identifier.
   *
   * \return The majorVersion component.
   */
  unsigned int GetMajor() const;

  /**
   * \brief Returns the minorVersion component of this version identifier.
   *
   * \return The minorVersion component.
   */
  unsigned int GetMinor() const;

  /**
   * \brief Returns the microVersion component of this version identifier.
   *
   * \return The microVersion component.
   */
  unsigned int GetMicro() const;

  /**
   * \brief Returns the qualifier component of this version identifier.
   *
   * \return The qualifier component.
   */
  std::string GetQualifier() const;

  /**
   * \brief Returns the string representation of this version identifier.
   *
   * <p>
   * The format of the version string will be <code>majorVersion.minorVersion.microVersion</code>
   * if qualifier is the empty string or
   * <code>majorVersion.minorVersion.microVersion.qualifier</code> otherwise.
   *
   * \return The string representation of this version identifier.
   */
  std::string ToString() const;

  /**
   * \brief Compares this <code>ModuleVersion</code> object to another object.
   *
   * <p>
   * A version is considered to be <b>equal to </b> another version if the
   * majorVersion, minorVersion and microVersion components are equal and the qualifier component
   * is equal.
   *
   * \param[in] object The <code>ModuleVersion</code> object to be compared.
   * \return <code>true</code> if <code>object</code> is a
   *         <code>ModuleVersion</code> and is equal to this object;
   *         <code>false</code> otherwise.
   */
  bool operator==(const ModuleVersion& object) const;

  /**
   * \brief Compares this <code>ModuleVersion</code> object to another object.
   *
   * <p>
   * A version is considered to be <b>less than </b> another version if its
   * majorVersion component is less than the other version's majorVersion component, or the
   * majorVersion components are equal and its minorVersion component is less than the other
   * version's minorVersion component, or the majorVersion and minorVersion components are equal
   * and its microVersion component is less than the other version's microVersion component,
   * or the majorVersion, minorVersion and microVersion components are equal and it's qualifier
   * component is less than the other version's qualifier component (using
   * <code>std::string::operator<()</code>).
   *
   * <p>
   * A version is considered to be <b>equal to</b> another version if the
   * majorVersion, minorVersion and microVersion components are equal and the qualifier component
   * is equal.
   *
   * \param[in] object The <code>ModuleVersion</code> object to be compared.
   * \return A negative integer, zero, or a positive integer if this object is
   *         less than, equal to, or greater than the specified
   *         <code>ModuleVersion</code> object.
   */
  int Compare(const ModuleVersion& object) const;

};

}

/**
 * \ingroup MicroServices
 * \brief Stream output operator for ModuleVersion.
 *
 * \param[in] os The output stream.
 * \param[in] v The module version to write to the stream.
 * \return The output stream.
 */
MITKCPPMICROSERVICES_EXPORT std::ostream& operator<<(std::ostream& os, const us::ModuleVersion& v);

#endif // USMODULEVERSION_H
