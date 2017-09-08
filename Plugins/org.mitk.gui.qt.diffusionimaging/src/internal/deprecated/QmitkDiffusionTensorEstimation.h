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

#if !defined(QmitkDiffusionTensorEstimation_H__INCLUDED)
#define QmitkDiffusionTensorEstimation_H__INCLUDED

#include <QmitkAbstractView.h>
#include <mitkILifecycleAwarePart.h>
#include "mitkDiffusionVolumes.h"

class QmitkDiffusionTensorEstimationControls;

typedef short DiffusionPixelType;


/*!
\brief QmitkDiffusionTensorEstimation

One needs to reimplement the methods CreateControlWidget(..),
and CreateAction(..).
*/
class QmitkDiffusionTensorEstimation : public QmitkAbstractView, public mitk::ILifecycleAwarePart
{
  Q_OBJECT

public:
  /*!
  \brief default constructor
  */
  QmitkDiffusionTensorEstimation(QObject *parent=0, const char *name=0, mitk::DataTreeIteratorBase* dataIt = nullptr);

  /*!
  \brief default destructor
  */
  virtual ~QmitkDiffusionTensorEstimation();

  /*!
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.
  */
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!
  \brief method for creating the connections of main and control widget
  */
  virtual void CreateConnections();

  ///
  /// Sets the focus to an internal widget.
  ///
  virtual void SetFocus() override;

  /*!
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget
  */
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated() override;
  virtual void Deactivated() override;
  virtual void Visible() override;
  virtual void Hidden() override;

  protected slots:
    void TreeChanged();

    void TensorVolumesSaveButton();

    void TensorVolumesLoadButton();

    void TensorVolumesRemoveButton();

    void OdfVolumesSaveButton();

    void OdfVolumesLoadButton();

    void OdfVolumesRemoveButton();

    void DirectionVolumesSaveButton();

    void DirectionVolumesLoadButton();

    void DirectionVolumesRemoveButton();

    void DirectionVolumesAngularErrorButton();

    void TensorEstimationTeemEstimateButton();

    void OdfReconstructionButton();

    void OdfReconstructionAnalyticalButton();

    void TensorEstimationButton();

    void StandardAlgorithmsFAButton();

    void StandardAlgorithmsRAButton();

    void StandardAlgorithmsDirectionButton();

    void OdfStandardAlgorithmsDirectionButton();

    void OdfStandardAlgorithmsDeconvolutionButton();

    void OdfStandardAlgorithmsGFAButton();

    void OdfVolumesVisualizeSelectedButton();

    void DiffusionVolumeSaveButton();

    void DiffusionVolumesLoadButton();

    void DiffusionVolumesRemoveButton();

    void DiffusionVolumesSelectAll();

    //void DwiStandardAlgorithmsGFAButton();

    void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

protected:

  /*!
  * controls containing sliders for scrolling through the slices
  */
  QmitkDiffusionTensorEstimationControls * m_Controls;

  mitk::DataTreeFilter::Pointer m_DiffusionVolumesDataTreeFilter;

  mitk::DataTreeFilter::Pointer m_TensorVolumesDataTreeFilter;

  mitk::DataTreeFilter::Pointer m_OdfVolumesDataTreeFilter;

  mitk::DataTreeFilter::Pointer m_DirectionVolumesDataTreeFilter;

  bool m_FilterInitialized;

  static const int odfsize;

  static const int nrconvkernels;

  template<int L>
  void ReconstructAnalytically(mitk::DiffusionVolumes<DiffusionPixelType>* vols,
    float lambda, std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes);

};
#endif // !defined(QmitkDiffusionTensorEstimation_H__INCLUDED)
