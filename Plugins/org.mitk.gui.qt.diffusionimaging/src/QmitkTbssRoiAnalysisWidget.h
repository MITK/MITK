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

#ifndef QmitkTbssRoiAnalysisWidget_H_
#define QmitkTbssRoiAnalysisWidget_H_

#include "QmitkPlotWidget.h"

#include <org_mitk_gui_qt_diffusionimaging_Export.h>

//#include "QmitkHistogram.h"
#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include "itkVectorImage.h"
#include <mitkFiberBundleX.h>
#include <mitkPlanarCircle.h>


//#include <itkHistogram.h>

//#include <vtkQtChartWidget.h>
#include <vtkQtBarChart.h>
#include <QStandardItemModel>
#include <qwt_plot.h>
#include <QPainter>
#include <qwt_plot_picker.h>

typedef itk::VectorImage<float,3>     VectorImageType;

typedef std::vector< itk::Index<3> > RoiType;


typedef itk::Point<float,3>               PointType;
typedef std::vector< PointType>           TractType;
typedef std::vector< TractType > TractContainerType;


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


  void PlotFiberBundles(TractContainerType tracts, mitk::Image* img, bool avg=false);


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

  std::vector <std::vector<double> > GetIndividualProfiles()
  {
    return m_IndividualProfiles;
  }


  std::vector<double> GetAverageProfile()
  {
    return m_Average;
  }


  void SetPlottingFiber(bool b)
  {
    m_PlottingFiberBundle = b;
  }

  bool IsPlottingFiber()
  {
    return m_PlottingFiberBundle;
  }


  void PlotFiberBetweenRois(mitk::FiberBundleX *fib, mitk::Image* img,
                            mitk::PlanarFigure* startRoi, mitk::PlanarFigure* endRoi, bool avg=-1, int number=25);




  // Takes an index which is an x coordinate from the plot and finds the corresponding position in world space
  mitk::Point3D GetPositionInWorld(int index);
  void ModifyPlot(int number, bool avg);

protected:

  mitk::FiberBundleX* m_Fib;


  std::vector< std::vector<double> > m_Vals;

  std::vector< std::vector<double> > m_IndividualProfiles;
  std::vector< double > m_Average;





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

  bool m_PlottingFiberBundle; // true when the plot results from a fiber tracking result (vtk .fib file)


  // Resample a collection of tracts so that every tract contains #number equidistant samples
  TractContainerType ParameterizeTracts(TractContainerType tracts, int number);




  TractContainerType m_CurrentTracts;



  mitk::Image* m_CurrentImage;

  mitk::PlanarFigure* m_CurrentStartRoi;
  mitk::PlanarFigure* m_CurrentEndRoi;


  void DoPlotFiberBundles(mitk::FiberBundleX *fib, mitk::Image* img,
                          mitk::PlanarFigure* startRoi, mitk::PlanarFigure* endRoi, bool avg=false, int number=25);






};

#endif
