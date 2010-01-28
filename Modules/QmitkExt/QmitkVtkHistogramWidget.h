/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date: 2009-05-15 18:09:46 +0200 (Fr, 15 Mai 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QmitkVtkHistogramWidget_H_
#define QmitkVtkHistogramWidget_H_

#include "QmitkHistogram.h"
#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"

#include <qlayout.h>
#include <qtextedit.h>
#include <qdialog.h>

#include <itkHistogram.h>

#include <vtkQtChartWidget.h>
#include <vtkQtBarChart.h>

#include <QStandardItemModel>




/** 
 * \brief Widget for displaying image histograms based on the vtkQtChart
 * framework
 */
class QmitkExt_EXPORT QmitkVtkHistogramWidget : public QDialog, public itk::Object
{
  Q_OBJECT

public:
  QmitkVtkHistogramWidget( QWidget *parent = 0 );
  virtual ~QmitkVtkHistogramWidget();

  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  typedef itk::ImageRegion< 3 > RegionType;

  enum
  {
    HISTOGRAM_MODE_DIRECT = 0,
    HISTOGRAM_MODE_ENTIREIMAGE,
    HISTOGRAM_MODE_MASKEDIMAGE,
    HISTOGRAM_MODE_IMAGEREGION,
    HISTOGRAM_MODE_PLANARFIGUREREGION
  };


  /** \brief Set histogram to be displayed directly. */
  itkSetConstObjectMacro( Histogram, HistogramType );
  
  /** \brief Set image from which to calculate the histogram. */
  itkSetConstObjectMacro( Image, mitk::Image );

  /** \brief Set binary image mask determining the voxels to include in 
   * histogram calculation. */
  itkSetConstObjectMacro( ImageMask, mitk::Image );

  /** \brief Set 3D image region for which to calculate the histogram. */
  itkSetMacro( ImageRegion, RegionType );

  /** \brief Set planar figure describing the region for which to calculate
   * the histogram. */
  itkSetConstObjectMacro( PlanarFigure, mitk::PlanarFigure  );


  /** \brief Set/Get operation mode for Histogram */
  itkSetMacro( HistogramMode, unsigned int );
  
  /** \brief Set/Get operation mode for Histogram */
  itkGetMacro( HistogramMode, unsigned int );

  /** \brief Set/Get operation mode for Histogram */
  void SetHistogramModeToDirectHistogram();

  /** \brief Set/Get operation mode for Histogram */
  void SetHistogramModeToEntireImage();

  /** \brief Set/Get operation mode for Histogram */
  void SetHistogramModeToMaskedImage();

  /** \brief Set/Get operation mode for Histogram */
  void SetHistogramModeToImageRegion();

  /** \brief Set/Get operation mode for Histogram */
  void SetHistogramModeToPlanarFigureRegion();


  /** Fill the graphical widget with currently specified histogram. */
  void UpdateItemModelFromHistogram();

  /** \brief Clear the histogram (nothing is displayed). */
  void ClearItemModel();


protected slots:


protected:

  void ComputeHistogram();

  vtkQtChartWidget *m_ChartWidget;
  vtkQtBarChart *m_BarChart;

  QStandardItemModel *m_ItemModel;

  mitk::Image::ConstPointer m_Image;

  mitk::Image::ConstPointer m_ImageMask;

  RegionType m_ImageRegion;

  mitk::PlanarFigure::ConstPointer m_PlanarFigure;

  unsigned int m_HistogramMode;


  // Histogram set explicitly by user
  HistogramType::ConstPointer m_Histogram;

  // Histogram derived from image (not set explicitly by user)
  HistogramType::ConstPointer m_DerivedHistogram;


};

#endif /* QmitkVtkHistogramWidget_H_ */
