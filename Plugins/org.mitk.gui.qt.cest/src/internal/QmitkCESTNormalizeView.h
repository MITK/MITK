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

#ifndef QmitkCESTNormalizeView_h
#define QmitkCESTNormalizeView_h

#include <QString>

#include <QmitkAbstractView.h>

#include <itkCommand.h>

#include "ui_QmitkCESTNormalizeViewControls.h"

#include <mitkNodePredicateBase.h>

/*!
*	@brief Test Plugin for SUV calculations of PET images
*/
class QmitkCESTNormalizeView : public QmitkAbstractView
{
  Q_OBJECT

public:

  /*! @brief The view's unique ID - required by MITK */
  static const std::string VIEW_ID;

  QmitkCESTNormalizeView();

protected slots:

  void OnNormalizeButtonClicked();

  /**Sets visibility and enabled state of the GUI depending on the settings and workflow state.*/
  void UpdateGUIControls();

protected:
  using SelectedDataNodeVectorType = QList<mitk::DataNode*>;

  // Overridden base class functions

  /*!
  *	@brief					Sets up the UI controls and connects the slots and signals. Gets
  *							called by the framework to create the GUI at the right time.
  *	@param[in,out] parent	The parent QWidget, as this class itself is not a QWidget
  *							subclass.
  */
  virtual void CreateQtPartControl(QWidget* parent) override;

  /*!
  *	@brief	Sets the focus to the plot curve button. Gets called by the framework to set the
  *			focus on the right widget.
  */
  virtual void SetFocus() override;

  // Variables

  /*! @brief The view's UI controls */
  Ui::QmitkCESTNormalizeViewControls m_Controls;

  mitk::NodePredicateBase::Pointer m_IsCESTImagePredicate;
};

#endif