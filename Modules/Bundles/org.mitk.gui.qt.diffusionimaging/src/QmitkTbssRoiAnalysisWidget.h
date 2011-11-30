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

#ifndef QmitkTbssRoiAnalysisWidget_H_
#define QmitkTbssRoiAnalysisWidget_H_

#include "QmitkPlotWidget.h"

#include <org_mitk_gui_qt_diffusionimaging_Export.h>

//#include "QmitkHistogram.h"
#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include "itkVectorImage.h"


//#include <itkHistogram.h>

//#include <vtkQtChartWidget.h>
#include <vtkQtBarChart.h>
#include <QStandardItemModel>
#include <qwt_plot.h>
#include <QPainter>
#include <qwt_plot_picker.h>

typedef itk::VectorImage<float,3>     VectorImageType;

typedef std::vector< itk::Index<3> > RoiType;

/** 
 * \brief Widget for displaying boxplots
 * framework
 */
class DIFFUSIONIMAGING_EXPORT QmitkTbssRoiAnalysisWidget : public QmitkPlotWidget
{

Q_OBJECT

public:


  QmitkTbssRoiAnalysisWidget( QWidget * parent);
  virtual ~QmitkTbssRoiAnalysisWidget();

  void SetGroups(std::vector< std::pair<std::string, int> > groups)
  {
    m_Groups = groups;
  }

  void DrawProfiles(std::string preprocessed);

  void Boxplots();

  void SetProjections(VectorImageType::Pointer projections)
  {
    m_Projections = projections;
  }

  void SetRoi(RoiType roi)
  {
    m_Roi = roi;
  }

  void SetStructure(std::string structure)
  {
    m_Structure = structure;
  }

  void SetMeasure(std::string measure)
  {
    m_Measure = measure;
  }

  QwtPlot* GetPlot()
  {
    return m_Plot;
  }

  QwtPlotPicker* m_PlotPicker;



  void drawBar(int x);

  std::vector <std::vector<double> > GetVals()
  {
    return m_Vals;
  }


protected:

  std::vector< std::vector<double> > m_Vals;




  std::vector< std::vector<double> > CalculateGroupProfiles(std::string preprocessed);



  void Tokenize(const std::string& str,
                std::vector<std::string>& tokens,
                const std::string& delimiters = " ")
  {
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
  }

  std::vector< std::pair<std::string, int> > m_Groups;

  VectorImageType::Pointer m_Projections;
  RoiType m_Roi;
  std::string m_Structure;
  std::string m_Measure;



};

#endif
