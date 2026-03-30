/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOperation_h
#define mitkOperation_h

#include <MitkCoreExports.h>
#include <itkEventObject.h>

#include <mitkCommon.h>

namespace mitk
{
  /** \brief Type alias for operation type identifiers. */
  typedef int OperationType;

  /**
   * \brief Base class for all operations in the undo/redo framework.
   *
   * An Operation encapsulates a single undoable/redoable action. Each concrete
   * operation subclass stores the data needed to perform and reverse the action.
   * Operations are paired in OperationEvent objects (do-operation and
   * undo-operation).
   *
   * \sa OperationEvent, UndoController, OperationActor
   * \ingroup Undo
   */
  class MITKCORE_EXPORT Operation
  {
  public:
    mitkClassMacroNoParent(Operation)

    /**
     * \brief Check whether this operation is still valid and can be executed.
     *
     * The default implementation always returns true. Subclasses may override
     * to reflect invalidation (e.g. when the target object has been deleted).
     *
     * \return True if the operation can be executed.
     */
    virtual bool IsValid() const;

    /** \brief Virtual destructor. */
    virtual ~Operation() = default;

    /**
     * \brief Get the type identifier of this operation.
     * \return The operation type (values defined in mitkInteractionConst.h).
     */
    OperationType GetOperationType();

  protected:
    /**
     * \brief Construct an operation with the given type.
     * \param[in] operationType The operation type identifier.
     */
    Operation(OperationType operationType);
    Operation(const Operation&) = default;
    Operation(Operation&&) = default;
    Operation& operator=(const Operation&) = default;
    Operation& operator=(Operation&&) = default;

    OperationType m_OperationType; ///< The type identifier for this operation.
  };

  /**
   * \brief ITK event fired after an operation has been executed.
   *
   * Carries a pointer to the Operation that was completed.
   *
   * \sa Operation
   * \ingroup Undo
   */
  class MITKCORE_EXPORT OperationEndEvent : public itk::EndEvent
  {
  public:
    typedef OperationEndEvent Self;
    typedef itk::EndEvent Superclass;

    /**
     * \brief Construct an OperationEndEvent.
     * \param[in] operation The operation that finished (may be nullptr).
     */
    OperationEndEvent(Operation *operation = nullptr) : m_Operation(operation) {}
    /** \brief Destructor. */
    ~OperationEndEvent() override {}
    /** \brief Get the event name. \return "OperationEndEvent". */
    const char *GetEventName() const override { return "OperationEndEvent"; }
    /** \brief Check event type compatibility. \return True if \p e is an OperationEndEvent. */
    bool CheckEvent(const ::itk::EventObject *e) const override { return dynamic_cast<const Self *>(e); }
    /** \brief Clone this event. \return A new OperationEndEvent with the same operation pointer. */
    ::itk::EventObject *MakeObject() const override { return new Self(m_Operation); }
    /** \brief Get the associated operation. \return The Operation pointer. */
    Operation *GetOperation() const { return m_Operation; }
  private:
    Operation *m_Operation;
    OperationEndEvent(const Self &);
    void operator=(const Self &);
  };

} // namespace mitk
#endif
