/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 15:15:45 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13136 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#if !defined(QmitkDiffusionTensorEstimation_H__INCLUDED)
#define QmitkDiffusionTensorEstimation_H__INCLUDED

#include "QmitkFunctionality.h"
#include "mitkDiffusionVolumes.h"

class QmitkStdMultiWidget;
class QmitkDiffusionTensorEstimationControls;

typedef short DiffusionPixelType;


/*!
\brief QmitkDiffusionTensorEstimation 

One needs to reimplement the methods CreateControlWidget(..), CreateMainWidget(..) 
and CreateAction(..) from QmitkFunctionality. 

\sa QmitkFunctionality
\ingroup Functionalities
*/
class QmitkDiffusionTensorEstimation : public QmitkFunctionality
{  
  Q_OBJECT

public:  
  /*!  
  \brief default constructor  
  */  
  QmitkDiffusionTensorEstimation(QObject *parent=0, const char *name=0, QmitkStdMultiWidget *mitkStdMultiWidget = NULL, mitk::DataTreeIteratorBase* dataIt = NULL);

  /*!  
  \brief default destructor  
  */  
  virtual ~QmitkDiffusionTensorEstimation();

  /*!  
  \brief method for creating the widget containing the application   controls, like sliders, buttons etc.  
  */  
  virtual QWidget * CreateControlWidget(QWidget *parent);

  /*!  
  \brief method for creating the applications main widget  
  */  
  virtual QWidget * CreateMainWidget(QWidget * parent);

  /*!  
  \brief method for creating the connections of main and control widget  
  */  
  virtual void CreateConnections();

  /*!  
  \brief method for creating an QAction object, i.e. button & menu entry  @param parent the parent QWidget  
  */  
  virtual QAction * CreateAction(QActionGroup *parent);

  virtual void Activated();

  protected slots:  
    void TreeChanged();

    void TensorVolumesSaveButton();

    void TensorVolumesLoadButton();

    void TensorVolumesRemoveButton();

    void QBallVolumesSaveButton();

    void QBallVolumesLoadButton();

    void QBallVolumesRemoveButton();

    void DirectionVolumesSaveButton();

    void DirectionVolumesLoadButton();

    void DirectionVolumesRemoveButton();

    void DirectionVolumesAngularErrorButton();

    void TensorEstimationTeemEstimateButton();

    void QBallReconstructionButton();

    void QBallReconstructionAnalyticalButton();

    void TensorEstimationButton();

    void StandardAlgorithmsFAButton();

    void StandardAlgorithmsRAButton();

    void StandardAlgorithmsDirectionButton();

    void QBallStandardAlgorithmsDirectionButton();

    void QBallStandardAlgorithmsDeconvolutionButton();

    void QBallStandardAlgorithmsGFAButton();

    void QBallVolumesVisualizeSelectedButton();

    void DiffusionVolumeSaveButton();

    void DiffusionVolumesLoadButton();

    void DiffusionVolumesRemoveButton();

    void DiffusionVolumesSelectAll();

    //void DwiStandardAlgorithmsGFAButton();

    void SetDefaultNodeProperties(mitk::DataNode::Pointer node, std::string name);

protected:  
  /*!  
  * default main widget containing 4 windows showing 3   
  * orthogonal slices of the volume and a 3d render window  
  */  
  QmitkStdMultiWidget * m_MultiWidget;

  /*!  
  * controls containing sliders for scrolling through the slices  
  */  
  QmitkDiffusionTensorEstimationControls * m_Controls;

  mitk::DataTreeFilter::Pointer m_DiffusionVolumesDataTreeFilter;
  
  mitk::DataTreeFilter::Pointer m_TensorVolumesDataTreeFilter;
  
  mitk::DataTreeFilter::Pointer m_QballVolumesDataTreeFilter;

  mitk::DataTreeFilter::Pointer m_DirectionVolumesDataTreeFilter;

  bool m_FilterInitialized;

  static const int odfsize;
  
  static const int nrconvkernels;

  template<int L>
  void ReconstructAnalytically(mitk::DiffusionVolumes<DiffusionPixelType>* vols, 
    float lambda, std::string nodename, std::vector<mitk::DataNode::Pointer>* nodes);

};
#endif // !defined(QmitkDiffusionTensorEstimation_H__INCLUDED)
