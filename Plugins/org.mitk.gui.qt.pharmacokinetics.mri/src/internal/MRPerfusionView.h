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

#ifndef MRPerfusionView_h
#define MRPerfusionView_h

#include <QString>

#include "QmitkAbstractView.h"

#include "itkCommand.h"

#include "ui_MRPerfusionViewControls.h"
#include "mitkModelBase.h"
#include "QmitkParameterFitBackgroundJob.h"
#include "mitkModelFitResultHelper.h"
#include "mitkModelFactoryBase.h"
#include "mitkLevenbergMarquardtModelFitFunctor.h"
#include "mitkSimpleBarrierConstraintChecker.h"
#include "mitkAIFBasedModelBase.h"



/*!
*	@brief Test Plugin for SUV calculations of PET images
*/
class MRPerfusionView : public QmitkAbstractView
{
  Q_OBJECT

public:

  /*! @brief The view's unique ID - required by MITK */
  static const std::string VIEW_ID;

  MRPerfusionView();

protected slots:

  void OnModellingButtonClicked();

  void OnJobFinished();
  void OnJobError(QString err);
  void OnJobResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType results,
                                const ParameterFitBackgroundJob* pJob);
  void OnJobProgress(double progress);
  void OnJobStatusChanged(QString info);

  void OnModellSet(int);
  void LoadAIFfromFile();

  /**Sets visibility and enabled state of the GUI depending on the settings and workflow state.*/
  void UpdateGUIControls();

protected:
  typedef QList<mitk::DataNode*> SelectedDataNodeVectorType;

  // Overridden base class functions

  /*!
  *	@brief					Sets up the UI controls and connects the slots and signals. Gets
  *							called by the framework to create the GUI at the right time.
  *	@param[in,out] parent	The parent QWidget, as this class itself is not a QWidget
  *							subclass.
  */
  void CreateQtPartControl(QWidget* parent);

  /*!
  *	@brief	Sets the focus to the plot curve button. Gets called by the framework to set the
  *			focus on the right widget.
  */
  void SetFocus();

  /*! @brief Generates a configured fit generator and the corresponding modelinfo for a descriptive brix model with pixel based strategy.
  * @remark add GenerateFunction for each model in the Combo box*/
  void GenerateDescriptiveBrixModel_PixelBased(mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
      mitk::ParameterFitImageGeneratorBase::Pointer& generator);
  void GenerateDescriptiveBrixModel_ROIBased(mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
      mitk::ParameterFitImageGeneratorBase::Pointer& generator);

  void Generate3StepLinearModelFit_PixelBased(mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
      mitk::ParameterFitImageGeneratorBase::Pointer& generator);
  void Generate3StepLinearModelFit_ROIBased(mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
      mitk::ParameterFitImageGeneratorBase::Pointer& generator);


  template <typename TParameterizer>
  void GenerateAIFbasedModelFit_ROIBased(mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
                                 mitk::ParameterFitImageGeneratorBase::Pointer& generator);

  template <typename TParameterizer>
  void GenerateAIFbasedModelFit_PixelBased(mitk::modelFit::ModelFitInfo::Pointer& modelFitInfo,
                                   mitk::ParameterFitImageGeneratorBase::Pointer& generator);

  /** Helper function that configures the initial parameter strategy of a parameterizer
   according to the settings of the GUI.*/
  void ConfigureInitialParametersOfParameterizer(mitk::ModelParameterizerBase* parameterizer) const;

  /*! Starts the fitting job with the passed generator and session info*/
  void DoFit(const mitk::modelFit::ModelFitInfo* fitSession,
             mitk::ParameterFitImageGeneratorBase* generator);

  /**Checks if the settings in the GUI are valid for the chosen model.*/
  bool CheckModelSettings() const;

  void InitModelComboBox() const;

  /*! Helper method that generates a node for the passed concentration image.*/
  mitk::DataNode::Pointer GenerateConcentrationNode(mitk::Image* image, const std::string& nodeName) const;

    /*! \brief called by QmitkFunctionality when DataManager's selection has changed
  */
  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
                                  const QList<mitk::DataNode::Pointer>& selectedNodes);

  // Variables

  /*! @brief The view's UI controls */
  Ui::MRPerfusionViewControls m_Controls;

  /* Nodes selected by user/ui for the fit */
  mitk::DataNode::Pointer m_selectedNode;
  mitk::DataNode::Pointer m_selectedMaskNode;
  mitk::DataNode::Pointer m_selectedAIFMaskNode;
  mitk::DataNode::Pointer m_selectedAIFImageNode;

  /* Images selected by user/ui for the fit */
  mitk::Image::Pointer m_selectedImage;
  mitk::Image::Pointer m_selectedMask;
  mitk::Image::Pointer m_selectedAIFMask;
  mitk::Image::Pointer m_selectedAIFImage;

  mitk::ModelFactoryBase::Pointer m_selectedModelFactory;

  mitk::SimpleBarrierConstraintChecker::Pointer m_modelConstraints;

private:
  bool IsTurboFlashSequenceFlag() const;

  bool m_FittingInProgress;

  typedef std::vector<mitk::ModelFactoryBase::Pointer> ModelFactoryStackType;
  ModelFactoryStackType m_FactoryStack;

  /**Converts the selected image to a concentration image based on the given gui settings.
   AIFMode controls if the concentration image for the fit input or the AIF will be converted.*/
  mitk::Image::Pointer ConvertConcentrationImage(bool AIFMode);

  /**Helper function that (depending on the gui settings) prepares m_inputNode and m_inputImage.
   Either by directly pass back the selected image/node or the newly generated concentration image/node.
   After calling this method  m_inputImage are always what should be used as input image
   for the fitting.*/
  void PrepareConcentrationImage();

  /**Helper function that (depending on the gui settings) prepares m_inputAIFNode and m_inputAIFImage.
  Either by directly pass back the selected image/node or the newly generated concentration image/node.
  After calling this method m_inputAIFImage are always what should be used as AIF image
  for the fitting.*/
  void PrepareAIFConcentrationImage();

  /**Helper function that (depending on the gui settings) generates and passes back the AIF and its time grid
   that should be used for fitting.
   @remark the parameters aif and aifTimeGrid will be initialized accordingly if the method returns.*/
  void GetAIF(mitk::AIFBasedModelBase::AterialInputFunctionType& aif,
              mitk::AIFBasedModelBase::AterialInputFunctionType& aifTimeGrid);

  /**Helper function that generates a default fitting functor
   * default is a levenberg marquart based optimizer with all scales set to 1.0.
   * Constraint setter will be set based on the gui setting and a evaluation parameter
   * "sum of squared differences" will always be set.*/
  mitk::ModelFitFunctorBase::Pointer CreateDefaultFitFunctor(const mitk::ModelParameterizerBase*
      parameterizer) const;

  /**Returns the default fit name, derived from the current GUI settings.*/
  std::string GetDefaultFitName() const;
  /**Returns the current set name of the fit (either default name or use defined name).*/
  std::string GetFitName() const;

  std::vector<double> AIFinputGrid;
  std::vector<double> AIFinputFunction;

  mitk::NodePredicateBase::Pointer m_IsNoMaskImagePredicate;
  mitk::NodePredicateBase::Pointer m_IsMaskPredicate;

  /* Node used for the fit (my be the selected image
  or converted ones (depending on the ui settings */
  mitk::DataNode::Pointer m_inputNode;
  mitk::DataNode::Pointer m_inputAIFNode;
  bool m_HasGeneratedNewInput;
  bool m_HasGeneratedNewInputAIF;

  /* Image used for the fit (my be the selected image
  or converted ones (depending on the ui settings */
  mitk::Image::Pointer m_inputImage;
  mitk::Image::Pointer m_inputAIFImage;

};

#endif
