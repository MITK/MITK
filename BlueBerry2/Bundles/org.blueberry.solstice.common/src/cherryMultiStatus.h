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


#ifndef CHERRYMULTISTATUS_H_
#define CHERRYMULTISTATUS_H_

#include "cherryStatus.h"

#include "cherryCommonRuntimeDll.h"

namespace cherry {

/**
 * A concrete multi-status implementation,
 * suitable either for instantiating or subclassing.
 * <p>
 * This class can be used without OSGi running.
 * </p>
 */
class CHERRY_COMMON_RUNTIME MultiStatus : public Status {

private:

  /** List of child statuses.
   */
  std::vector<IStatus::Pointer> children;


public:

  /**
   * Creates and returns a new multi-status object with the given children.
   *
   * @param pluginId the unique identifier of the relevant plug-in
   * @param code the plug-in-specific status code
   * @param newChildren the list of children status objects
   * @param message a human-readable message, localized to the
   *    current locale
   * @param exception a low-level exception, or <code>null</code> if not
   *    applicable
   */
  MultiStatus(const std::string& pluginId, int code, const std::vector<IStatus::Pointer>& newChildren, const std::string& message, const std::exception& exception = std::exception());

  /**
   * Creates and returns a new multi-status object with no children.
   *
   * @param pluginId the unique identifier of the relevant plug-in
   * @param code the plug-in-specific status code
   * @param message a human-readable message, localized to the
   *    current locale
   * @param exception a low-level exception, or <code>null</code> if not
   *    applicable
   */
  MultiStatus(const std::string& pluginId, int code, const std::string& message, const std::exception& exception = std::exception());

  /**
   * Adds the given status to this multi-status.
   *
   * @param status the new child status
   */
  void Add(IStatus::Pointer status);

  /**
   * Adds all of the children of the given status to this multi-status.
   * Does nothing if the given status has no children (which includes
   * the case where it is not a multi-status).
   *
   * @param status the status whose children are to be added to this one
   */
  void AddAll(IStatus::Pointer status);

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  std::vector<IStatus::Pointer> GetChildren() const;

  /* (Intentionally not javadoc'd)
   * Implements the corresponding method on <code>IStatus</code>.
   */
  bool IsMultiStatus() const;

  /**
   * Merges the given status into this multi-status.
   * Equivalent to <code>add(status)</code> if the
   * given status is not a multi-status.
   * Equivalent to <code>addAll(status)</code> if the
   * given status is a multi-status.
   *
   * @param status the status to merge into this one
   * @see #add(IStatus)
   * @see #addAll(IStatus)
   */
  void Merge(IStatus::Pointer status);

  /**
   * Returns a string representation of the status, suitable
   * for debugging purposes only.
   */
  std::string ToString() const;

};

}

#endif /* CHERRYMULTISTATUS_H_ */
