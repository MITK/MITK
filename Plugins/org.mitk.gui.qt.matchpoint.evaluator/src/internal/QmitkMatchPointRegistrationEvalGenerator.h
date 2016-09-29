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

#ifndef __Q_MITK_MATCHPOINT_REGISTRATION_EVAL_GENERATOR_H
#define __Q_MITK_MATCHPOINT_REGISTRATION_EVAL_GENERATOR_H

#include <QmitkAbstractView.h>

#include "ui_QmitkMatchPointRegistrationEvalGenerator.h"

/*!
  \brief QmitkMatchPointRegistrationEvalGenerator

  Class implements the eval generator view, containing the needed business logic and interaction schemes.
  Purpose of the view is to generate registration eval (helper) objects by (auto) selecting appropriate
  data nodes from the data manager.

  \sa QmitkFunctionality
  \ingroup ${plugin_target}_internal
*/
class QmitkMatchPointRegistrationEvalGenerator : public QmitkAbstractView
{
	// this is needed for all Qt objects that should have a Qt meta-object
	// (everything that derives from QObject and wants to have signal/slots)
	Q_OBJECT

public:

	static const std::string VIEW_ID;

	/**
	 * Creates smartpointer typedefs
	 */
	berryObjectMacro(QmitkMatchPointRegistrationEvalGenerator)

	QmitkMatchPointRegistrationEvalGenerator();

	virtual void CreateQtPartControl(QWidget* parent);

protected slots:

	/// \brief Called when the user clicks the GUI button
	void OnEvalBtnPushed();

protected:
	virtual void SetFocus();

	/// \brief called by QmitkFunctionality when DataManager's selection has changed
	virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source,
	                                const QList<mitk::DataNode::Pointer>& nodes);

	Ui::MatchPointRegistrationEvalGeneratorControls m_Controls;

private:
	QWidget* m_Parent;

	void Error(QString msg);

	/**
	 * Checks if appropriated nodes are selected in the data manager. If nodes are selected,
	 * they are stored m_spSelectedRegNode, m_spSelectedInputNode and m_spSelectedRefNode.
	 * They are also checked for vadility and stored in m_ValidInput,... .
	 * It also sets the info lables accordingly.*/
	void CheckInputs();

	/**
	 * Updates the state of mapping control button regarding to selected
	 * input, registration and reference.*/
	void ConfigureMappingControls();

	mitk::DataNode::Pointer m_spSelectedRegNode;
	mitk::DataNode::Pointer m_spSelectedMovingNode;
	mitk::DataNode::Pointer m_spSelectedTargetNode;

	bool m_autoTarget;
	bool m_autoMoving;
};

#endif // MatchPoint_h

