/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PerfusionCurveDescriptionParameterView_h
#define PerfusionCurveDescriptionParameterView_h

#include <mitkCurveDescriptionParameterBase.h>
#include <mitkImage.h>
#include <mitkNodePredicateBase.h>

#include <QmitkAbstractView.h>
#include <QmitkDescriptionParameterBackgroundJob.h>

#include <QString>

#include <memory>

namespace mitk
{
  class CurveParameterFunctor;
}

namespace Ui
{
  class PerfusionCurveDescriptionParameterViewControls;
}

/*!
 *	@brief Test Plugin for SUV calculations of PET images
 */
class PerfusionCurveDescriptionParameterView : public QmitkAbstractView
{
	Q_OBJECT

public:

    typedef mitk::CurveDescriptionParameterBase::CurveDescriptionParameterNameType ParameterNameType;

	/*! @brief The view's unique ID - required by MITK */
	static const std::string VIEW_ID;

  PerfusionCurveDescriptionParameterView();
  ~PerfusionCurveDescriptionParameterView() override;

protected slots:

    void InitParameterList();

    void OnJobFinished();
    void OnJobError(QString err);
    void OnJobResultsAreAvailable(mitk::modelFit::ModelFitResultNodeVectorType results,
      const DescriptionParameterBackgroundJob* pJob);
    void OnJobProgress(double progress);
    void OnJobStatusChanged(QString info);

	/*!
	 *	@brief	Is triggered of the update button is clicked and the selected node should get the (new) iso level set.
	 */
    void OnCalculateParametersButtonClicked();

protected:

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

  /** Configures the passed functor according to the selected image and parameters*/
  void ConfigureFunctor(mitk::CurveParameterFunctor* functor) const;


  void OnNodeSelectionChanged(const QList<mitk::DataNode::Pointer>& nodes);

	// Variables

	/*! @brief The view's UI controls */
    std::unique_ptr<Ui::PerfusionCurveDescriptionParameterViewControls> m_Controls;
    mitk::DataNode::Pointer m_selectedNode;

private:

    typedef std::map<ParameterNameType, mitk::CurveDescriptionParameterBase::Pointer > ParameterMapType;

    ParameterMapType m_ParameterMap;

    mitk::Image::Pointer m_selectedImage;


    mitk::NodePredicateBase::Pointer m_isValidTimeSeriesImagePredicate;

};

#endif
