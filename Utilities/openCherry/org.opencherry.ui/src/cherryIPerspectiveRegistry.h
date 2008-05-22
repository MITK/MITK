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

#ifndef CHERRYIPERSPECTIVEREGISTRY_H_
#define CHERRYIPERSPECTIVEREGISTRY_H_

#include "cherryIPerspectiveDescriptor.h"

#include <vector>

namespace cherry {

/**
 * The workbench's global registry of perspectives. 
 * <p>
 * This registry contains a descriptor for each perspectives in the workbench.
 * It is initially populated with stock perspectives from the workbench's 
 * perspective extension point (<code>"org.eclipse.ui.perspectives"</code>) and 
 * with custom perspectives defined by the user.
 * </p><p>
 * This interface is not intended to be implemented by clients.
 * </p>
 * @see IWorkbench#getPerspectiveRegistry
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct IPerspectiveRegistry {
    /**
     * Clones an existing perspective.
     * 
     * @param id the id for the cloned perspective, which must not already be used by 
     *   any registered perspective
     * @param label the label assigned to the cloned perspective
     * @param desc the perspective to clone
     * @return the cloned perspective descriptor
     * @throws IllegalArgumentException if there is already a perspective with the given id 
     * 
     * @since 3.0
     */
    virtual IPerspectiveDescriptor::Pointer ClonePerspective(const std::string& id, const std::string& label,
            IPerspectiveDescriptor::Pointer desc) = 0;

  /**
   * Deletes a perspective. Has no effect if the perspective is defined in an
   * extension.
   * 
   * @param persp the perspective to delete
   * @since 3.2
   */
  virtual void DeletePerspective(IPerspectiveDescriptor::Pointer persp) = 0;
    
    /**
     * Finds and returns the registered perspective with the given perspective id.
     *
     * @param perspectiveId the perspective id 
     * @return the perspective, or <code>null</code> if none
     * @see IPerspectiveDescriptor#getId
     */
    virtual IPerspectiveDescriptor::Pointer FindPerspectiveWithId(const std::string& perspectiveId) = 0;

    /**
     * Finds and returns the registered perspective with the given label.
     *
     * @param label the label
     * @return the perspective, or <code>null</code> if none
     * @see IPerspectiveDescriptor#getLabel
     */
    virtual IPerspectiveDescriptor::Pointer FindPerspectiveWithLabel(const std::string& label) = 0;

    /**
     * Returns the id of the default perspective for the workbench.  This identifies one
     * perspective extension within the workbench's perspective registry.
     * <p>
     * Returns <code>null</code> if there is no default perspective.
     * </p> 
     *
     * @return the default perspective id, or <code>null</code>
     */
    virtual std::string GetDefaultPerspective() = 0;

    /**
     * Returns a list of the perspectives known to the workbench.
     *
     * @return a list of perspectives
     */
    virtual std::vector<IPerspectiveDescriptor::Pointer> GetPerspectives() = 0;

    /**
     * Sets the default perspective for the workbench to the given perspective id.
     * If non-<code>null</code>, the id must correspond to a perspective extension 
     * within the workbench's perspective registry.
     * <p>
     * A <code>null</code> id indicates no default perspective.
     * </p>
     *
     * @param id a perspective id, or <code>null</code>
     */
    virtual void SetDefaultPerspective(const std::string& id) = 0;

    /**
     * Reverts a perspective back to its original definition
     * as specified in the plug-in manifest.
     * 
     * @param perspToRevert the perspective to revert  
     * 
     * @since 3.0
     */
    virtual void RevertPerspective(IPerspectiveDescriptor::Pointer perspToRevert) = 0;
};

}

#endif /*CHERRYIPERSPECTIVEREGISTRY_H_*/
