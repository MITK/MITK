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

#ifndef MITKSIMPELEMEASUREMENT_H
#define MITKSIMPELEMEASUREMENT_H

//#include "QmitkFunctionality.h"

#include <QmitkFunctionality.h>
#include <QmitkStandardViews.h>
#include <QmitkStdMultiWidgetEditor.h>

#include <mitkWeakPointer.h>
#include <mitkPointSetInteractor.h>

#include "ui_QmitkSimpleMeasurementControls.h"

class QmitkStdMultiWidget;
//!mm
//class QmitkSimpleMeasurementControls;
//!

/*!
\brief SimpleMeasurement
Allows to measure distances, angles, etc.

\sa QmitkFunctionality
\ingroup org_mitk_gui_qt_simplemeasurement_internal
*/
class QmitkSimpleMeasurement : public QmitkFunctionality
{
  Q_OBJECT


  public:

  QmitkSimpleMeasurement() {}
  QmitkSimpleMeasurement(const QmitkSimpleMeasurement& other)
  {
    Q_UNUSED(other)
    throw std::runtime_error("Copy constructor not implemented");
  }
    virtual ~QmitkSimpleMeasurement();
    virtual void Activated();
    virtual void Deactivated();
    virtual bool IsExclusiveFunctionality() const;
    ///
    /// Called when the selection in the workbench changed
    ///
    virtual void OnSelectionChanged(std::vector<mitk::DataNode*> nodes);

    virtual void NodeRemoved(const mitk::DataNode* node);

  protected:
    // Pseudo Ctor
    void CreateQtPartControl(QWidget* parent);

  protected slots:

    void AddDistanceSimpleMeasurement();

    void AddAngleSimpleMeasurement();

    void AddPathSimpleMeasurement();

    void Finished();

  protected:

    /*!
    * controls containing sliders for scrolling through the slices
    */
    Ui::QmitkSimpleMeasurementControls * m_Controls;

    /*
     * Interactor for performing the simplemeasurements.
     */
    mitk::WeakPointer<mitk::PointSetInteractor> m_PointSetInteractor;

    /*
    * Interactor for performing the simplemeasurements.
    */
    mitk::WeakPointer<mitk::DataNode> m_SelectedPointSetNode;

    /** @brief
     *  Node representing the PointSets which were created by this application.
     */
    std::vector<mitk::DataNode::Pointer> m_CreatedDistances;
    std::vector<mitk::DataNode::Pointer> m_CreatedAngles;
    std::vector<mitk::DataNode::Pointer> m_CreatedPaths;

    void StartEditingMeasurement();
    void EndEditingMeasurement();
    void UpdateMeasurementList();

};

#endif // !defined(QMITK_MEASUREMENT_H__INCLUDED)
