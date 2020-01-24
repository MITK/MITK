/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __Q_MITK_MATCHPOINT_MAPPER_H
#define __Q_MITK_MATCHPOINT_MAPPER_H

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include <QSortFilterProxyModel>

#include "ui_QmitkMatchPointMapper.h"

#include "QmitkMappingJob.h"

/*!
  \brief QmitkMatchPointMapper

  View class that implements the logic/functionality to map point sets or images based on MatchPoint registration objects.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
  */
class QmitkMatchPointMapper : public QmitkAbstractView
{
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    /**
     * Creates smartpointer typedefs
     */
    berryObjectMacro(QmitkMatchPointMapper);

    QmitkMatchPointMapper();

    void CreateQtPartControl(QWidget *parent) override;

protected slots:

    /**
     * @brief Connect all GUI elements to its corresponding slots
     */
    virtual void CreateConnections();

    void OnManualRefChecked();
    void OnLinkSampleFactorChecked();

    void OnXFactorChanged(double d);

    void OnMapBtnPushed();
    void OnRefineBtnPushed();

    void OnMapJobError(QString err);
    void OnMapResultIsAvailable(mitk::BaseData::Pointer spMappedData, const QmitkMappingJob* job);
    void OnMappingInfo(QString info);

    void OnRegNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
    void OnInputNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);
    void OnReferenceNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

protected:
    void SetFocus() override;

    Ui::MatchPointMapperControls m_Controls;

private:
    QWidget *m_Parent;

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

    /** If a registration node is set, this function determines the auto reference node.
     * The auto reference node is the node of the target data used to determine the
     * registration object or, if the first cannot be determined, the currently selected input
     * node.
     * @return Pointer to the reference node (target data of the registration algorithm run).
     * Function may return nullptr if the referenced node cannot be found or is not defined
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
    Configure the reg node selector predicates. If a input pointer is passed,
    the predicate will be configured according to the input.
    */
    void ConfigureRegNodePredicate(const mitk::DataNode* input = nullptr);
    /**
    Configure the input and ref node selector predicates. If a reg pointer is passed,
    the predicate will be configured according to the input.
    */
    void ConfigureNodePredicates(const mitk::DataNode* reg = nullptr);


    /**
     * Used to generate, configure and execute a mapping job regarding the
     * current settings.
     * @param doGeometryRefinement Set true if only a geometry refinement should be done.
     * Set to false if a mapping/resampling of the input should be done.*/
    void SpawnMappingJob(bool doGeometryRefinement = false);

    mitk::DataNode::Pointer m_spSelectedRegNode;
    mitk::DataNode::Pointer m_spSelectedInputNode;
    mitk::DataNode::Pointer m_spSelectedRefNode;

    /** used for the internal logic to indicate of the current settings
     are set for mapping binary images (used by ConfigureMappingControls()).*/
    bool m_preparedForBinaryInput;
};

#endif // MatchPoint_h

