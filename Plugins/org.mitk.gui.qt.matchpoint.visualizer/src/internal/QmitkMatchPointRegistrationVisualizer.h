/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef __Q_MITK_MATCHPOINT_REGISTRATION_VISUALIZER_H
#define __Q_MITK_MATCHPOINT_REGISTRATION_VISUALIZER_H

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include <QSortFilterProxyModel>

#include "mitkMAPRegistrationWrapper.h"

#include "ui_QmitkMatchPointRegistrationVisualizer.h"


/*!
  \brief QmitkMatchPointRegistrationVisualizer

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
  */
class QmitkMatchPointRegistrationVisualizer : public QmitkAbstractView
{
    // this is needed for all Qt objects that should have a Qt meta-object
    // (everything that derives from QObject and wants to have signal/slots)
    Q_OBJECT

public:

    static const std::string VIEW_ID;

    /**
     * Creates smartpointer typedefs
     */
    berryObjectMacro(QmitkMatchPointRegistrationVisualizer);

        QmitkMatchPointRegistrationVisualizer();

    void CreateQtPartControl(QWidget* parent) override;

    /** Returns the registration currently handled by the plugin.
     * May return nullptr, if no registration is selected/handled by
     * the plugin*/
    mitk::MAPRegistrationWrapper* GetCurrentRegistration();

protected slots:
    /**
     * @brief Connect all GUI elements to its corresponding slots
     */
    virtual void CreateConnections();

    void OnStyleButtonPushed();
    void OnDirectionChanged(int index);

    void OnUpdateBtnPushed();

    void OnColorInterpolationChecked(bool);

    void OnNodeSelectionChanged(QList<mitk::DataNode::Pointer> nodes);

    void TransferFOVRefGeometry();

protected:
    void SetFocus() override;

    void ActualizeRegInfo(mitk::MAPRegistrationWrapper* currentReg);

    Ui::MatchPointRegVisControls* m_Controls;

private:
    QWidget* m_Parent;

    void Error(QString msg);

    void CheckInputs();

    /** Function returns the first data node containing a registration wrapper
     * in the current selection of the data manager. If nothing is selected or
     * no registration wrapper is selected it return nullptr.*/
    mitk::DataNode::Pointer GetSelectedRegNode() const;

    /** If a registration node is set, this function determines the relevant referenced node.
     * The reference node is the node of the target or moving data (depending on the selected directions)
     * used to determine the* registration object.
     * @return Pointer to the reference node (moving or target data of the registration algorithm run).
     * Function may return nullptr if the referenced node cannot be found or is not defined
     * by the registration.*/
    mitk::DataNode::Pointer GetRefNodeOfReg(bool target) const;

    /** Methods returns the first node selected in the data manager that does
     * NOT contain a registration wrapper and has a data pointer with a geometry.
     * Returned pointer may be nullptr if nothing is selected or no appropriate data
     * node is selected.*/
    mitk::DataNode::Pointer GetSelectedDataNode();

    /** Methods checks if the given gridRes is larger than maxGridRes and scales down the spacing if necessary.
     * @param gridRes the resolution of the "grid".
     * @param spacing the spacing that would be used on the grid.
     * @param maxGridRes the maximum resolution of the "grid".*/
    mitk::ScalarType GetSaveSpacing(mitk::ScalarType gridRes, mitk::ScalarType spacing,
        unsigned int maxGridRes) const;

    /**
     * Checks for mandatory node properties and defines them with default values if they are missing.*/
    void InitRegNode();

    /**
     * Updates the gui controls regarding the current state of the instance.*/
    void ConfigureVisualizationControls();

    /**
    Configure the node selectors predicates according to the selected algorithm.
    */
    void ConfigureNodePredicates();

    /**
     * Updates the properties of the selected node according to the states of the gui controls.*/
    void StoreStateInNode();

    /**
     * Updates the state of the gui controls according to the properties of the selected node.*/
    void LoadStateFromNode();

    /**
     * Checks accordung to the current direction if there is a default FOV ref and sets it,
     * if the current FOV ref is not locked.*/
    void CheckAndSetDefaultFOVRef();

    void UpdateOrientationMatrixWidget();

    /**indicates if the gui updates is triggered internally or by user. Needed to
     * avoid update loops by ConfigureVisualizationControlls();*/
    bool m_internalUpdateGuard;

    mitk::DataNode::Pointer m_spSelectedFOVRefNode;
    mitk::DataNode::Pointer m_spSelectedRegNode;

    /**Used to store informations about the FOV reference orientation. Default is identity matrix.*/
    mitk::AffineTransform3D::MatrixType m_FOVRefOrientation;
};

#endif // MatchPoint_h
