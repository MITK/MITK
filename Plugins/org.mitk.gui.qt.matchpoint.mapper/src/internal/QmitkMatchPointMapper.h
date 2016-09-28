/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef __Q_MITK_MATCHPOINT_MAPPER_H
#define __Q_MITK_MATCHPOINT_MAPPER_H

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include <QSortFilterProxyModel>

#include "ui_QmitkMatchPointMapper.h"

#include "QmitkMappingJob.h"

// MatchPoint


/*!
  \brief QmitkMatchPointMapper

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
  */
class QmitkMatchPointMapper : public QmitkAbstractView
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    /**
     * Creates smartpointer typedefs
     */
    berryObjectMacro(QmitkMatchPointMapper)

        QmitkMatchPointMapper();

    virtual void CreateQtPartControl(QWidget *parent);

    protected slots:

    /**
     * @brief Connect all GUI elements to its corresponding slots
     */
    virtual void CreateConnections();

    /// \brief Called when the user clicks the GUI button

    void OnLockRegButtonPushed();
    void OnLockInputButtonPushed();
    void OnLockReferenceButtonPushed();
    void OnManualRefChecked();
    void OnLinkSampleFactorChecked();

    void OnXFactorChanged(double d);

    void OnMapBtnPushed();
    void OnRefineBtnPushed();

    void OnMapJobError(QString err);
    void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);
    void OnMappingInfo(QString info);

protected:
    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
        const QList<mitk::DataNode::Pointer>& nodes);

    Ui::MatchPointMapperControls m_Controls;

private:
    QWidget *m_Parent;

    void MapNStoreImage(QmitkMappingJob* job);

    void Error(QString msg);

    /** Method checks if the currently selected reg node has a direct kernel that
     * can be decomposed in a rotation matrix and a offset. If this is true, true
     * is returned. In all other cases false is returned.*/
    bool IsAbleToRefineGeometry() const;

    /** Method checks if the currently selected input is a binary image (property binary == true).
     * If this is true, true is returned. In all other cases false is returned.*/
    bool IsBinaryInput() const;

    /** Method checks if the currently selected input is a point set.
     * If this is true, true is returned. In all other cases false is returned.*/
    bool IsPointSetInput() const;

    /** Function returns the first data node containing a registration wrapper
     * in the current selection of the data manager. If nothing is selected or
     * no registration wrapper is selected it return NULL.*/
    mitk::DataNode::Pointer GetSelectedRegNode();

    /** Methods returns a list of all nodes selected in the data manager that do
     * NOT contain registration wrappers. The list may be empty if nothing is
     * selected or no appropriate data node is selected.*/
    QList<mitk::DataNode::Pointer> GetSelectedDataNodes();

    /** If a registration node is set, this function determines the auto reference node.
     * The auto reference node is the node of the target data used to determine the
     * registration object or, if the first cannot be determined, the currently selected input
     * node.
     * @return Pointer to the reference node (target data of the registration algorithm run).
     * Function may return NULL if the referenced node cannot be found or is not defined
     * by the registration.*/
    mitk::DataNode::Pointer GetAutoRefNodeByReg();

    /**
     * Checks if appropriated nodes are selected in the data manager. If nodes are selected,
     * they are stored m_spSelectedRegNode, m_spSelectedInputNode and m_spSelectedRefNode.
     * They are also checked for vadility and stored in m_ValidInput,... .
     * It also sets the info lables accordingly.*/
    void CheckInputs();

    /** Methods checks the validity of the currently stored
     * m_spSelectedRegNode, m_spSelectedInputNode and m_spSelectedRefNode.
     * Results are returned via the parameters. Details of the evaluation are
     * printed to the log widget.*/
    void CheckNodesValidity(bool& validReg, bool& validInput, bool& validRef);

    /**
     * Updates the state of mapping control button regarding to selected
     * input, registration and reference.*/
    void ConfigureMappingControls();

    /**
     * Configures the progress bars accoarding to the choosen algorithm.
     */
    void ConfigureProgressInfos();

    /**
     * Used to generate, configure and execute a mapping job regarding the
     * current settings.
     * @param doGeometryRefinement Set true if only a geometry refinement should be done.
     * Set to false if a mapping/resampling of the input should be done.*/
    void SpawnMappingJob(bool doGeometryRefinement = false);

    mitk::DataNode::Pointer m_spSelectedRegNode;
    mitk::DataNode::Pointer m_spSelectedInputNode;
    mitk::DataNode::Pointer m_spSelectedRefNode;

    /** boolean variable to control visibility of GUI elements*/
    bool m_ValidReg;
    /** boolean variable to control visibility of GUI elements*/
    bool m_ValidInput;
    /** boolean variable to control visibility of GUI elements*/
    bool m_ValidRef;

    /** used for the internal logic to indicate of the current settings
     are set for mapping binary images (used by ConfigureMappingControls()).*/
    bool m_preparedForBinaryInput;
};

#endif // MatchPoint_h

