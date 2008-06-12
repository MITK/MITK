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

#ifndef CHERRYVIEWFACTORY_H_
#define CHERRYVIEWFACTORY_H_

#include <cherrySmartPointer.h>

#include "../cherryIMemento.h"
#include "../cherryIViewReference.h"

#include "cherryReferenceCounter.h"

#include <map>
#include <vector>

namespace cherry {

class WorkbenchPage;
struct IViewRegistry;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 * The ViewFactory is used to control the creation and disposal of views.  
 * It implements a reference counting strategy so that one view can be shared
 * by more than one client.
 */
class ViewFactory { // implements IExtensionChangeHandler {

private: 
  
 std::map<std::string, IMemento::Pointer> mementoTable;
 ReferenceCounter<std::string, IViewReference::Pointer> counter;

    SmartPointer<WorkbenchPage> page;

    IViewRegistry* viewReg;

public:
  
    /**
     * Separates a view's primary id from its secondary id in view key strings.
     */
    static const std::string ID_SEP; // = ":"; //$NON-NLS-1$

    /**
     * Returns a string representing a view with the given id and (optional) secondary id,
     * suitable for use as a key in a map.  
     * 
     * @param id primary id of the view
     * @param secondaryId secondary id of the view or <code>null</code>
     * @return the key
     */
    static std::string GetKey(const std::string& id, const std::string& secondaryId);

    /**
     * Returns a string representing the given view reference, suitable for use as a key in a map.  
     * 
     * @param viewRef the view reference
     * @return the key
     */
    static std::string GetKey(IViewReference::Pointer viewRef);

    /**
     * Extracts ths primary id portion of a compound id.
     * @param compoundId a compound id of the form: primaryId [':' secondaryId]
     * @return the primary id
     */
    static std::string ExtractPrimaryId(const std::string& compoundId);

    /**
     * Extracts ths secondary id portion of a compound id.
     * @param compoundId a compound id of the form: primaryId [':' secondaryId]
     * @return the secondary id, or <code>null</code> if none
     */
    static std::string ExtractSecondaryId(const std::string& compoundId);

    /**
     * Returns whether the given view id contains a wildcard. Wildcards cannot
     * be used in regular view ids, only placeholders.
     * 
     * @param viewId the view id
     * @return <code>true</code> if the given view id contains a wildcard,
     *         <code>false</code> otherwise
     * 
     * @since 3.1
     */
    static bool HasWildcard(const std::string& viewId);
    
    
    /**
     * Constructs a new view factory.
     */
    ViewFactory(SmartPointer<WorkbenchPage> page, IViewRegistry* reg);
    
    
    /**
     * Creates an instance of a view defined by id and secondary id.
     * 
     * This factory implements reference counting.  The first call to this
     * method will return a new view.  Subsequent calls will return the
     * first view with an additional reference count.  The view is
     * disposed when releaseView is called an equal number of times
     * to createView.
     */
    IViewReference::Pointer CreateView(const std::string& id, const std::string& secondaryId = "");
    
    /**
     * Returns the set of views being managed by this factory
     *
     * @return the set of views being managed by this factory
     */
    std::vector<IViewReference::Pointer> GetViewReferences();

    /**
     * Returns the view with the given id, or <code>null</code> if not found.
     */
    IViewReference::Pointer GetView(const std::string& id);

    /**
     * Returns the view with the given id and secondary id, or <code>null</code> if not found.
     */
    IViewReference::Pointer GetView(const std::string& id, const std::string& secondaryId);

    /**
     * @return the <code>IViewRegistry</code> used by this factory.
     * @since 3.0
     */
    const IViewRegistry* GetViewRegistry() const;

    /**
     * Returns a list of views which are open.
     */
    std::vector<IViewReference::Pointer> GetViews();

    /**
     * @return the <code>WorkbenchPage</code> used by this factory.
     * @since 3.0
     */
    SmartPointer<WorkbenchPage> GetWorkbenchPage() const;
    
    int GetReferenceCount(IViewReference::Pointer viewRef);
    
    /**
     * Releases an instance of a view.
     *
     * This factory does reference counting.  For more info see
     * getView.
     */
    void ReleaseView(IViewReference::Pointer viewRef);

    /**
     * Restore view states.
     *  
     * @param memento the <code>IMemento</code> to restore from.
     * @return <code>IStatus</code>
     */
    /*IStatus*/bool RestoreState(IMemento::Pointer memento);

    /**
     * Save view states.
     * 
     * @param memento the <code>IMemento</code> to save to.
     * @return <code>IStatus</code>
     */
    /*IStatus*/bool SaveState(IMemento::Pointer memento);

    //  for dynamic UI
    IMemento::Pointer SaveViewState(IMemento::Pointer memento, IViewReference::Pointer ref /*,MultiStatus res*/);

    // for dynamic UI
    void RestoreViewState(IMemento::Pointer memento);
    
    IMemento::Pointer GetViewState(const std::string& key);

};

}

#endif /*CHERRYVIEWFACTORY_H_*/
