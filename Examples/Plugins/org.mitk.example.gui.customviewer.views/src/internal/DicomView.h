/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef DICOMVIEW_H_
#define DICOMVIEW_H_

#include <QmitkAbstractView.h>
#include <berryQtViewPart.h>

namespace Ui
{
  class QmitkDicomViewControls;
}

/**
 * \brief A view class suited for the DicomPerspective within the custom viewer plug-in.
 *
 * This view class contributes dicom import functionality to the DicomPerspective.
 * The view controls are provided within CreatePartControl() by the QmitkDicomExternalDataWidget
 * class. A DicomView instance is part of the DicomPerspective for Dicom import functionality.
 */
// //! [DicomViewDecl]
class DicomView : public QmitkAbstractView
// //! [DicomViewDecl]
{
  Q_OBJECT

public:
  /**
   * String based view identifier.
   */
  static const std::string VIEW_ID;

  /**
   * Standard constructor.
   */
  DicomView();

  /**
   * Standard destructor.
   */
  ~DicomView() override;

  /**
   * Creates the view control widgets provided by the QmitkDicomExternalDataWidget class.
   * Widgets associated with unused functionality are being removed and DICOM import and data
   * storage transfer functionality being connected to the appropriate slots.
   */
  void CreateQtPartControl(QWidget *parent) override;

protected Q_SLOTS:

  /**
   * Loads the given DICOM series.
   */
  void OnViewSeries(const std::vector<std::pair<std::string, std::optional<std::string>>>& series);

protected:
  void SetFocus() override;

  Ui::QmitkDicomViewControls *m_Controls;

  QWidget *m_Parent;
};

#endif /*DICOMVIEW_H_*/
