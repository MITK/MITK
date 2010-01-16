/*=========================================================================

 Program:   BlueBerry Platform
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


#ifndef BERRYSTATUS_H_
#define BERRYSTATUS_H_

#include "berryIStatus.h"

#include "berryCommonRuntimeDll.h"

namespace berry {

/**
 * A concrete status implementation, suitable either for
 * instantiating or subclassing.
 * <p>
 * This class can be used without OSGi running.
 * </p>
 */
class BERRY_COMMON_RUNTIME Status : public IStatus {

private:

    /**
   * The severity. One of
   * <ul>
   * <li><code>CANCEL</code></li>
   * <li><code>ERROR</code></li>
   * <li><code>WARNING</code></li>
   * <li><code>INFO</code></li>
   * <li>or <code>OK</code> (0)</li>
   * </ul>
   */
  Severity severity;

  /** Unique identifier of plug-in.
   */
  std::string pluginId;

  /** Plug-in-specific status code.
   */
  int code;

  /** Message, localized to the current locale.
   */
  std::string message;

  /** Wrapped exception, or <code>null</code> if none.
   */
  std::exception exception;

  /** Constant to avoid generating garbage.
   */
  static const std::vector<IStatus::Pointer> theEmptyStatusArray;


public:

  /**
   * A standard OK status with an "ok"  message.
   *
   * @since 3.0
   */
  static const IStatus::Pointer OK_STATUS;
  /**
   * A standard CANCEL status with no message.
   *
   * @since 3.0
   */
  static const IStatus::Pointer CANCEL_STATUS;

  /**
   * Creates a new status object.  The created status has no children.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   * @param pluginId the unique identifier of the relevant plug-in
   * @param code the plug-in-specific status code, or <code>OK</code>
   * @param message a human-readable message, localized to the
   *    current locale
   * @param exception a low-level exception, or <code>null</code> if not
   *    applicable
   */
  Status(const Severity& severity, const std::string& pluginId, int code, const std::string& message, const std::exception& exception = std::exception());

  /**
   * Simplified constructor of a new status object; assumes that code is <code>OK</code>.
   * The created status has no children.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   * @param pluginId the unique identifier of the relevant plug-in
   * @param message a human-readable message, localized to the
   *    current locale
   * @param exception a low-level exception, or <code>null</code> if not
   *    applicable
   *
   * @since org.eclipse.equinox.common 3.3
   */
  Status(const Severity& severity, const std::string& pluginId, const std::string& message, const std::exception& exception = std::exception());

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  std::vector<IStatus::Pointer> GetChildren() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  int GetCode() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  std::exception GetException() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  std::string GetMessage() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  std::string GetPlugin() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  Severity GetSeverity() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool IsMultiStatus() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool IsOK() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool Matches(const Severities& severityMask) const;


protected:

  /**
   * Sets the status code.
   *
   * @param code the plug-in-specific status code, or <code>OK</code>
   */
  virtual void SetCode(int code);

  /**
   * Sets the exception.
   *
   * @param exception a low-level exception, or <code>null</code> if not
   *    applicable
   */
  virtual void SetException(const std::exception& exception);

  /**
   * Sets the message. If null is passed, message is set to an empty
   * string.
   *
   * @param message a human-readable message, localized to the
   *    current locale
   */
  virtual void SetMessage(const std::string& message);

  /**
   * Sets the plug-in id.
   *
   * @param pluginId the unique identifier of the relevant plug-in
   */
  virtual void SetPlugin(const std::string& pluginId);

  /**
   * Sets the severity.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   */
  virtual void SetSeverity(const Severity& severity);


public:

  /**
   * Returns a string representation of the status, suitable
   * for debugging purposes only.
   */
  std::string ToString() const;

};

}

#endif /* BERRYSTATUS_H_ */
