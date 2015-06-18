/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) University College London (UCL).
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef QmitkDataStorageComboBoxWithSelectNone_h
#define QmitkDataStorageComboBoxWithSelectNone_h

#include <MitkQtWidgetsExports.h>

#include "QmitkDataStorageComboBox.h"
#include "QmitkCustomVariants.h"
#include "mitkDataNode.h"


/**
 * \class QmitkDataStorageComboBoxWithSelectNone
 * \brief Displays all or a subset (defined by a predicate) of nodes of the Data Storage,
 * and additionally, index 0 is always "please select", indicating no selection, and will
 * hence always return a NULL mitk::DataNode* if asked for the node at index 0.
 *
 * \author Matt Clarkson (m.clarkson@ucl.ac.uk)
 * \ingroup org_mitk_gui_qt_cmdlinemodules_internal
 * \sa QmitkDataStorageComboBox
 */
class MITKQTWIDGETS_EXPORT QmitkDataStorageComboBoxWithSelectNone : public QmitkDataStorageComboBox
{

  Q_OBJECT
  Q_PROPERTY(mitkDataNodePtr SelectedNode READ GetSelectedNode WRITE SetSelectedNode)
  Q_PROPERTY(QString currentValue READ currentValue WRITE setCurrentValue)

  public:
    /**
     * \brief Calls base class constructor.
     * \see QmitkDataStorageComboBox
     */
    QmitkDataStorageComboBoxWithSelectNone(QWidget* parent = nullptr, bool autoSelectNewNodes=false);

    /**
     * \brief Calls base class constructor.
     * \see QmitkDataStorageComboBox
     */
    QmitkDataStorageComboBoxWithSelectNone( mitk::DataStorage* _DataStorage,
                                            const mitk::NodePredicateBase* predicate,
                                            QWidget* parent = nullptr,
                                            bool autoSelectNewNodes=false);

    /**
     * \brief Nothing to do.
     * \see QmitkDataStorageComboBox
     */
    ~QmitkDataStorageComboBoxWithSelectNone();

    /**
     * \brief Stores the string that will be present on index 0, currently equal to "please select".
     */
    static const QString ZERO_ENTRY_STRING;

    /**
     * \brief Searches for a given node, returning the index if found.
     * \param dataNode an mitk::DataNode, can be NULL.
     * \return int -1 if not found, and compared to base class, will add 1 onto the retrieved index.
     */
    virtual int Find( const mitk::DataNode* dataNode ) const override;

    /**
     * \brief Retrieves the node at a given index, where if index is zero, will always return NULL.
     * \param index An integer between 0 and n = number of nodes.
     * \return mitk::DataNode::Pointer NULL or a data node pointer.
     */
    virtual mitk::DataNode::Pointer GetNode(int index) const override;

    /**
     * \brief Returns the selected DataNode or NULL if there is none, or the current index is zero.
     */
    virtual mitk::DataNode::Pointer GetSelectedNode() const override;

    /**
     * \brief Sets the combo box to the index that contains the specified node, or 0 if the node cannot be found.
     */
    virtual void SetSelectedNode(const mitk::DataNode::Pointer& node);

    /**
     * \brief Removes a node from the ComboBox at a specified index (if the index exists).
     * Gets called when a DataStorage Remove Event was thrown.
     */
    virtual void RemoveNode(int index) override;

    /**
     * \brief Set a DataNode in the ComboBox at the specified index (if the index exists).
     * Internally the method just calls InsertNode(unsigned int)
     */
    virtual void SetNode(int index, const mitk::DataNode* dataNode) override;

    /**
     * \brief Get the current file path.
     */
    virtual QString currentValue() const;

    /**
     * \brief Set the current file path.
     */
    virtual void setCurrentValue(const QString& path);

    /**
     * \brief Set the string that will be present on index 0.
     */
    void SetZeroEntryText(const QString& zeroEntryString);

  protected:

    /**
     * \brief Checks if the given index is within range.
     */
    bool HasIndex(unsigned int index) const;

    /**
     * \brief Inserts a new node at the given index, unless index is 0, which is silently ignored.
     */
    virtual void InsertNode(int index, const mitk::DataNode* dataNode) override;

    /**
     * \brief Reset function whenever datastorage or predicate changes.
     */
    virtual void Reset() override;

  private:

    /**
     * \brief This should store the current file path of the current image.
     *
     *
     * The reason is so that we can store and retrieve a temporary file name.
     */
    QString m_CurrentPath;
};

#endif // QmitkDataStorageComboBoxWithSelectNone_h
