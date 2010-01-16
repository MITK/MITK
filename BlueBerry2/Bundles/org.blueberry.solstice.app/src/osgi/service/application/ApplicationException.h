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


#ifndef APPLICATIONEXCEPTION_H_
#define APPLICATIONEXCEPTION_H_

namespace osgi {
namespace service {
namespace application {

/**
 * This exception is used to indicate problems related to application lifecycle
 * management.
 *
 * <code>ApplicationException</code> object is created by the Application Admin
 * to denote an exception condition in the lifecycle of an application.
 * <code>ApplicationException</code>s should not be created by developers. <br/>
 * <code>ApplicationException</code>s are associated with an error code. This
 * code describes the type of problem reported in this exception. The possible
 * codes are:
 * <ul>
 * <li> {@link #APPLICATION_LOCKED} - The application couldn't be launched
 * because it is locked.</li>
 * <li> {@link #APPLICATION_NOT_LAUNCHABLE} - The application is not in
 * launchable state.</li>
 * <li> {@link #APPLICATION_INTERNAL_ERROR} - An exception was thrown by the
 * application or its container during launch.</li>
 * <li> {@link #APPLICATION_SCHEDULING_FAILED} - The scheduling of an application
 * failed.
 * <li> {@link #APPLICATION_DUPLICATE_SCHEDULE_ID} - The application scheduling
 * failed because the specified identifier is already in use.
 * <li> {@link #APPLICATION_EXITVALUE_NOT_AVAILABLE} - The exit value is not
 * available for an application instance because the instance has not
 * terminated.
 * <li> {@link #APPLICATION_INVALID_STARTUP_ARGUMENT} - One of the specified
 * startup arguments is invalid, for example its type is not permitted.
 * </ul>
 *
 * @version $Revision$
 */
public class ApplicationException extends Exception {
  private static final long serialVersionUID = -7173190453622508207L;
  private final int errorCode;

  /**
   * The application couldn't be launched because it is locked.
   */
  public static final int APPLICATION_LOCKED  = 0x01;

  /**
   * The application is not in launchable state, it's
   * {@link ApplicationDescriptor#APPLICATION_LAUNCHABLE}
   * attribute is false.
   */
  public static final int APPLICATION_NOT_LAUNCHABLE = 0x02;

  /**
   * An exception was thrown by the application or the corresponding container
   * during launch. The exception is available from <code>getCause()</code>.
   */
  public static final int APPLICATION_INTERNAL_ERROR = 0x03;

    /**
     * The application schedule could not be created due to some internal error
     * (for example, the schedule information couldn't be saved due to some
   * storage error).
     */
    public static final int APPLICATION_SCHEDULING_FAILED = 0x04;

    /**
     * The application scheduling failed because the specified identifier
     * is already in use.
     */
    public static final int APPLICATION_DUPLICATE_SCHEDULE_ID = 0x05;

  /**
   * The exit value is not available for an application instance because the
   * instance has not terminated.
   *
   * @since 1.1
   */
    public static final int APPLICATION_EXITVALUE_NOT_AVAILABLE = 0x06;

  /**
   * One of the specified startup arguments is invalid, for example its
   * type is not permitted.
   *
   * @since 1.1
   */
    public static final int APPLICATION_INVALID_STARTUP_ARGUMENT = 0x07;

  /**
   * Creates an <code>ApplicationException</code> with the specified error code.
   * @param errorCode The code of the error
   */
  public ApplicationException(int errorCode) {
    super();
    this.errorCode = errorCode;
  }

  /**
   * Creates a <code>ApplicationException</code> that wraps another exception.
   *
   * @param errorCode The code of the error
   * @param cause The cause of this exception.
   */
  public ApplicationException(int errorCode, Throwable cause) {
    super(cause);
    this.errorCode = errorCode;
  }

  /**
   * Creates an <code>ApplicationException</code> with the specified error code.
   * @param errorCode The code of the error
   * @param message The associated message
   */
  public ApplicationException(int errorCode, String message) {
    super(message);
    this.errorCode = errorCode;
  }

  /**
   * Creates a <code>ApplicationException</code> that wraps another exception.
   *
   * @param errorCode The code of the error
   * @param message The associated message.
   * @param cause The cause of this exception.
   */
  public ApplicationException(int errorCode, String message, Throwable cause) {
    super(message, cause);
    this.errorCode = errorCode;
  }

  /**
   * Returns the cause of this exception or <code>null</code> if no cause was
   * set.
   *
   * @return The cause of this exception or <code>null</code> if no cause was
   *         set.
   */
  public Throwable getCause() {
    return super.getCause();
  }

  /**
   * Returns the error code associated with this exception.
   *
   * @return The error code of this exception.
   */
  public int getErrorCode() {
    return errorCode;
  }
};

}
}
}

#endif /* APPLICATIONEXCEPTION_H_ */
