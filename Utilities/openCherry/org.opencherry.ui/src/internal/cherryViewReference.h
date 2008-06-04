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

#ifndef CHERRYVIEWREFERENCE_H_
#define CHERRYVIEWREFERENCE_H_

#include "../cherryIMemento.h"

#include "cherryWorkbenchPartReference.h"
#include "../cherryIViewReference.h"
#include "../cherryPartPane.h"

namespace cherry {

class ViewFactory;
struct IWorkbenchPage;

/**
 * \ingroup org_opencherry_ui_internal
 * 
 */
class ViewReference : public WorkbenchPartReference, public IViewReference {

  
private: const ViewFactory* factory;

public: std::string secondaryId;

private: IMemento::Pointer memento;

//private: ViewActionBuilder actionBuilder;

public: ViewReference(ViewFactory* factory, const std::string& id,
    const std::string& secondaryId, IMemento::Pointer memento);

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.internal.WorkbenchPartReference#dispose()
   */
protected: virtual void DoDisposePart();

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IWorkbenchPartReference#getPage()
   */
public: virtual SmartPointer<IWorkbenchPage> GetPage();

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.internal.WorkbenchPartReference#getRegisteredName()
   */
public: virtual std::string GetRegisteredName();

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IViewReference
   */
public: virtual std::string GetSecondaryId();

  /*
   * (non-Javadoc)
   * 
   * @see org.eclipse.ui.IViewReference#getView(boolean)
   */
public: virtual IViewPart::Pointer GetView(bool restore);


  /**
   * Wrapper for restoring the view. First, this delegates to
   * busyRestoreViewHelper to do the real work of restoring the view. If
   * unable to restore the view, this method tries to substitute an error part
   * and return success.
   * 
   * @return the created part
   */
protected: IWorkbenchPart::Pointer CreatePart();

protected: PartPane::Pointer CreatePane();

private: IWorkbenchPart::Pointer CreatePartHelper();

  /**
   * The memento is that last view state saved by the workbench.
   * 
   * @return the last state that was saved by the workbench. It can return
   *         <code>null</code>.
   * @since 3.1.1
   */
public: virtual IMemento::Pointer GetMemento();
};

}  // namespace cherry

#endif /*CHERRYVIEWREFERENCE_H_*/
