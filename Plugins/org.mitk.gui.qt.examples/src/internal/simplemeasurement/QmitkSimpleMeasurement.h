/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-21 14:46:24 +0100 (Sat, 21 Mar 2009) $
Version:   $Revision: 15412 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QMITK_MEASUREMENT_H__INCLUDED)
#define QMITK_MEASUREMENT_H__INCLUDED

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

    /*
     * Node representing the PointSet last created. It is used to delete empty point sets.
     */
    std::vector<mitk::DataNode*> m_CreatedPointSetNodes;
};

#endif // !defined(QMITK_MEASUREMENT_H__INCLUDED)
