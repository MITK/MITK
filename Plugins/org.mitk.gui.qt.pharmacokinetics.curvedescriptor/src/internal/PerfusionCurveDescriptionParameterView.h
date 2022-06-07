/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef PerfusionCurveDescriptionParameterView_h
#define PerfusionCurveDescriptionParameterView_h

#include <QString>

#include <QmitkAbstractView.h>
#include "ui_PerfusionCurveDescriptionParameterViewControls.h"
#include "mitkCurveDescriptionParameterBase.h"
#include <QmitkDescriptionParameterBackgroundJob.h>

#include <mitkImage.h>

namespace mitk
{
  class CurveParameterFunctor;
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

  void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
    const QList<mitk::DataNode::Pointer>& nodes) override;

	// Variables

	/*! @brief The view's UI controls */
    Ui::PerfusionCurveDescriptionParameterViewControls m_Controls;
    mitk::DataNode::Pointer m_selectedNode;

private:

    typedef std::map<ParameterNameType, mitk::CurveDescriptionParameterBase::Pointer > ParameterMapType;

    ParameterMapType m_ParameterMap;

    mitk::Image::Pointer m_selectedImage;
    mitk::Image::Pointer m_selectedMask;
};

#endif
