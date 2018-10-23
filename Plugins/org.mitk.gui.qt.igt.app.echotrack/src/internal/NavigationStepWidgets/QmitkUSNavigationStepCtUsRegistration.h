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

#ifndef QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H
#define QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H

#include "QmitkUSAbstractNavigationStep.h"

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateOr.h>

#include <mitkPointSet.h>
#include <mitkImage.h>

namespace itk {
template<class T> class SmartPointer;
}

namespace mitk {
class NodeDisplacementFilter;
class NavigationDataSource;
}


namespace Ui {
class QmitkUSNavigationStepCtUsRegistration;
}

/**
 * \brief Navigation step for marking risk structures.
 * The user can add risk structures by interacting with the render windows. The
 * risk structures are organized in an embedded table view.
 *
 * The risk structures are stored under DATANAME_BASENODE -> DATANAME_ZONES.
 *
 * This step is ready for the next step directly after activating. All actions
 * to be done in this step are optional.
 */
class QmitkUSNavigationStepCtUsRegistration : public QmitkUSAbstractNavigationStep
{
  Q_OBJECT



public:
  explicit QmitkUSNavigationStepCtUsRegistration(QWidget *parent = 0);
  ~QmitkUSNavigationStepCtUsRegistration();

  /**
   * \brief Initialization of the data storage nodes.
   * \return always true
   */
  virtual bool OnStartStep();

  /**
   * \brief Resets widget and filter and removes nodes from the data storage.
   * \return always true
   */
  virtual bool OnStopStep();

  /**
   * \brief There is nothing to be done.
   * \return always true
   */
  virtual bool OnFinishStep();

  /**
   * \brief Selects input for the node displacement filter and emits "ReadyForNextStep" signal.
   * The input selection cannot be done earlier.
   * \return
   */
  virtual bool OnActivateStep();

  virtual bool OnDeactivateStep();

  /**
   * \brief Updates just the tracking validity status.
   */
  virtual void OnUpdate();

  /**
   * The property "settings.interaction-concept" is used.
   */
  virtual void OnSettingsChanged(const itk::SmartPointer<mitk::DataNode> settingsNode);

  virtual QString GetTitle();

  /**
   * @return a node displacement filter for the zone surfaces
   */
  virtual FilterVector GetFilter();

protected:
  virtual void OnSetCombinedModality();

  void UnsetFloatingImageGeometry();
  void SetFloatingImageGeometryInformation(mitk::Image* image);
  double GetVoxelVolume();
  double GetFiducialVolume(double radius);

  void DefineDataStorageImageFilter();
  void CreateQtPartControl(QWidget *parent);
  
protected slots:
  void OnFloatingImageComboBoxSelectionChanged(const mitk::DataNode* node);
  void OnFiducialMarkerModelComboBoxSelectionChanged(const mitk::DataNode* node);
  void OnDevComboBoxChanged(const mitk::DataNode* node);
  void OnRegisterMarkerToFloatingImageCS();
  void OnFilterFloatingImage();

private:
  Ui::QmitkUSNavigationStepCtUsRegistration *ui;

  mitk::NodePredicateAnd::Pointer m_IsOfTypeImagePredicate;
  mitk::NodePredicateOr::Pointer m_IsASegmentationImagePredicate;
  mitk::NodePredicateAnd::Pointer m_IsAPatientImagePredicate;
  mitk::TNodePredicateDataType<mitk::PointSet>::Pointer m_IsAPointSetPredicate;

  itk::SmartPointer<mitk::NavigationDataSource> m_NavigationDataSource;
  mitk::Image::Pointer m_FloatingImage;
  mitk::PointSet::Pointer m_MarkerModelCoordinateSystemPointSet;
  mitk::PointSet::Pointer m_MarkerFloatingImageCoordinateSystemPointSet;
  
  std::vector<mitk::Point3D> m_CentroidsOfFiducialCandidates;

  mitk::AffineTransform3D::Pointer m_TransformMarkerCSToFloatingImageCS;
  /*!
  \brief The 3D dimension of the CT image given in index size.
  */
  mitk::Vector3D m_ImageDimension;
  mitk::Vector3D m_ImageSpacing;

};

#endif // QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H
