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

#ifndef QmitkRigidRegistrationSelectorViewWidgetHIncluded
#define QmitkRigidRegistrationSelectorViewWidgetHIncluded

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "ui_QmitkRigidRegistrationSelector.h"
#include "qobject.h"
#include <org_mitk_gui_qt_registration_Export.h>
#include "QmitkRigidRegistrationTransformsGUIBase.h"
#include "QmitkRigidRegistrationMetricsGUIBase.h"
#include "QmitkRigidRegistrationOptimizerGUIBase.h"


/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class REGISTRATION_EXPORT QmitkRigidRegistrationSelectorView : public QWidget
{
  Q_OBJECT

public:

  QmitkRigidRegistrationSelectorView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkRigidRegistrationSelectorView();

  signals:
    void TransformChanged();
    void OptimizerChanged(double value);
    void AddNewTransformationToUndoList();

  public slots:
    void SetFixedNode( mitk::DataNode * fixedNode );
    void SetFixedMaskNode(mitk::DataNode * fixedMaskNode );
    void SetFixedDimension( int dimension );
    void SetMovingNode( mitk::DataNode * movingNode );
    void SetMovingNodeChildren(mitk::DataStorage::SetOfObjects::ConstPointer children);
    void SetMovingMaskNode(mitk::DataNode * movingMaskNode );
    void SetMovingDimension(int dimension );
    int GetSelectedTransform();
    void CalculateTransformation(unsigned int timestep = 0);
    void StopOptimization(bool stopOptimization);

  protected slots:
    // this is a callback function that retrieves the current transformation
    // parameters after every step of progress in the optimizer.
    // depending on the choosen transformation, we construct a vtktransform
    // that will be applied to the geometry of the moving image.
    // the values are delivered by mitkRigidRgistrationObserver.cpp
    void SetOptimizerValue( const itk::EventObject & );
    /// this method is called whenever the combobox with the selectable transforms changes
    /// responsible for showing the selected transformparameters
    void TransformSelected( int transform );
    /// this method is called whenever the combobox with the selectable metrics changes
    /// responsible for showing the selected metricparameters
    void MetricSelected( int metric );
    /// this method is called whenever the combobox with the selectable optimizer changes
    /// responsible for showing the selected optimizerparameters
    void OptimizerSelected( int optimizer );

    void LoadRigidRegistrationParameter();
    void SaveRigidRegistrationParameter();
    //void LoadRigidRegistrationTestParameter();
    //void SaveRigidRegistrationTestParameter();
    void DoLoadRigidRegistrationParameter();
    void DoLoadRigidRegistrationPreset(std::string presetName);
    void DoSaveRigidRegistrationParameter();
    void AddTransform(QmitkRigidRegistrationTransformsGUIBase* transform);
    void AddMetric(QmitkRigidRegistrationMetricsGUIBase* metric);
    void AddOptimizer(QmitkRigidRegistrationOptimizerGUIBase* optimizer);

protected:

  Ui::QmitkRigidRegistrationSelector m_Controls;
  mitk::DataNode::Pointer m_FixedNode;
  mitk::DataNode::Pointer m_FixedMaskNode;
  mitk::DataNode::Pointer m_MovingNode;
  mitk::DataNode::Pointer m_MovingMaskNode;
  int m_FixedDimension;
  int m_MovingDimension;
  bool m_StopOptimization;
  mitk::RigidRegistrationPreset* m_Preset;
  mitk::BaseGeometry::TransformType::Pointer m_GeometryItkPhysicalToWorldTransform;
  mitk::BaseGeometry::TransformType::Pointer m_GeometryWorldToItkPhysicalTransform;
  mitk::BaseGeometry* m_MovingGeometry;
  mitk::BaseGeometry::Pointer m_ImageGeometry;
  mitk::RigidRegistrationObserver::Pointer m_Observer;
  mitk::DataStorage::SetOfObjects::ConstPointer m_MovingNodeChildren;
  std::map<mitk::DataNode::Pointer, mitk::BaseGeometry*> m_ChildNodes;
  std::map<mitk::DataNode::Pointer, mitk::BaseGeometry::Pointer> m_ChildNodes2;
};

#endif
