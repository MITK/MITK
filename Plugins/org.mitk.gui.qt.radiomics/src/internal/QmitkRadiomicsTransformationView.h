/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkRadiomicsTransformationView_h
#define QmitkRadiomicsTransformationView_h

#include <QmitkAbstractView.h>
#include <org_mitk_gui_qt_radiomics_Export.h>
#include "ui_QmitkRadiomicsTransformationViewControls.h"

#include "QmitkStepperAdapter.h"

#include <mitkDataStorageSelection.h>
#include <usAny.h>

/*!
\brief This module allows to use some basic image processing filters for preprocessing, image enhancement and testing purposes

Several basic ITK image processing filters, like denoising, morphological and edge detection
are encapsulated in this module and can be selected via a list and an intuitive parameter input.
The selected filter will be applied on the image, and a new image showing the output is displayed
as result.
Also, some image arithmetic operations are available.

Images can be 3D or 4D.
In the 4D case, the filters work on the 3D image selected via the
time slider. The result is also a 3D image.

\class QmitkRadiomicsTransformation
\author Tobias Schwarz
\version 1.0 (3M3)
\date 2009-05-10
\ingroup Bundles
*/

class RADIOMICS_EXPORT QmitkRadiomicsTransformation : public QmitkAbstractView
{
  Q_OBJECT

public:

  /*!
  \brief default constructor
  */
  QmitkRadiomicsTransformation();

  /*!
  \brief default destructor
  */
  ~QmitkRadiomicsTransformation() override;

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
  void executeButtonMultiResolutionPressed();
  void executeButtonWaveletPressed();
  void executeButtonLoGPressed();
  void executeButtonResamplingPressed();

private:
  /*!
  * controls containing sliders for scrolling through the slices
  */
  Ui::QmitkRadiomicsTransformationViewControls *m_Controls;

};

#endif
