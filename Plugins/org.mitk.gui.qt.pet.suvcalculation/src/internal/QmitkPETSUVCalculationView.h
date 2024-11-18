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

#ifndef QmitkPETSUVCalculationView_h
#define QmitkPETSUVCalculationView_h

#include "ui_QmitkPETSUVCalculationViewControls.h"
#include <QString>
#include <QmitkAbstractView.h>
#include <mitkImage.h>
#include <mitkSUVCalculationHelper.h>

/*!
 *	@brief Test Plugin for SUV calculations of PET images
 */
class QmitkPETSUVCalculationView : public QmitkAbstractView
{
  Q_OBJECT

public:
  /*! @brief The view's unique ID - required by MITK */
  static const std::string VIEW_ID;

  QmitkPETSUVCalculationView();

protected slots:

  void OnCalculateSUVButtonClicked();
  void OnNuclideLookupClicked();

  void OnInjectedActivityChanged(double);
  void OnBodyWeightChanged(int);
  void OnHalfLifeChanged(double);
  void OnTimeToMeasurementChanged(int);

protected:
  // Overridden base class functions

  /*!
   *	@brief					Sets up the UI controls and connects the slots and signals. Gets
   *							called by the framework to create the GUI at the right time.
   *	@param[in,out] parent	The parent QWidget, as this class itself is not a QWidget
   *							subclass.
   */
  void CreateQtPartControl(QWidget *parent);

  /*!
   *	@brief	Sets the focus to the plot curve button. Gets called by the framework to set the
   *			focus on the right widget.
   */
  void SetFocus();

  /**updates the widgets according to the internal values.*/
  void UpdateWidgets();

  /**Function populates the nuclide half life map.*/
  void GenerateHalfLifeMap();

  mitk::Image::Pointer CalcSUV(mitk::Image *inputImage) const;

  void UpdatePatientWeight();

  virtual void OnSelectionChanged(berry::IWorkbenchPart::Pointer source, const QList<mitk::DataNode::Pointer> &nodes);

  // Variables

  /*! @brief The view's UI controls */
  Ui::QmitkPETSUVCalculationViewControls m_Controls;
  mitk::DataNode::Pointer m_selectedNode;

private:
  /**Activity in Bq*/
  double m_injectedActivity;

  /**Weight in kg*/
  int m_bodyweight;

  /** Time between injection and image acquesition in sec. Used when defined by user and not autodetected.*/
  int m_userDecayTime;
  bool m_validAutoTime;
  mitk::DecayTimeMapType m_autoDecayTime;

  /** Half life in sec*/
  double m_halfLife;

  /**The predefined nuclide by the image data. Empty string implies no definition/custom value.*/
  std::string m_DefinedNuclide;

  typedef std::map<std::string, double> HalfLifeMapType;

  HalfLifeMapType m_HalfLifeMap;

  /** Helper flag that helps to prevent recursive triggering in the gui logic.*/
  bool m_internalUpdate;

  QWidget *m_ParentWidget;
};

#endif
