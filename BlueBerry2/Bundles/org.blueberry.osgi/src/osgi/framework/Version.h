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


#ifndef VERSION_H_
#define VERSION_H_

#include "Exceptions.h"

namespace osgi {
namespace framework {

/**
 * Version identifier for bundles and packages.
 *
 * <p>
 * Version identifiers have four components.
 * <ol>
 * <li>Major version. A non-negative integer.</li>
 * <li>Minor version. A non-negative integer.</li>
 * <li>Micro version. A non-negative integer.</li>
 * <li>Qualifier. A text string. See <code>Version(String)</code> for the
 * format of the qualifier string.</li>
 * </ol>
 *
 * <p>
 * <code>Version</code> objects are immutable.
 *
 * @since 1.3
 * @Immutable
 * @version $Revision$
 */

class Version {

private:

  int     major;
  int     minor;
  int     micro;
  std::string    qualifier;

  static const std::string SEPARATOR; //   = ".";


  /**
   * Called by the Version constructors to validate the version components.
   *
   * @throws IllegalArgumentException If the numerical components are negative
   *         or the qualifier string is invalid.
   */
  void Validate() const throw(IllegalArgumentException);

public:

  /**
   * The empty version "0.0.0".
   */
  static const Version emptyVersion;

  /**
   * Creates a version identifier from the specified numerical components.
   *
   * <p>
   * The qualifier is set to the empty string.
   *
   * @param major Major component of the version identifier.
   * @param minor Minor component of the version identifier.
   * @param micro Micro component of the version identifier.
   * @throws IllegalArgumentException If the numerical components are
   *         negative.
   */
  Version(int major, int minor, int micro) throw(IllegalArgumentException);

  /**
   * Creates a version identifier from the specified components.
   *
   * @param major Major component of the version identifier.
   * @param minor Minor component of the version identifier.
   * @param micro Micro component of the version identifier.
   * @param qualifier Qualifier component of the version identifier. If
   *        <code>null</code> is specified, then the qualifier will be set to
   *        the empty string.
   * @throws IllegalArgumentException If the numerical components are negative
   *         or the qualifier string is invalid.
   */
  Version(int major, int minor, int micro, const std::string& qualifier) throw(IllegalArgumentException);

  /**
   * Created a version identifier from the specified string.
   *
   * <p>
   * Here is the grammar for version strings.
   *
   * <pre>
   * version ::= major('.'minor('.'micro('.'qualifier)?)?)?
   * major ::= digit+
   * minor ::= digit+
   * micro ::= digit+
   * qualifier ::= (alpha|digit|'_'|'-')+
   * digit ::= [0..9]
   * alpha ::= [a..zA..Z]
   * </pre>
   *
   * There must be no whitespace in version.
   *
   * @param version String representation of the version identifier.
   * @throws IllegalArgumentException If <code>version</code> is improperly
   *         formatted.
   */
  Version(const std::string& version) throw(IllegalArgumentException);

  /**
   * Create a version identifier from another.
   *
   * @param version Another version identifier
   */
  Version(const Version& version) throw();


  /**
   * Parses a version identifier from the specified string.
   *
   * <p>
   * See <code>Version(String)</code> for the format of the version string.
   *
   * @param version String representation of the version identifier. Leading
   *        and trailing whitespace will be ignored.
   * @return A <code>Version</code> object representing the version
   *         identifier. If <code>version</code> is <code>null</code> or
   *         the empty string then <code>emptyVersion</code> will be
   *         returned.
   * @throws IllegalArgumentException If <code>version</code> is improperly
   *         formatted.
   */
  static Version ParseVersion(const std::string& version) throw(IllegalArgumentException);

  /**
   * Returns the major component of this version identifier.
   *
   * @return The major component.
   */
  int GetMajor() const;

  /**
   * Returns the minor component of this version identifier.
   *
   * @return The minor component.
   */
  int GetMinor() const;

  /**
   * Returns the micro component of this version identifier.
   *
   * @return The micro component.
   */
  int GetMicro() const;

  /**
   * Returns the qualifier component of this version identifier.
   *
   * @return The qualifier component.
   */
  std::string GetQualifier() const;

  /**
   * Returns the string representation of this version identifier.
   *
   * <p>
   * The format of the version string will be <code>major.minor.micro</code>
   * if qualifier is the empty string or
   * <code>major.minor.micro.qualifier</code> otherwise.
   *
   * @return The string representation of this version identifier.
   */
  std::string ToString() const;

  /**
   * Returns a hash code value for the object.
   *
   * @return An integer which is a hash code value for this object.
   */
  std::size_t HashCode() const;

  /**
   * Compares this <code>Version</code> object to another object.
   *
   * <p>
   * A version is considered to be <b>equal to </b> another version if the
   * major, minor and micro components are equal and the qualifier component
   * is equal (using <code>String.equals</code>).
   *
   * @param object The <code>Version</code> object to be compared.
   * @return <code>true</code> if <code>object</code> is a
   *         <code>Version</code> and is equal to this object;
   *         <code>false</code> otherwise.
   */
  bool operator==(const Version& object) const;

  /**
   * Compares this <code>Version</code> object to another object.
   *
   * <p>
   * A version is considered to be <b>less than </b> another version if its
   * major component is less than the other version's major component, or the
   * major components are equal and its minor component is less than the other
   * version's minor component, or the major and minor components are equal
   * and its micro component is less than the other version's micro component,
   * or the major, minor and micro components are equal and it's qualifier
   * component is less than the other version's qualifier component (using
   * <code>std::string::compare</code>).
   *
   * <p>
   * A version is considered to be <b>equal to</b> another version if the
   * major, minor and micro components are equal and the qualifier component
   * is equal (using <code>std::string::compare</code>).
   *
   * @param object The <code>Version</code> object to be compared.
   * @return A negative integer, zero, or a positive integer if this object is
   *         less than, equal to, or greater than the specified
   *         <code>Version</code> object.
   */
  bool operator<(const Version& object) const;

};

}
}

#endif /* VERSION_H_ */
