/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PerfusionDataSimulationView_h
#define PerfusionDataSimulationView_h

#include <QString>

#include <QmitkAbstractView.h>
#include "ui_PerfusionDataSimulationViewControls.h"
#include "mitkModelBase.h"
#include <mitkModelFactoryBase.h>

#include "itkArray.h"
#include "mitkImage.h"

#include <mitkImage.h>

/*!
 *	@brief Test Plugin for SUV calculations of PET images
 */
class PerfusionDataSimulationView : public QmitkAbstractView
{
	Q_OBJECT

public:

	/*! @brief The view's unique ID - required by MITK */
	static const std::string VIEW_ID;

  PerfusionDataSimulationView();

protected slots:

	/*!
	 *	@brief	Is triggered of the update button is clicked and the selected node should get the (new) iso level set.
	 */
    void OnGenerateDataButtonClicked();
    void LoadAIFFile();
    void OnModellSet(int);

    void OnSimulationConfigurationChanged();

    void UpdateDataSelection();

protected:
    typedef std::map<std::string, mitk::Image::Pointer> ParameterMapType;

	// Overridden base class functions

	/*!
	 *	@brief					Sets up the UI controls and connects the slots and signals. Gets
	 *							called by the framework to create the GUI at the right time.
	 *	@param[in,out] parent	The parent QWidget, as this class itself is not a QWidget
	 *							subclass.
	 */
	void CreateQtPartControl(QWidget* parent) override;

	/*!
	 *	@brief	Sets the focus to the plot curve button. Gets called by the framework to set the
	 *			focus on the right widget.
	 */
	void SetFocus() override;

  void InitModelComboBox() const;

  mitk::Image::Pointer Generate2CXModelData();
  mitk::Image::Pointer GenerateNumeric2CXModelData();
  mitk::Image::Pointer GenerateETModelData();
  void FillParameterMap2CXM();
  void FillParameterMapNumeric2CXM();
  void FillParameterMapETM();

  ///////////////////// dynamic PET Models///////////////
  mitk::Image::Pointer Generate2TCModelData();
  mitk::Image::Pointer Generate1TCModelData();
  void FillParameterMap2TCM();
  void FillParameterMap1TCM();


  bool CheckModelSettings();

	// Variables

	/*! @brief The view's UI controls */
    Ui::PerfusionDataSimulationViewControls m_Controls;
    mitk::DataNode::Pointer m_selectedNode;

    ParameterMapType m_ParameterImageMap;
    itk::Array<double> m_AterialInputFunction;
    itk::Array<double> m_TimeGrid;

    mitk::ModelFactoryBase::Pointer m_selectedModelFactory;

private:

    typedef std::vector<mitk::ModelFactoryBase::Pointer> ModelFactoryStackType;
    ModelFactoryStackType m_FactoryStack;
    mitk::NodePredicateBase::Pointer m_IsNotABinaryImagePredicate;

    double m_CNR, m_MaxConcentration, m_Sigma;

};

#endif
