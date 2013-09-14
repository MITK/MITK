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

#ifndef BERRYISTATUS_H_
#define BERRYISTATUS_H_

#include <berryObject.h>
#include <berryMacros.h>
#include <berryFlags.h>

#include <org_blueberry_solstice_common_Export.h>

namespace berry
{


/**
 * A status object represents the outcome of an operation.
 * All <code>CoreException</code>s carry a status object to indicate
 * what went wrong. Status objects are also returned by methods needing
 * to provide details of failures (e.g., validation methods).
 * <p>
 * A status carries the following information:
 * <ul>
 * <li> plug-in identifier (required)</li>
 * <li> severity (required)</li>
 * <li> status code (required)</li>
 * <li> message (required) - localized to current locale</li>
 * <li> exception (optional) - for problems stemming from a failure at
 *    a lower level</li>
 * </ul>
 * Some status objects, known as multi-statuses, have other status objects
 * as children.
 * </p>
 * <p>
 * The class <code>Status</code> is the standard public implementation
 * of status objects; the subclass <code>MultiStatus</code> is the
 * implements multi-status objects.
 * </p><p>
 * This interface can be used without OSGi running.
 * </p>
 * @see MultiStatus
 * @see Status
 */
struct BERRY_COMMON_RUNTIME IStatus: public Object
{

  berryInterfaceMacro(IStatus, berry);

  enum Severity
  {
    /** Status severity constant (value 0) indicating this status represents the nominal case.
     * This constant is also used as the status code representing the nominal case.
     */
    OK_TYPE = 0x00,

    /** Status type severity (bit mask, value 1) indicating this status is informational only. */
    INFO_TYPE = 0x01,

    /** Status type severity (bit mask, value 2) indicating this status represents a warning. */
    WARNING_TYPE = 0x02,

    /** Status type severity (bit mask, value 4) indicating this status represents an error. */
    ERROR_TYPE = 0x04,

    /** Status type severity (bit mask, value 8) indicating this status represents a cancelation. */
    CANCEL_TYPE = 0x08
  };

  BERRY_DECLARE_FLAGS(Severities, Severity)

  /**
   * Returns a list of status object immediately contained in this
   * multi-status, or an empty list if this is not a multi-status.
   *
   * @return an array of status objects
   * @see #isMultiStatus()
   */
  virtual std::vector<IStatus::Pointer> GetChildren() const = 0;

  /**
   * Returns the plug-in-specific status code describing the outcome.
   *
   * @return plug-in-specific status code
   */
  virtual int GetCode() const = 0;

  /**
   * Returns the relevant low-level exception, or <code>null</code> if none.
   * For example, when an operation fails because of a network communications
   * failure, this might return the <code>java.io.IOException</code>
   * describing the exact nature of that failure.
   *
   * @return the relevant low-level exception, or <code>null</code> if none
   */
  virtual std::exception GetException() const = 0;

  /**
   * Returns the message describing the outcome.
   * The message is localized to the current locale.
   *
   * @return a localized message
   */
  virtual std::string GetMessage() const = 0;

  /**
   * Returns the unique identifier of the plug-in associated with this status
   * (this is the plug-in that defines the meaning of the status code).
   *
   * @return the unique identifier of the relevant plug-in
   */
  virtual std::string GetPlugin() const = 0;

  /**
   * Returns the severity. The severities are as follows (in
   * descending order):
   * <ul>
   * <li><code>CANCEL_TYPE</code> - cancelation occurred</li>
   * <li><code>ERROR_TYPE</code> - a serious error (most severe)</li>
   * <li><code>WARNING_TYPE</code> - a warning (less severe)</li>
   * <li><code>INFO_TYPE</code> - an informational ("fyi") message (least severe)</li>
   * <li><code>OK_TYPE</code> - everything is just fine</li>
   * </ul>
   * <p>
   * The severity of a multi-status is defined to be the maximum
   * severity of any of its children, or <code>OK</code> if it has
   * no children.
   * </p>
   *
   * @return the severity: one of <code>OK_TYPE</code>, <code>ERROR_TYPE</code>,
   * <code>INFO_TYPE</code>, <code>WARNING_TYPE</code>,  or <code>CANCEL_TYPE</code>
   * @see #matches(int)
   */
  virtual Severity GetSeverity() const = 0;

  /**
   * Returns whether this status is a multi-status.
   * A multi-status describes the outcome of an operation
   * involving multiple operands.
   * <p>
   * The severity of a multi-status is derived from the severities
   * of its children; a multi-status with no children is
   * <code>OK_TYPE</code> by definition.
   * A multi-status carries a plug-in identifier, a status code,
   * a message, and an optional exception. Clients may treat
   * multi-status objects in a multi-status unaware way.
   * </p>
   *
   * @return <code>true</code> for a multi-status,
   *    <code>false</code> otherwise
   * @see #getChildren()
   */
  virtual bool IsMultiStatus() const = 0;

  /**
   * Returns whether this status indicates everything is okay
   * (neither info, warning, nor error).
   *
   * @return <code>true</code> if this status has severity
   *    <code>OK</code>, and <code>false</code> otherwise
   */
  virtual bool IsOK() const = 0;

  /**
   * Returns whether the severity of this status matches the given
   * severity mask. Note that a status with severity <code>OK_TYPE</code>
   * will never match; use <code>isOK</code> instead to detect
   * a status with a severity of <code>OK</code>.
   *
   * @param severityMask a mask formed by bitwise or'ing severity mask
   *    constants (<code>ERROR_TYPE</code>, <code>WARNING_TYPE</code>,
   *    <code>INFO_TYPE</code>, <code>CANCEL_TYPE</code>)
   * @return <code>true</code> if there is at least one match,
   *    <code>false</code> if there are no matches
   * @see #getSeverity()
   * @see #CANCEL_TYPE
   * @see #ERROR_TYPE
   * @see #WARNING_TYPE
   * @see #INFO_TYPE
   */
  virtual bool Matches(const Severities& severityMask) const = 0;
};

}

BERRY_DECLARE_OPERATORS_FOR_FLAGS(berry::IStatus::Severities)

#endif /* BERRYISTATUS_H_ */
