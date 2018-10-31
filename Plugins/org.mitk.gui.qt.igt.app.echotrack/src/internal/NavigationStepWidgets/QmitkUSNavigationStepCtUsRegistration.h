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
#include <mitkPlaneFit.h>

#include <itkImage.h>
#include <itkThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkGradientMagnitudeImageFilter.h>
#include <itkLaplacianRecursiveGaussianImageFilter.h>
#include "itkVotingBinaryIterativeHoleFillingImageFilter.h"
#include <itkBinaryImageToShapeLabelMapFilter.h>

namespace itk {
template<class T> class SmartPointer;

}

namespace mitk {
class NodeDisplacementFilter;
class NavigationDataSource;
class PlaneFit;
}

namespace Ui {
class QmitkUSNavigationStepCtUsRegistration;
}

// Declare typedefs:
typedef itk::Image<int, 3>  ImageType;
typedef itk::ThresholdImageFilter<ImageType> ThresholdImageFilterType;
typedef itk::BinaryThresholdImageFilter <ImageType, ImageType> BinaryThresholdImageFilterType;
typedef itk::LaplacianRecursiveGaussianImageFilter<ImageType, ImageType> LaplacianRecursiveGaussianImageFilterType;
typedef itk::VotingBinaryIterativeHoleFillingImageFilter<ImageType> VotingBinaryIterativeHoleFillingImageFilterType;
typedef itk::BinaryImageToShapeLabelMapFilter<ImageType> BinaryImageToShapeLabelMapFilterType;

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

  bool FilterFloatingImage();
  void InitializeImageFilters();
  void EliminateTooSmallLabeledObjects( ImageType::Pointer binaryImage);
  bool EliminateFiducialCandidatesByEuclideanDistances();
  void ClassifyFiducialCandidates();
  void GetCentroidsOfLabeledObjects();
  void CalculatePCA();
  void NumerateFiducialMarks();
  void CalculateDistancesBetweenFiducials(std::vector<std::vector<double>> &distanceVectorsFiducials);
  bool FindFiducialNo1(std::vector<std::vector<double>> &distanceVectorsFiducials);
  bool FindFiducialNo2And3();
  bool FindFiducialNo4(std::vector<std::vector<double>> &distanceVectorsFiducials);
  bool FindFiducialNo5();
  bool FindFiducialNo6();
  bool FindFiducialNo7();
  bool FindFiducialNo8();
  void OptimizeFiducialPositions();
  void CreateParallelPlanes(mitk::PlaneFit::Pointer planeA, mitk::PlaneFit::Pointer planeB,
                          mitk::PointSet::Pointer pointSetA, mitk::PointSet::Pointer pointSetB,
                          mitk::PlaneGeometry::Pointer planeGeometryA, mitk::PlaneGeometry::Pointer planeGeometryB);
  void MovePlanes(mitk::PlaneGeometry::Pointer planeA, mitk::PlaneGeometry::Pointer planeB,
                  double referenceDistance);
  void MovePoint(mitk::PlaneGeometry::Pointer planeGeometry, int fiducialNo);
  void DefineDataStorageImageFilter();
  void CreateQtPartControl(QWidget *parent);

protected slots:
  void OnFloatingImageComboBoxSelectionChanged(const mitk::DataNode* node);
  void OnFiducialMarkerModelComboBoxSelectionChanged(const mitk::DataNode* node);
  void OnRegisterMarkerToFloatingImageCS();
  void OnLocalizeFiducials();

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

  ThresholdImageFilterType::Pointer m_ThresholdFilter;
  BinaryThresholdImageFilterType::Pointer m_BinaryThresholdFilter;
  LaplacianRecursiveGaussianImageFilterType::Pointer m_LaplacianFilter1;
  LaplacianRecursiveGaussianImageFilterType::Pointer m_LaplacianFilter2;
  VotingBinaryIterativeHoleFillingImageFilterType::Pointer m_HoleFillingFilter;
  BinaryImageToShapeLabelMapFilterType::Pointer m_BinaryImageToShapeLabelMapFilter;

  std::vector<mitk::Vector3D> m_CentroidsOfFiducialCandidates;
  std::map<double, mitk::Vector3D> m_EigenVectorsFiducialCandidates;
  std::vector<double> m_EigenValuesFiducialCandidates;
  mitk::Vector3D m_MeanCentroidFiducialCandidates;
  std::map<int, mitk::Vector3D> m_FiducialMarkerCentroids;

  mitk::AffineTransform3D::Pointer m_TransformMarkerCSToFloatingImageCS;
  /*!
  \brief The 3D dimension of the CT image given in index size.
  */
  mitk::Vector3D m_ImageDimension;
  mitk::Vector3D m_ImageSpacing;

};

#endif // QMITKUSNAVIGATIONSTEPCTUSREGISTRATION_H
