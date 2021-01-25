/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYSAVEABLE_H_
#define BERRYSAVEABLE_H_

#include <berryMacros.h>
#include <berryObject.h>
#include <berryIAdaptable.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

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
 */
class BERRY_UI_QT Saveable : /*public InternalSaveable*/ public virtual Object, public IAdaptable
{

public:

  berryObjectMacro(Saveable);

  typedef QSet<Saveable::Pointer> Set;

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
  virtual QString GetName() const = 0;

  /**
   * Returns the tool tip text for this saveable. This text is used to
   * differentiate between two inputs with the same name. For instance,
   * MyClass.java in folder X and MyClass.java in folder Y. The format of the
   * text varies between input types.
   *
   * @return the tool tip text; never <code>null</code>
   */
  virtual  QString GetToolTipText() const = 0;

  /**
   * Returns the image descriptor for this saveable.
   *
   * @return the image descriptor for this model; may be <code>null</code>
   *         if there is no image
   */
  virtual QIcon GetImageDescriptor() const = 0;

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
   * \c Object.equals(Object) and \c Object.hashCode() . Two
   * saveables should be equal if their dirty state is shared, and saving one
   * will save the other. If two saveables are equal, their names, tooltips,
   * and images should be the same because only one of them will be shown when
   * prompting the user to save.
   *
   * @param object
   * @return true if this Saveable is equal to the given object
   */
  bool operator<(const Object* object) const override = 0;

  /**
   * Clients must implement equals and hashCode as defined in
   * \c Object.equals(Object) and \c Object.hashCode() . Two
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
  uint HashCode() const override = 0;

  /**
   * Disables the UI of the given parts containing this saveable if necessary.
   * This method is not intended to be called by clients. A corresponding call
   * to
   * <p>
   * Saveables that can be saved in the background should ensure that the user
   * cannot make changes to their data from the UI, for example by disabling
   * controls, unless they are prepared to handle this case. This method is
   * called on the UI thread after a job runnable has been returned from
   * <code>doSave(IProgressMonitor, IShellProvider)</code> and before
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
  virtual void DisableUI(const QList<SmartPointer<IWorkbenchPart> >& parts, bool closing);

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
  virtual void EnableUI(QList<SmartPointer<IWorkbenchPart> >& parts);

protected:

  /**
   * This implementation of <code>IAdaptable.GetAdapterImpl(const std::type_info&)</code> returns
   * <code>null</code>. Subclasses may override. This allows two unrelated
   * subclasses of Saveable to implement \c equals(Object) and
   * \c hashCode() based on an underlying implementation class that is
   * shared by both Saveable subclasses.
   *
   * @since 3.3
   */
  Object* GetAdapter(const QString& adapter) const override;
};

}


#endif /* BERRYSAVEABLE_H_ */
