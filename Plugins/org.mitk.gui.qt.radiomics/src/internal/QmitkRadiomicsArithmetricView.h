/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#if !defined(QmitkRadiomicsArithmetricView_H__INCLUDED)
#define QmitkRadiomicsArithmetricView_H__INCLUDED

#include <QmitkAbstractView.h>
#include <org_mitk_gui_qt_radiomics_Export.h>
#include "ui_QmitkRadiomicsArithmetricViewControls.h"

#include "QmitkStepperAdapter.h"

#include <mitkImage.h>
#include <mitkDataStorageSelection.h>
#include <usAny.h>



class RADIOMICS_EXPORT QmitkRadiomicsArithmetric : public QmitkAbstractView
{
  Q_OBJECT

public:

  /*!
  \brief default constructor
  */
  QmitkRadiomicsArithmetric();

  /*!
  \brief default destructor
  */
  ~QmitkRadiomicsArithmetric() override;

  /*!
  \brief method for creating the widget containing the application controls, like sliders, buttons etc.
  */
  void CreateQtPartControl(QWidget *parent) override;

  void SetFocus() override;

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  /*!
  \brief Invoked when the DataManager selection changed
  */
  void OnSelectionChanged(berry::IWorkbenchPart::Pointer part, const QList<mitk::DataNode::Pointer>& nodes) override;


  protected slots:
  void TanButton();
  void ATanButton();
  void CosButton();
  void ACosButton();
  void SinButton();
  void ASinButton();
  void SquareButton();
  void SqrtButton();
  void AbsButton();
  void ExpButton();
  void ExpNegButton();
  void Log10Button();

  void AddLeftButton();
  void SubLeftButton();
  void SubRightButton();
  void MulLeftButton();
  void DivLeftButton();
  void DivRightButton();

  void AddButton();
  void SubButton();
  void MulButton();
  void DivButton();

private:
  /*!
  * controls containing sliders for scrolling through the slices
  */
  Ui::QmitkRadiomicsArithmetricViewControls *m_Controls;

  mitk::Image::Pointer GetFirstImage();
  mitk::Image::Pointer GetSecondImage();
  void AddImageToNode(mitk::Image::Pointer image, std::string nameAddition);


};

#endif // !defined(QmitkRadiomicsArithmetric_H__INCLUDED)


