/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
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

#ifndef QmitkVtkLineProfileWidget_H_
#define QmitkVtkLineProfileWidget_H_

#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"

#include <qlayout.h>
#include <qtextedit.h>
#include <qdialog.h>

#include <itkParametricPath.h>
#include <itkPolyLineParametricPath.h>

#include <vtkQtChartWidget.h>
#include <vtkQtLineChart.h>

#include <QStandardItemModel>




/** 
 * \brief Widget for displaying intensity profiles of images along a given
 * path
 */
class QmitkExt_EXPORT QmitkVtkLineProfileWidget : public QDialog
{
  Q_OBJECT

public:
  QmitkVtkLineProfileWidget( QWidget *parent = 0 );
  virtual ~QmitkVtkLineProfileWidget();

  typedef itk::ParametricPath< 3 >::Superclass PathType;
  typedef itk::PolyLineParametricPath< 3 > ParametricPathType;

  enum
  {
    PATH_MODE_DIRECT = 0,
    PATH_MODE_PLANARFIGURE
  };


  /** \brief Set image from which to calculate derive the intensity profile. */
  void SetImage(mitk::Image* image);

  /** \brief Set path for calculating intensity profile directly. */
  void SetPath(const PathType* path);

  /** \brief Set planar figure for calculating intensity profile. */
  void SetPlanarFigure(const mitk::PlanarFigure* planarFigure);

  /** \brief Set/Get mode which path to use */
  void SetPathMode(unsigned int pathMode);
  
  /** \brief Set/Get mode which path to use */
  unsigned int GetPathMode();

  /** \brief Set/Get mode which path to use */
  void SetPathModeToDirectPath();

  /** \brief Set/Get mode which path to use */
  void SetPathModeToPlanarFigure();


  /** Fill the graphical widget with intensity profile from currently specified image/path . */
  void UpdateItemModelFromPath();

  /** \brief Clear the intensity profile (nothing is displayed). */
  void ClearItemModel();


protected slots:


protected:

  void CreatePathFromPlanarFigure();

  void ComputePath();

  vtkQtChartWidget *m_ChartWidget;
  vtkQtLineChart *m_LineChart;

  QStandardItemModel *m_ItemModel;

  mitk::Image::Pointer m_Image;

  mitk::PlanarFigure::ConstPointer m_PlanarFigure;

  unsigned int m_PathMode;


  // Path set explicitly by user
  PathType::ConstPointer m_Path;

  // Parametric path as generated from PlanarFigure
  ParametricPathType::Pointer m_ParametricPath;

  // Path derived either form user-specified path or from PlanarFigure-generated
  // path
  PathType::ConstPointer m_DerivedPath;

};

#endif /* QmitkVtkLineProfileWidget_H_ */
