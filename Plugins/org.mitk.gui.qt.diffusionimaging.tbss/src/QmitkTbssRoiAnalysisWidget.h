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

#include <org_mitk_gui_qt_diffusionimaging_tbss_Export.h>

#include <mitkFiberBundle.h>
#include <mitkTbssImage.h>

#include <itkVectorImage.h>

typedef itk::VectorImage<float,3>     VectorImageType;
typedef std::vector< itk::Index<3> > RoiType;

typedef mitk::Point3D                     PointType;
typedef std::vector< PointType>           TractType;
typedef std::vector< TractType > TractContainerType;

class QwtPlotPicker;

/**
 * \brief Plot widget for TBSS Data
 * This widget can plot regions of interest on TBSS projection data. The projection data is created by importing FSL TBSS subject data and
 * completing it with patient data using the QmitkTractbasedSpatialStatisticsView.
 * The region of interest is a vector of indices from which data for plotting should be obtained.
 */

class DIFFUSIONIMAGING_TBSS_EXPORT QmitkTbssRoiAnalysisWidget : public QmitkPlotWidget
{

Q_OBJECT

public:


  QmitkTbssRoiAnalysisWidget( QWidget * parent);
  virtual ~QmitkTbssRoiAnalysisWidget();


  /* \brief Set group information as a vector of pairs of group name and number of group members */
  void SetGroups(std::vector< std::pair<std::string, int> > groups)
  {
    m_Groups = groups;
  }

  /* \brief Draws the group averaged profiles */
  void DrawProfiles();


  void PlotFiber4D(mitk::TbssImage::Pointer tbssImage,
                   mitk::FiberBundle *fib,
                   mitk::DataNode *startRoi,
                   mitk::DataNode *endRoi,
                   int number);


  template <typename T>
  void PlotFiberBundles(const mitk::PixelType, TractContainerType tracts, mitk::Image* img, bool avg=false);


  /* \brief Sets the projections of the individual subjects */
  void SetProjections(VectorImageType::Pointer projections)
  {
    m_Projections = projections;
  }

  /* \brief Set the region of interest*/
  void SetRoi(RoiType roi)
  {
    m_Roi = roi;
  }

  /* \brief Set structure information to display in the plot */
  void SetStructure(std::string structure)
  {
    m_Structure = structure;
  }

  /* \brief Set measurement type for display in the plot */
  void SetMeasure(std::string measure)
  {
    m_Measure = measure;
  }

  /* \brief Draws a bar to indicate were the user clicked in the plot */
  void drawBar(int x);



  /* \brief Returns the values of the group averaged profiles */
  std::vector <std::vector<double> > GetVals()
  {
    return m_Vals;
  }


  /* \brief Returns the values of the individual subjects profiles */
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


  void PlotFiberBetweenRois(mitk::FiberBundle *fib, mitk::Image* img,
                            mitk::DataNode *startRoi, mitk::DataNode *endRoi, bool avg=-1, int number=25);




  // Takes an index which is an x coordinate from the plot and finds the corresponding position in world space
  mitk::Point3D GetPositionInWorld(int index);
  void ModifyPlot(int number, bool avg);



  QwtPlotPicker* m_PlotPicker;

protected:

  mitk::FiberBundle* m_Fib;


  std::vector< std::vector<double> > m_Vals;

  std::vector< std::vector<double> > m_IndividualProfiles;
  std::vector< double > m_Average;



  std::vector< std::vector<double> > CalculateGroupProfiles();
  std::vector< std::vector<double> > CalculateGroupProfilesFibers(mitk::TbssImage::Pointer tbssImage,
                                                                  mitk::FiberBundle *fib,
                                                                  mitk::DataNode* startRoi,
                                                                  mitk::DataNode* endRoi,
                                                                  int number);

  void Plot(std::vector <std::vector<double> > groupProfiles);


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
  mitk::TbssImage* m_CurrentTbssImage;

  mitk::DataNode* m_CurrentStartRoi;
  mitk::DataNode* m_CurrentEndRoi;


  void DoPlotFiberBundles(mitk::FiberBundle *fib, mitk::Image* img,
                          mitk::DataNode* startRoi, mitk::DataNode* endRoi, bool avg=false, int number=25);



  /* \brief Creates tracts from a mitk::FiberBundle and two planar figures indicating the start end end point */
  TractContainerType CreateTracts(mitk::FiberBundle *fib, mitk::DataNode* startRoi, mitk::DataNode* endRoi);



};

#endif
