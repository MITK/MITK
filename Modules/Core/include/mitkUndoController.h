/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkUndoController_h
#define mitkUndoController_h

#include <mitkOperationEvent.h>
#include <mitkUndoModel.h>
#include <MitkCoreExports.h>
#include <map>

namespace mitk
{
  /** \brief Default maximum number of operations kept on the undo/redo stack.
   *
   * Used by UndoController when no \c UndoLimit preference is stored, and by the
   * Undo/Redo UI as the value to fall back to when (re-)enabling a limited stack.
   * A value of 0 means "unlimited"; this default is a finite cap.
   */
  inline constexpr unsigned int DEFAULT_UNDO_REDO_LIMIT = 50;

  /**
   * \brief Controller that manages undo/redo operations by delegating to an UndoModel.
   *
   * The UndoController selects and delegates to an UndoModel implementation
   * (e.g. LimitedLinearUndo or VerboseLimitedLinearUndo). Multiple UndoModel
   * types can be registered and switched at runtime.
   *
   * \ingroup Undo
   */
  class MITKCORE_EXPORT UndoController
  {
  public:
    /** \brief Enumeration of available undo model types. */
    enum UndoType
    {
      LIMITEDLINEARUNDO = 10,
      VERBOSE_LIMITEDLINEARUNDO = 11,
      TREEUNDO = 20
    };

    typedef std::map<UndoType, UndoModel::Pointer> UndoModelMap;
    typedef std::map<UndoType, UndoModel::Pointer>::iterator UndoModelMapIter;

    /** \brief Default UndoModel to use. */
    static const UndoType DEFAULTUNDOMODEL;

    /**
     * \brief Construct an UndoController.
     *
     * Adds the specified UndoType to the model list if it does not exist,
     * or switches to it if it already exists.
     *
     * \param[in] undoType The type of undo model to use. Defaults to DEFAULTUNDOMODEL.
     */
    UndoController(UndoType undoType = DEFAULTUNDOMODEL);
    virtual ~UndoController();

    /**
     * \brief Store an operation event in the current undo model.
     *
     * \param[in] operationEvent The undo stack item to store.
     * \return True if the operation was stored successfully.
     */
    bool SetOperationEvent(UndoStackItem *operationEvent);

    /**
     * \brief Undo the last change (fine-grained, by ObjectEventId).
     * \return True if the undo was performed successfully.
     */
    bool Undo();

    /**
     * \brief Undo the last change with control over granularity.
     *
     * The undo mechanism can undo in two ways:
     * - Fine undo: undo all operations with the same ObjectEventId
     * - Coarse undo: undo all operations with the same GroupEventId
     *
     * \param[in] fine If true, undo by ObjectEventId; if false, undo by GroupEventId.
     * \return True if the undo was performed successfully.
     */
    bool Undo(bool fine);

    /**
     * \brief Redo the last undone change (fine-grained, by ObjectEventId).
     * \return True if the redo was performed successfully.
     */
    bool Redo();

    /**
     * \brief Redo the last undone change with control over granularity.
     *
     * \param[in] fine If true, redo by ObjectEventId; if false, redo by GroupEventId.
     * \return True if the redo was performed successfully.
     */
    bool Redo(bool fine);

    /** \brief Clear both the undo and redo lists. */
    void Clear();

    /** \brief Clear the redo list only. */
    void ClearRedoList();

    /**
     * \brief Check whether the redo list is empty.
     * \return True if the redo list contains no entries.
     */
    bool RedoListEmpty();

    /**
     * \brief Switch to a different undo model type.
     *
     * \param[in] undoType The undo model type to switch to.
     * \return True if the switch was successful, false if the type is not registered.
     */
    bool SwitchUndoModel(UndoType undoType);

    /**
     * \brief Add a new undo model type and switch to it.
     *
     * If the type already exists, nothing is done and false is returned.
     *
     * \param[in] undoType The undo model type to add.
     * \return True if the model was added successfully.
     */
    bool AddUndoModel(UndoType undoType);

    /**
     * \brief Remove an undo model type from the list.
     *
     * If the removed type is the currently active one, the controller switches
     * to the default model or the first available model. The last remaining
     * model cannot be removed.
     *
     * \param[in] undoType The undo model type to remove.
     * \return True if the model was removed successfully.
     */
    bool RemoveUndoModel(UndoType undoType);

    /**
     * \brief Return the ObjectEventId of the top element in the undo history.
     * \return The ObjectEventId of the most recent undo stack entry.
     */
    int GetLastObjectEventIdInList();

    /**
     * \brief Return the GroupEventId of the top element in the undo history.
     * \return The GroupEventId of the most recent undo stack entry.
     */
    int GetLastGroupEventIdInList();

    /**
     * \brief Return the last OperationEvent matching the given destination and operation type.
     *
     * \param[in] destination The target OperationActor.
     * \param[in] opType The operation type to search for.
     * \return The matching OperationEvent, or nullptr if not found.
     */
    OperationEvent *GetLastOfType(OperationActor *destination, OperationType opType);

    /**
     * \brief Return the currently active UndoModel.
     *
     * Can be used to access model-specific functions, such as retrieving
     * text descriptions of the undo/redo stack from VerboseLimitedLinearUndo.
     *
     * \return Pointer to the current UndoModel.
     */
    static UndoModel *GetCurrentUndoModel();

  private:
    /** \brief Currently selected UndoModel. */
    static UndoModel::Pointer m_CurUndoModel;
    /** \brief Currently selected UndoType of m_CurUndoModel. */
    static UndoType m_CurUndoType;
    /** \brief Map of registered UndoModels available for selection. */
    static UndoModelMap m_UndoModelList;
  };
} // namespace mitk

#endif
