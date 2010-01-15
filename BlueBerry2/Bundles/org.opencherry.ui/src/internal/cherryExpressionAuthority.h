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


#ifndef CHERRYEXPRESSIONAUTHORITY_H_
#define CHERRYEXPRESSIONAUTHORITY_H_

#include "../cherryISourceProviderListener.h"
#include "../cherryISourceProvider.h"
#include "cherryIEvaluationResultCache.h"

#include <cherryIEvaluationContext.h>


namespace cherry {

/**
 * <p>
 * Provides common functionality for evaluating expressions and listening to
 * {@link ISourceProvider} (i.e., the common event framework for commands).
 * </p>
 * <p>
 * This class is not intended for use outside of the
 * <code>org.opencherry.ui</code> plug-in.
 * </p>
 *
 * @since 3.2
 * @see ISourceProvider
 * @see ISources
 * @see Expression
 * @see IEvaluationContext
 */
class ExpressionAuthority : public ISourceProviderListener {

private:

  /**
   * The evaluation context instance to use when evaluating expression. This
   * context is shared, and so all calls into <code>sourceChanged</code>
   * must happen on the event thread.
   */
  IEvaluationContext::Pointer context;

  /**
   * The current state of this authority. This is a child of the
   * {@link #context} that has been given the selection as the default
   * variable. This value is cleared to <code>null</code> whenever the
   * selection changes.
   */
  IEvaluationContext::Pointer currentState;

  /**
   * The collection of source providers used by this authority. This
   * collection is consulted whenever a contribution is made. This collection
   * only contains instances of <code>ISourceProvider</code>.
   */
  std::vector<ISourceProvider::Pointer> providers;

  /**
   * @param sourcePriority
   * @param strings
   */
  void SourceChanged(int sourcePriority, const std::vector<std::string>& sourceNames);


protected:

  /**
   * Constructs a new instance of <code>ExpressionAuthority</code>.
   */
  ExpressionAuthority();

  /**
   * Returns whether at least one of the <code>IEvaluationResultCache</code>
   * instances in <code>collection</code> evaluates to <code>true</code>.
   *
   * @param collection
   *            The evaluation result caches to check; must not be
   *            <code>null</code>, but may be empty.
   * @return <code>true</code> if there is at least one expression that
   *         evaluates to <code>true</code>; <code>false</code>
   *         otherwise.
   */
  bool Evaluate(const std::vector<IEvaluationResultCache::Pointer>& collection);

  /**
   * Returns whether the <code>IEvaluationResultCache</code> evaluates to
   * <code>true</code>.
   *
   * @param expression
   *            The evaluation result cache to check; must not be
   *            <code>null</code>.
   * @return <code>true</code> if the expression evaluates to
   *         <code>true</code>; <code>false</code> otherwise.
   */
  bool Evaluate(IEvaluationResultCache::Pointer expression);

  /**
   * Returns the variable of the given name.
   *
   * @param name
   *            The name of the variable to get; must not be <code>null</code>.
   * @return The variable of the given name; <code>null</code> if none.
   */
  Object::Pointer GetVariable(const std::string& name);

  /**
   * Changes the variable of the given name. If the <code>value</code> is
   * <code>null</code>, then the variable is removed.
   *
   * @param name
   *            The name of the variable to change; must not be
   *            <code>null</code>.
   * @param value
   *            The new value; the variable should be removed if this is
   *            <code>null</code>.
   */
  void ChangeVariable(const std::string& name, Object::Pointer value);

  /**
   * Carries out the actual source change notification. It assumed that by the
   * time this method is called, <code>getEvaluationContext()</code> is
   * up-to-date with the current state of the application.
   *
   * @param sourcePriority
   *            A bit mask of all the source priorities that have changed.
   */
  virtual void SourceChanged(int sourcePriority) = 0;

  /**
   * Similar to sourceChanged(int) this notifies the subclass about the
   * change, but using the array of source names that changed instead of the
   * priority ... int based.
   * <p>
   * Clients may override this method.
   * </p>
   *
   * @param sourceNames
   *            The array of names that changed.
   * @since 3.3
   */
  virtual void SourceChanged(const std::vector<std::string>& sourceNames);

  /**
   * Updates the evaluation context with the current state from all of the
   * source providers.
   */
  void UpdateCurrentState();

  /**
   * Updates this authority's evaluation context.
   *
   * @param name
   *            The name of the variable to update; must not be
   *            <code>null</code>.
   * @param value
   *            The new value of the variable. If this value is
   *            <code>null</code>, then the variable is removed.
   */
  virtual void UpdateEvaluationContext(const std::string& name, Object::Pointer value);

public:

  /**
   * Adds a source provider to a list of providers to check when updating.
   * This also attaches this authority as a listener to the provider.
   *
   * @param provider
   *            The provider to add; must not be <code>null</code>.
   */
  void AddSourceProvider(ISourceProvider::Pointer provider);

  /**
   * Removes all of the source provider listeners. Subclasses may extend, but
   * must not override.
   */
  virtual void Dispose();

  /**
   * Creates a new evaluation context based on the current evaluation context
   * (i.e., the current state), and places the current selection as the
   * default variable.
   *
   * @return An evaluation context that can be used for evaluating
   *         expressions; never <code>null</code>.
   */
  IEvaluationContext::Pointer GetCurrentState();

  /**
   * Removes this source provider from the list, and detaches this authority
   * as a listener.
   *
   * @param provider
   *            The provider to remove; must not be <code>null</code>.
   */
  void RemoveSourceProvider(ISourceProvider::Pointer provider);

  void SourceChanged(int sourcePriority,
      const std::map<std::string, Object::Pointer>& sourceValuesByName);

  void SourceChanged(int sourcePriority,
      const std::string& sourceName, Object::Pointer sourceValue);

};

}

#endif /* CHERRYEXPRESSIONAUTHORITY_H_ */
