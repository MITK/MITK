/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSTATUS_H_
#define BERRYSTATUS_H_

#include "berryIStatus.h"

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

/**
 * A concrete status implementation, suitable either for
 * instantiating or subclassing.
 * <p>
 * This class can be used without OSGi running.
 * </p>
 */
class org_blueberry_core_runtime_EXPORT Status : public virtual IStatus
{

public:

  struct SourceLocation {

    SourceLocation(const QString& fileName = QString(), const QString& methodName = QString(), int lineNumber = 0)
      : fileName(fileName), methodName(methodName), lineNumber(lineNumber)
    {}

    const QString fileName;
    const QString methodName;
    const int lineNumber;
  };

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
  QString pluginId;

  /** Plug-in-specific status code.
   */
  int code;

  /** Message, localized to the current locale.
   */
  QString message;

  /** Wrapped exception, or <code>null</code> if none.
   */
  ctkException* exception;

  /** Constant to avoid generating garbage.
   */
  static const QList<IStatus::Pointer> theEmptyStatusArray;

  SourceLocation sourceLocation;

public:

  /**
   * A standard OK status with an "ok"  message.
   */
  static const IStatus::Pointer OK_STATUS(const SourceLocation& sl);
  /**
   * A standard CANCEL status with no message.
   */
  static const IStatus::Pointer CANCEL_STATUS(const SourceLocation& sl);

  /**
   * Creates a new status object.  The created status has no children.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   * @param pluginId the unique identifier of the relevant plug-in
   * @param code the plug-in-specific status code, or <code>OK</code>
   * @param message a human-readable message, localized to the
   *    current locale
   */
  Status(const Severity& severity, const QString& pluginId, int code, const QString& message,
         const SourceLocation& sl);

  /**
   * Creates a new status object.  The created status has no children.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   * @param pluginId the unique identifier of the relevant plug-in
   * @param code the plug-in-specific status code, or <code>OK</code>
   * @param message a human-readable message, localized to the
   *    current locale
   * @param exception a low-level exception.
   */
  Status(const Severity& severity, const QString& pluginId, int code, const QString& message,
         const ctkException& exc, const SourceLocation& sl);

  /**
   * Simplified constructor of a new status object; assumes that code is <code>OK</code>.
   * The created status has no children.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   * @param pluginId the unique identifier of the relevant plug-in
   * @param message a human-readable message, localized to the
   *    current locale
   */
  Status(const Severity& severity, const QString& pluginId, const QString& message,
         const SourceLocation& sl);

  /**
   * Simplified constructor of a new status object; assumes that code is <code>OK</code>.
   * The created status has no children.
   *
   * @param severity the severity; one of <code>OK</code>, <code>ERROR</code>,
   * <code>INFO</code>, <code>WARNING</code>,  or <code>CANCEL</code>
   * @param pluginId the unique identifier of the relevant plug-in
   * @param message a human-readable message, localized to the
   *    current locale
   * @param exception a low-level exception.
   */
  Status(const Severity& severity, const QString& pluginId, const QString& message,
         const ctkException& exc, const SourceLocation& sl);

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  QList<IStatus::Pointer> GetChildren() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  int GetCode() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  const ctkException* GetException() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  QString GetMessage() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  QString GetPlugin() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  Severity GetSeverity() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool IsMultiStatus() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool IsOK() const override;

  /*
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool Matches(const Severities& severityMask) const override;

  QString GetFileName() const override;

  QString GetMethodName() const override;

  int GetLineNumber() const override;


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
  virtual void SetException(const ctkException& exception);

  /**
   * Sets the message. If null is passed, message is set to an empty
   * string.
   *
   * @param message a human-readable message, localized to the
   *    current locale
   */
  virtual void SetMessage(const QString& message);

  /**
   * Sets the plug-in id.
   *
   * @param pluginId the unique identifier of the relevant plug-in
   */
  virtual void SetPlugin(const QString& pluginId);

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
  QString ToString() const override;

};

}

#define BERRY_STATUS_LOC berry::Status::SourceLocation(__FILE__, __FUNCTION__, __LINE__)

#endif /* BERRYSTATUS_H_ */
