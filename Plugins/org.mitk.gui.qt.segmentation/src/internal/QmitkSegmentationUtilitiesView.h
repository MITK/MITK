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


#ifndef QmitkSegmentationUtilities_h
#define QmitkSegmentationUtilities_h

#include <berryISelectionListener.h>

#include <QmitkAbstractView.h>

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateNot.h>
#include <mitkSurface.h>
#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryCrossStructuringElement.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryFillholeImageFilter.h>



#include "ui_QmitkSegmentationUtilitiesViewControls.h"

namespace mitk
{
class Surface;
}


/**
  \brief QmitkSegmentationUtilities

  \warning  This class is not yet documented. Use "git blame" and ask the author to provide basic documentation.

  \sa QmitkAbstractView
  \ingroup ${plugin_target}_internal
*/
class QmitkSegmentationUtilitiesView : public QmitkAbstractView
{
  // this is needed for all Qt objects that should have a Qt meta-object
  // (everything that derives from QObject and wants to have signal/slots)
  Q_OBJECT

  public:

    static const std::string VIEW_ID;

    QmitkSegmentationUtilitiesView();

  protected slots:

    void OnImageMaskingToggled(bool);
    void OnSurfaceMaskingToggled(bool);
    void OnMaskImagePressed();
    void OnMaskingDataSelectionChanged(const mitk::DataNode*);
    void OnMaskingReferenceDataSelectionChanged(const mitk::DataNode*);

    /// \brief Called when the image selection for the morphological operations was changed
    void OnMorphologicalOperationsDataSelectionChanged(const mitk::DataNode*);

    ///@{
    /**
    * \brief Performs a morphological operation on selected image.
    */
     void OnbtnClosingClicked();
     void OnbtnOpeningClicked();
     void OnbtnDilatationClicked();
     void OnbtnErosionClicked();
     void OnbtnFillHolesClicked();
    ///@}

    void OnBooleanOperationsSegImage1SelectionChanged(const mitk::DataNode*);
    void OnBooleanOperationsSegImage2SelectionChanged(const mitk::DataNode*);
    //TODO create slots for all boolean buttons

    void OnSurface2ImagePressed();
    void OnSurface2ImageDataSelectionChanged(const mitk::DataNode*);
    void OnSurface2ImageReferenceDataSelectionChanged(const mitk::DataNode*);

  protected:

    virtual void CreateQtPartControl(QWidget *parent);

    virtual void SetFocus();

    /// \brief called by QmitkFunctionality when DataManager's selection has changed
    virtual void OnSelectionChanged( berry::IWorkbenchPart::Pointer source,
                                     const QList<mitk::DataNode::Pointer>& nodes );

    /// \brief Mask an image with a mask. Note, input and mask images must be of the same size.
    mitk::Image::Pointer MaskImage(mitk::Image::Pointer referenceImage, mitk::Image::Pointer maskImage );

    /// \brief Convert a surface to an binary image.
    mitk::Image::Pointer ConvertSurfaceToImage( mitk::Image::Pointer image, mitk::Surface::Pointer surface );

    Ui::QmitkSegmentationUtilitiesViewControls m_Controls;

private:

    void EnableBooleanButtons(bool);


    ///@{
    /**
    * \brief Implements morphological filters with itk functions
    */
    template<typename TPixel, unsigned int VDimension>
    void itkClosing(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor);

    template<typename TPixel, unsigned int VDimension>
    void itkOpening(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor);

    template<typename TPixel, unsigned int VDimension>
    void itkErode(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor);

    template<typename TPixel, unsigned int VDimension>
    void itkDilate(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor);

    template<typename TPixel, unsigned int VDimension>
    void itkFillhole(itk::Image<TPixel, VDimension>* sourceImage, mitk::Image::Pointer & resultImage, int factor);
    ///@}

    mitk::NodePredicateAnd::Pointer m_IsOfTypeImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsOfTypeSurface;
    mitk::NodePredicateProperty::Pointer m_IsBinaryPredicate;
    mitk::NodePredicateNot::Pointer m_IsNotBinaryPredicate;
    mitk::NodePredicateAnd::Pointer m_IsNotABinaryImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsABinaryImagePredicate;



};

#endif // QmitkSegmentationUtilities_h
