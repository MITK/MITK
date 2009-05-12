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


#ifndef CHERRYSAVEABLE_H_
#define CHERRYSAVEABLE_H_

#include <cherryMacros.h>
#include <cherryObject.h>
#include <cherryIAdaptable.h>
#include <set>
#include <vector>

#include "cherryUiDll.h"
#include "cherryImageDescriptor.h"

namespace cherry {

struct IWorkbenchPage;
struct IWorkbenchPart;

/**
 * A <code>Saveable</code> represents a unit of saveability, e.g. an editable
 * subset of the underlying domain model that may contain unsaved changes.
 * Different workbench parts (editors and views) may present the same saveables
 * in different ways. This interface allows the workbench to provide more
 * appropriate handling of operations such as saving and closing workbench
 * parts. For example, if two editors sharing the same saveable with unsaved
 * changes are closed simultaneously, the user is only prompted to save the
 * changes once for the shared saveable, rather than once for each editor.
 * <p>
 * Workbench parts that work in terms of saveables should implement
 * {@link ISaveablesSource}.
 * </p>
 *
 * @see ISaveablesSource
 * @since 3.2
 */
class CHERRY_UI Saveable : /*public InternalSaveable*/ public virtual Object, public IAdaptable
{

public:

  cherryObjectMacro(Saveable);

private:
  //Cursor waitCursor;
  //Cursor originalCursor;

public:

  struct SaveableCmp : public std::binary_function<Saveable::Pointer, Saveable::Pointer, bool>
  {
    bool operator()(const Saveable::Pointer& s1, const Saveable::Pointer& s2) const
    {
      return s1->operator<(s2.GetPointer());
    }
  };

  typedef std::set<Saveable::Pointer, SaveableCmp> Set;

  /**
   * Attempts to show this saveable in the given page and returns
   * <code>true</code> on success. The default implementation does nothing
   * and returns <code>false</code>.
   *
   * @param page
   *            the workbench page in which to show this saveable
   * @return <code>true</code> if this saveable is now visible to the user
   * @since 3.3
   */
  virtual bool Show(SmartPointer<IWorkbenchPage> page);

  /**
   * Returns the name of this saveable for display purposes.
   *
   * @return the model's name; never <code>null</code>.
   */
  virtual std::string GetName() const = 0;

  /**
   * Returns the tool tip text for this saveable. This text is used to
   * differentiate between two inputs with the same name. For instance,
   * MyClass.java in folder X and MyClass.java in folder Y. The format of the
   * text varies between input types.
   *
   * @return the tool tip text; never <code>null</code>
   */
  virtual  std::string GetToolTipText() const = 0;

  /**
   * Returns the image descriptor for this saveable.
   *
   * @return the image descriptor for this model; may be <code>null</code>
   *         if there is no image
   */
  virtual SmartPointer<ImageDescriptor> GetImageDescriptor() const = 0;

  /**
   * Saves the contents of this saveable.
   * <p>
   * If the save is cancelled through user action, or for any other reason,
   * the part should invoke <code>setCancelled</code> on the
   * <code>IProgressMonitor</code> to inform the caller.
   * </p>
   * <p>
   * This method is long-running; progress and cancellation are provided by
   * the given progress monitor.
   * </p>
   *
   * @param monitor
   *            the progress monitor
   * @throws CoreException
   *             if the save fails; it is the caller's responsibility to
   *             report the failure to the user
   */
  virtual void DoSave(/*IProgressMonitor monitor*/) = 0;

  /**
   * Returns whether the contents of this saveable have changed since the last
   * save operation.
   * <p>
   * <b>Note:</b> this method is called frequently, for example by actions to
   * determine their enabled status.
   * </p>
   *
   * @return <code>true</code> if the contents have been modified and need
   *         saving, and <code>false</code> if they have not changed since
   *         the last save
   */
  virtual bool IsDirty() const = 0;

  /**
   * Clients must implement equals and hashCode as defined in
   * {@link Object#equals(Object)} and {@link Object#hashCode()}. Two
   * saveables should be equal if their dirty state is shared, and saving one
   * will save the other. If two saveables are equal, their names, tooltips,
   * and images should be the same because only one of them will be shown when
   * prompting the user to save.
   *
   * @param object
   * @return true if this Saveable is equal to the given object
   */
  virtual bool operator<(const Saveable* object) const = 0;

  /**
   * Clients must implement equals and hashCode as defined in
   * {@link Object#equals(Object)} and {@link Object#hashCode()}. Two
   * saveables should be equal if their dirty state is shared, and saving one
   * will save the other. If two saveables are equal, their hash codes MUST be
   * the same, and their names, tooltips, and images should be the same
   * because only one of them will be shown when prompting the user to save.
   * <p>
   * IMPORTANT: Implementers should ensure that the hashCode returned is
   * sufficiently unique so as not to collide with hashCodes returned by other
   * implementations. It is suggested that the defining plug-in's ID be used
   * as part of the returned hashCode, as in the following example:
   * </p>
   *
   * <pre>
   *     int PRIME = 31;
   *     int hash = ...; // compute the &quot;normal&quot; hash code, e.g. based on some identifier unique within the defining plug-in
   *     return hash * PRIME + MY_PLUGIN_ID.hashCode();
   * </pre>
   *
   * @return a hash code
   */
  //virtual int hashCode();

  /**
   * Saves this saveable, or prepares this saveable for a background save
   * operation. Returns null if this saveable has been successfully saved, or
   * a job runnable that needs to be run to complete the save in the
   * background. This method is called in the UI thread. If this saveable
   * supports saving in the background, it should do only minimal work.
   * However, since the job runnable returned by this method (if any) will not
   * run on the UI thread, this method should copy any state that can only be
   * accessed from the UI thread so that the job runnable will be able to
   * access it.
   * <p>
   * The supplied shell provider can be used from within this method and from
   * within the job runnable for the purpose of parenting dialogs. Care should
   * be taken not to open dialogs gratuitously and only if user input is
   * required for cases where the save cannot otherwise proceed - note that in
   * any given save operation, many saveable objects may be saved at the same
   * time. In particular, errors should be signaled by throwing an exception,
   * or if an error occurs while running the job runnable, an error status
   * should be returned.
   * </p>
   * <p>
   * If the foreground part of the save is cancelled through user action, or
   * for any other reason, the part should invoke <code>setCancelled</code>
   * on the <code>IProgressMonitor</code> to inform the caller. If the
   * background part of the save is cancelled, the job should return a
   * {@link IStatus#CANCEL} status.
   * </p>
   * <p>
   * This method is long-running; progress and cancellation are provided by
   * the given progress monitor.
   * </p>
   * <p>
   * The default implementation of this method calls
   * {@link #doSave(IProgressMonitor)} and returns <code>null</code>.
   * </p>
   *
   * @param monitor
   *            a progress monitor used for reporting progress and
   *            cancellation
   * @param shellProvider
   *            an object that can provide a shell for parenting dialogs
   * @return <code>null</code> if this saveable has been saved successfully,
   *         or a job runnable that needs to be run to complete the save in
   *         the background.
   *
   * @since 3.3
   */
  //TODO Saveable IJobRunnable, IProgressMonitor, and IShellProvider
//  virtual /*IJobRunnable*/void DoSave(/*IProgressMonitor monitor,
//      IShellProvider shellProvider*/);

  /**
   * Disables the UI of the given parts containing this saveable if necessary.
   * This method is not intended to be called by clients. A corresponding call
   * to
   * <p>
   * Saveables that can be saved in the background should ensure that the user
   * cannot make changes to their data from the UI, for example by disabling
   * controls, unless they are prepared to handle this case. This method is
   * called on the UI thread after a job runnable has been returned from
   * {@link #doSave(IProgressMonitor, IShellProvider)} and before
   * spinning the event loop. The <code>closing</code> flag indicates that
   * this saveable is currently being saved in response to closing a workbench
   * part, in which case further changes to this saveable through the UI must
   * be prevented.
   * </p>
   * <p>
   * The default implementation calls setEnabled(false) on the given parts'
   * composites.
   * </p>
   *
   * @param parts
   *            the workbench parts containing this saveable
   * @param closing
   *            a boolean flag indicating whether the save was triggered by a
   *            request to close a workbench part, and all of the given parts
   *            will be closed after the save operation finishes successfully.
   *
   * @since 3.3
   */
  virtual void DisableUI(const std::vector<SmartPointer<IWorkbenchPart> >& parts, bool closing);

  /**
   * Enables the UI of the given parts containing this saveable after a
   * background save operation has finished. This method is not intended to be
   * called by clients.
   * <p>
   * The default implementation calls setEnabled(true) on the given parts'
   * composites.
   * </p>
   *
   * @param parts
   *            the workbench parts containing this saveable
   *
   * @since 3.3
   */
  virtual void EnableUI(std::vector<SmartPointer<IWorkbenchPart> >& parts);

protected:

  /**
   * This implementation of {@link IAdaptable#GetAdapterImpl(const std::type_info&)} returns
   * <code>null</code>. Subclasses may override. This allows two unrelated
   * subclasses of Saveable to implement {@link #equals(Object)} and
   * {@link #hashCode()} based on an underlying implementation class that is
   * shared by both Saveable subclasses.
   *
   * @since 3.3
   */
  virtual void* GetAdapterImpl(const std::type_info& adapter) const;
};

}


#endif /* CHERRYSAVEABLE_H_ */
