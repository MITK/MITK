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

#ifndef CHERRYIWORKBENCHPARTREFERENCE_H_
#define CHERRYIWORKBENCHPARTREFERENCE_H_

#include <org.opencherry.osgi/cherryMacros.h>

namespace cherry {

class IWorkbenchPart;
class IWorkbenchPage;

/**
 * Implements a reference to a IWorkbenchPart.
 * The IWorkbenchPart will not be instanciated until the part 
 * becomes visible or the API getPart is sent with true;
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 */
struct CHERRY_API IWorkbenchPartReference : public Object {
  
  cherryClassMacro(IWorkbenchPartReference);
  
    /**
     * Returns the IWorkbenchPart referenced by this object.
     * Returns <code>null</code> if the editors was not instantiated or
     * it failed to be restored. Tries to restore the editor
     * if <code>restore</code> is true.
     */
    virtual SmartPointer<IWorkbenchPart> GetPart(bool restore) = 0;

    /**
     * @see IWorkbenchPart#getTitle
     */
    virtual std::string GetTitle() = 0;

    /**
     * @see IWorkbenchPart#getTitleImage
     */
    virtual void* GetTitleImage() = 0;

    /**
     * @see IWorkbenchPart#getTitleToolTip
     */
    virtual std::string GetTitleToolTip() = 0;

    /**
     * @see IWorkbenchPartSite#getId
     */
    virtual std::string GetId();

    /**
     * @see IWorkbenchPart#addPropertyListener
     */
    //virtual void AddPropertyListener(IPropertyListener listener) = 0;

    /**
     * @see IWorkbenchPart#removePropertyListener
     */
    //virtual void removePropertyListener(IPropertyListener listener) = 0;

    /**
     * Returns the workbench page that contains this part
     */
    virtual SmartPointer<IWorkbenchPage> GetPage() = 0;

    /**
     * Returns the name of the part, as it should be shown in tabs.
     * 
     * @return the part name
     * 
     * @since 3.0
     */
    virtual std::string GetPartName() = 0;

    /**
     * Returns the content description for the part (or the empty string if none)
     * 
     * @return the content description for the part
     * 
     * @since 3.0  
     */
    virtual std::string GetContentDescription() = 0;
    

    /**
     * Returns whether the part is dirty (i.e. has unsaved changes).
     * 
     * @return <code>true</code> if the part is dirty, <code>false</code> otherwise
     * 
     * @since 3.2 (previously existed on IEditorReference)
     */
    virtual bool IsDirty() = 0;
    
    /**
   * Return an arbitrary property from the reference. If the part has been
   * instantiated, it just delegates to the part. If not, then it looks in its
   * own cache of properties. If the property is not available or the part has
   * never been instantiated, it can return <code>null</code>.
   * 
   * @param key
   *            The property to return. Must not be <code>null</code>.
   * @return The String property, or <code>null</code>.
   * @since 3.3
   */
    //virtual std::string GetPartProperty(const std::string& key) = 0;
    
    /**
   * Add a listener for changes in the arbitrary properties set.
   * 
   * @param listener
   *            Must not be <code>null</code>.
   * @since 3.3
   */
    //virtual void addPartPropertyListener(IPropertyChangeListener listener) = 0;
    
    /**
   * Remove a listener for changes in the arbitrary properties set.
   * 
   * @param listener
   *            Must not be <code>null</code>.
   * @since 3.3
   */
    //virtual void removePartPropertyListener(IPropertyChangeListener listener) = 0;
};

}  // namespace cherry

#endif /*CHERRYIWORKBENCHPARTREFERENCE_H_*/
