/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkRigidRegistrationSelectorViewWidgetHIncluded
#define QmitkRigidRegistrationSelectorViewWidgetHIncluded

#include "mitkDataTreeNode.h"
#include "mitkDataStorage.h"
#include "ui_QmitkRigidRegistrationSelector.h"
#include "mitkRigidRegistrationTestPreset.h"
#include "qobject.h"
#include "../RigidregistrationDll.h"


/*!
* \brief Widget for deformable demons registration
*
* Displays options for demons registration.
*/
class RIGIDREGISTRATION_EXPORT QmitkRigidRegistrationSelectorView : public QWidget
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
    void SetFixedNode( mitk::DataTreeNode * fixedNode );
    void SetFixedMaskNode(mitk::DataTreeNode * fixedMaskNode );
    void SetFixedDimension( int dimension );
    void SetMovingNode( mitk::DataTreeNode * movingNode );
    void SetMovingNodeChildren(mitk::DataStorage::SetOfObjects::ConstPointer children);
    void SetMovingMaskNode(mitk::DataTreeNode * movingMaskNode );
    void SetMovingDimension(int dimension );
    int GetSelectedTransform();
    void CalculateTransformation(unsigned int timestep = 0);
    void StopOptimization(bool stopOptimization);

  protected slots:
    /// method to make all TransformFrames invisible
    void hideAllTransformFrames();
    /// method to make all OptimizerFrames invisible
    void hideAllOptimizerFrames();
    /// method to make all MetricFrames invisible
    void hideAllMetricFrames();
    // this is a callback function that retrieves the current transformation
    // parameters after every step of progress in the optimizer.
    // depending on the choosen transformation, we construct a vtktransform
    // that will be applied to the geometry of the moving image.
    // the values are delivered by mitkRigidRgistrationObserver.cpp
    void SetOptimizerValue( const itk::EventObject & );
    /// this method is called whenever the combobox with the selectable transforms changes
    /// responsible for showing the selected transformparameters
    void TransformSelected( int transform );
    /// this method is called whenever the combobox with the selectable optimizers changes
    /// responsible for showing the selected optimizerparameters
    void OptimizerSelected( int optimizer );
    /// this method is called whenever the combobox with the selectable metrics changes
    /// responsible for showing the selected metricparameters
    void MetricSelected( int metric );
    /// this method writes the transform parameters from the selected transform into mitkTransformParameters class
    /// so that the desired transform can be build up with these parameters
    void setTransformParameters();
    /// this method writes the optimizer parameters from the selected optimizer into mitkOptimizerParameters class
    /// so that the desired optimizer can be build up with these parameters
    void setOptimizerParameters();
    /// this method writes the metric parameters from the selected metric into mitkMetricParameters class
    /// so that the desired metric can be build up with these parameters
    void setMetricParameters();
    /// this method is needed because the Amoeba optimizer needs a simplex delta parameter, which defines the search range for each transform parameter
    /// every transform has its own set of parameters, so this method looks for every time showing the correct number of simplex delta input fields for the
    /// Amoeba optimizer
    void SetSimplexDeltaVisible();
    void LoadRigidRegistrationParameter();
    void SaveRigidRegistrationParameter();
    void LoadRigidRegistrationTestParameter();
    void SaveRigidRegistrationTestParameter();
    void DoLoadRigidRegistrationParameter(bool testPreset);
    void DoLoadRigidRegistrationPreset(std::string presetName, bool testPreset);
    void DoSaveRigidRegistrationParameter(bool testPreset);

protected:

  Ui::QmitkRigidRegistrationSelector m_Controls;
  mitk::DataTreeNode::Pointer m_FixedNode;
  mitk::DataTreeNode::Pointer m_FixedMaskNode;
  mitk::DataTreeNode::Pointer m_MovingNode;
  mitk::DataTreeNode::Pointer m_MovingMaskNode;
  mitk::OptimizerParameters::Pointer m_OptimizerParameters;
  mitk::TransformParameters::Pointer m_TransformParameters;
  mitk::MetricParameters::Pointer m_MetricParameters;
  int m_FixedDimension;
  int m_MovingDimension;
  bool m_StopOptimization;
  mitk::RigidRegistrationPreset* m_Preset;
  mitk::RigidRegistrationTestPreset* m_TestPreset;
  mitk::Geometry3D::TransformType::Pointer m_GeometryItkPhysicalToWorldTransform;
  mitk::Geometry3D::TransformType::Pointer m_GeometryWorldToItkPhysicalTransform;
  mitk::Geometry3D* m_MovingGeometry;
  mitk::AffineGeometryFrame3D::Pointer m_ImageGeometry;
  mitk::RigidRegistrationObserver::Pointer m_Observer;
  mitk::DataStorage::SetOfObjects::ConstPointer m_MovingNodeChildren;
  std::map<mitk::DataTreeNode::Pointer, mitk::Geometry3D*> m_ChildNodes;
  std::map<mitk::DataTreeNode::Pointer, mitk::AffineGeometryFrame3D::Pointer> m_ChildNodes2;

  itk::Array2D<unsigned int> ParseSchedule(std::string s);
  void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");

};

#endif
