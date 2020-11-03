/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHANDLERPERSISTENCE_H
#define BERRYHANDLERPERSISTENCE_H

#include "berryRegistryPersistence.h"

namespace berry {

struct IEvaluationService;
struct IHandlerActivation;
struct IHandlerService;

/**
 * <p>
 * A static class for accessing the registry.
 * </p>
 */
class HandlerPersistence : public RegistryPersistence
{

private:

  /**
   * The index of the command elements in the indexed array.
   *
   * @see HandlerPersistence#read()
   */
  static const int INDEX_COMMAND_DEFINITIONS; // = 0;

  /**
   * The index of the command elements in the indexed array.
   *
   * @see HandlerPersistence#read()
   */
  static const int INDEX_HANDLER_DEFINITIONS; // = 1;

  /**
   * The index of the handler submissions in the indexed array.
   *
   * @see HandlerPersistence#read()
   */
  //static const int INDEX_HANDLER_SUBMISSIONS; // = 2;

  static const int INDEX_SIZE;

  /**
   * The handler activations that have come from the registry. This is used to
   * flush the activations when the registry is re-read. This value is never
   * <code>null</code>
   */
  QList<SmartPointer<IHandlerActivation> > handlerActivations;

  /**
   * The handler service with which this persistence class is associated. This
   * value must not be <code>null</code>.
   */
  IHandlerService* const handlerService;

  IEvaluationService* const evaluationService;

public:

  /**
   * Constructs a new instance of <code>HandlerPersistence</code>.
   *
   * @param handlerService
   *            The handler service with which the handlers should be
   *            registered; must not be <code>null</code>.
   * @param evaluationService
   *            The evaluation service used by handler proxies with enabled
   *            when expressions
   */
  HandlerPersistence(IHandlerService* handlerService,
                     IEvaluationService* evaluationService);

  ~HandlerPersistence() override;

  bool HandlersNeedUpdating(const QList<SmartPointer<IExtension> >& extensions);

  void ReRead();

protected:

  friend class HandlerService;

  bool IsChangeImportant(const QList<SmartPointer<IExtension> >& extensions,
                         RegistryChangeType changeType) override;

  bool IsChangeImportant(const QList<SmartPointer<IExtensionPoint> >& extensionPoints,
                         RegistryChangeType changeType) override;

  /**
   * Reads all of the handlers from the registry
   *
   * @param handlerService
   *            The handler service which should be populated with the values
   *            from the registry; must not be <code>null</code>.
   */
  void Read() override;

private:

  /**
   * Deactivates all of the activations made by this class, and then clears
   * the collection. This should be called before every read.
   *
   * @param handlerService
   *            The service handling the activations; must not be
   *            <code>null</code>.
   */
  void ClearActivations(IHandlerService* handlerService);

  /**
   * Reads the default handlers from an array of command elements from the
   * commands extension point.
   *
   * @param configurationElements
   *            The configuration elements in the commands extension point;
   *            must not be <code>null</code>, but may be empty.
   * @param configurationElementCount
   *            The number of configuration elements that are really in the
   *            array.
   */
  void ReadDefaultHandlersFromRegistry(
      const QList<SmartPointer<IConfigurationElement> >& configurationElements);

  /**
   * Reads all of the handlers from the handlers extension point.
   *
   * @param configurationElements
   *            The configuration elements in the commands extension point;
   *            must not be <code>null</code>, but may be empty.
   * @param configurationElementCount
   *            The number of configuration elements that are really in the
   *            array.
   * @param handlerService
   *            The handler service to which the handlers should be added;
   *            must not be <code>null</code>.
   */
  void ReadHandlersFromRegistry(
      const QList<SmartPointer<IConfigurationElement> >& configurationElements);

};

}

#endif // BERRYHANDLERPERSISTENCE_H
