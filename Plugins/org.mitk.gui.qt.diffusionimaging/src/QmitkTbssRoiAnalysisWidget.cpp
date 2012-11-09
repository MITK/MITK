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

#include "QmitkTbssRoiAnalysisWidget.h"


#include <qlabel.h>
#include <qpen.h>
#include <qgroupbox.h>

#include <qwt_text.h>
#include <qwt_plot_grid.h>
#include <qwt_painter.h>
#include <qwt_legend.h>
#include <qwt_plot_marker.h>


#include <iostream>
#include <fstream>
#include <limits>

#include <vtkCellArray.h>



QmitkTbssRoiAnalysisWidget::QmitkTbssRoiAnalysisWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  m_PlotPicker = new QwtPlotPicker(m_Plot->canvas());
  m_PlotPicker->setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection | QwtPicker::DragSelection);
  m_PlotPicker->setTrackerMode(QwtPicker::ActiveOnly);

  m_PlottingFiberBundle = false;
}



void QmitkTbssRoiAnalysisWidget::DoPlotFiberBundles(mitk::FiberBundleX *fib, mitk::Image* img,
                                                    mitk::PlanarFigure* startRoi, mitk::PlanarFigure* endRoi, bool avg, int number)
{

  mitk::Geometry3D* currentGeometry = fib->GetGeometry();

  mitk::PlaneGeometry* startGeometry2D = dynamic_cast<mitk::PlaneGeometry*>( const_cast<mitk::Geometry2D*>(startRoi->GetGeometry2D()) );
  mitk::PlaneGeometry* endGeometry2D = dynamic_cast<mitk::PlaneGeometry*>( const_cast<mitk::Geometry2D*>(endRoi->GetGeometry2D()) );


  mitk::Point3D startCenter = startRoi->GetWorldControlPoint(0); //center Point of start roi
  mitk::Point3D endCenter = endRoi->GetWorldControlPoint(0); //center Point of end roi

  mitk::FiberBundleX::Pointer inStart = fib->ExtractFiberSubset(startRoi);
  mitk::FiberBundleX::Pointer inBoth = inStart->ExtractFiberSubset(endRoi);



  int num = inBoth->GetNumFibers();


  TractContainerType tracts;


  vtkSmartPointer<vtkPolyData> fiberPolyData = inBoth->GetFiberPolyData();
  vtkCellArray* lines = fiberPolyData->GetLines();
  lines->InitTraversal();



  // Now find out for each fiber which ROI is encountered first. If this is the startRoi, the direction is ok
  // Otherwise the plot should be in the reverse direction
  for( int fiberID( 0 ); fiberID < num; fiberID++ )
  {
    vtkIdType   numPointsInCell(0);
    vtkIdType*  pointsInCell(NULL);
    lines->GetNextCell ( numPointsInCell, pointsInCell );

    int startId = 0;
    int endId = 0;

    float minDistStart = std::numeric_limits<float>::max();
    float minDistEnd = std::numeric_limits<float>::max();



    for( int pointInCellID( 0 ); pointInCellID < numPointsInCell ; pointInCellID++)
    {


      double *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );

      mitk::Point3D point;
      point[0] = p[0];
      point[1] = p[1];
      point[2] = p[2];

      float distanceToStart = point.EuclideanDistanceTo(startCenter);
      float distanceToEnd = point.EuclideanDistanceTo(endCenter);

      if(distanceToStart < minDistStart)
      {
        minDistStart = distanceToStart;
        startId = pointInCellID;
      }

      if(distanceToEnd < minDistEnd)
      {
        minDistEnd = distanceToEnd;
        endId = pointInCellID;
      }



    }

    /* We found the start and end points of of the part that should be plottet for
       the current fiber. now we need to plot them. If the endId is smaller than the startId the plot order
       must be reversed*/

    TractType singleTract;
    PointType point;

    if(startId < endId)
    {

      // Calculate the intersection of the ROI with the startRoi and decide if the startId is part of the roi or must be cut of
      double *p = fiberPolyData->GetPoint( pointsInCell[ startId ] );
      mitk::Vector3D p0;
      p0[0] = p[0];      p0[1] = p[1];      p0[2] = p[2];

      p = fiberPolyData->GetPoint( pointsInCell[ startId+1 ] );
      mitk::Vector3D p1;
      p1[0] = p[0];       p1[1] = p[1];       p1[2] = p[2];

      // Check if p and p2 are both on the same side of the plane
      mitk::Vector3D normal = startGeometry2D->GetNormal();

      mitk::Point3D pStart;
      pStart[0] = p0[0];      pStart[1] = p0[1];      pStart[2] = p0[2];

      mitk::Point3D pSecond;
      pSecond[0] = p1[0];      pSecond[1] = p1[1];      pSecond[2] = p1[2];

      bool startOnPositive = startGeometry2D->IsAbove(pStart);
      bool secondOnPositive = startGeometry2D->IsAbove(pSecond);


      mitk::Vector3D onPlane;
      onPlane[0] = startCenter[0];      onPlane[1] = startCenter[1];      onPlane[2] = startCenter[2];



      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* startId and startId+1 lie on the same side of the plane, so we need
           need startId-1 to calculate the intersection with the planar figure*/
        p = fiberPolyData->GetPoint( pointsInCell[ startId-1 ] );
        p1[0] = p[0];        p1[1] = p[1];        p1[2] = p[2];
      }


      double d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );
      mitk::Vector3D newPoint = (p0-p1);

      point[0] = d*newPoint[0] + p0[0];
      point[1] = d*newPoint[1] + p0[1];
      point[2] = d*newPoint[2] + p0[2];

      singleTract.push_back(point);

      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* StartId and startId+1 lie on the same side of the plane
           so startId is also part of the ROI*/

        double *start = fiberPolyData->GetPoint( pointsInCell[startId] );
        point[0] = start[0]; point[1] = start[1]; point[2] = start[2];
        singleTract.push_back(point);
      }



      for( int pointInCellID( startId+1 ); pointInCellID < endId ; pointInCellID++)
      {
        // push back point
        double *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );
        point[0] = p[0]; point[1] = p[1]; point[2] = p[2];
        singleTract.push_back( point );

      }

      /* endId must be included if endId and endId-1 lie on the same side of the
               plane defined by endRoi*/


      p = fiberPolyData->GetPoint( pointsInCell[ endId ] );
      p0[0] = p[0];      p0[1] = p[1];      p0[2] = p[2];

      p = fiberPolyData->GetPoint( pointsInCell[ endId-1 ] );
      p1[0] = p[0];      p1[1] = p[1];      p1[2] = p[2];

      mitk::Point3D pLast;
      pLast[0] = p0[0];      pLast[1] = p0[1];      pLast[2] = p0[2];

      mitk::Point3D pBeforeLast;
      pBeforeLast[0] = p1[0];      pBeforeLast[1] = p1[1];      pBeforeLast[2] = p1[2];

      normal = endGeometry2D->GetNormal();
      bool lastOnPositive = endGeometry2D->IsAbove(pLast);
      bool secondLastOnPositive = endGeometry2D->IsAbove(pBeforeLast);


      onPlane[0] = endCenter[0];
      onPlane[1] = endCenter[1];
      onPlane[2] = endCenter[2];

      if(! (lastOnPositive ^ secondLastOnPositive) )
      {
        /* endId and endId-1 lie on the same side of the plane, so we need
           need endId+1 to calculate the intersection with the planar figure.
           this should exist since we know that the fiber crosses the planar figure
           endId is also part of the tract and can be inserted here */

        p = fiberPolyData->GetPoint( pointsInCell[ endId ] );
        point[0] = p[0]; point[1] = p[1]; point[2] = p[2];
        singleTract.push_back( point );

        p = fiberPolyData->GetPoint( pointsInCell[ endId+1 ] );
      }

      d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );

      newPoint = (p0-p1);

      point[0] = d*newPoint[0] + p0[0];
      point[1] = d*newPoint[1] + p0[1];
      point[2] = d*newPoint[2] + p0[2];

      singleTract.push_back(point);



    }
    else{

      // Calculate the intersection of the ROI with the startRoi and decide if the startId is part of the roi or must be cut of
      double *p = fiberPolyData->GetPoint( pointsInCell[ startId ] );
      mitk::Vector3D p0;
      p0[0] = p[0];      p0[1] = p[1];      p0[2] = p[2];

      p = fiberPolyData->GetPoint( pointsInCell[ startId-1 ] );
      mitk::Vector3D p1;
      p1[0] = p[0];       p1[1] = p[1];       p1[2] = p[2];

      // Check if p and p2 are both on the same side of the plane
      mitk::Vector3D normal = startGeometry2D->GetNormal();

      mitk::Point3D pStart;
      pStart[0] = p0[0];      pStart[1] = p0[1];      pStart[2] = p0[2];

      mitk::Point3D pSecond;
      pSecond[0] = p1[0];      pSecond[1] = p1[1];      pSecond[2] = p1[2];

      bool startOnPositive = startGeometry2D->IsAbove(pStart);
      bool secondOnPositive = startGeometry2D->IsAbove(pSecond);

      mitk::Vector3D onPlane;
      onPlane[0] = startCenter[0];      onPlane[1] = startCenter[1];      onPlane[2] = startCenter[2];


      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* startId and startId+1 lie on the same side of the plane, so we need
           need startId-1 to calculate the intersection with the planar figure*/
        p = fiberPolyData->GetPoint( pointsInCell[ startId-1 ] );
        p1[0] = p[0];        p1[1] = p[1];        p1[2] = p[2];
      }


      double d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );
      mitk::Vector3D newPoint = (p0-p1);


      point[0] = d*newPoint[0] + p0[0];
      point[1] = d*newPoint[1] + p0[1];
      point[2] = d*newPoint[2] + p0[2];


      singleTract.push_back(point);

      if(! (secondOnPositive ^ startOnPositive) )
      {
        /* StartId and startId+1 lie on the same side of the plane
           so startId is also part of the ROI*/

        double *start = fiberPolyData->GetPoint( pointsInCell[startId] );
        point[0] = start[0]; point[1] = start[1]; point[2] = start[2];
        singleTract.push_back(point);
      }



      for( int pointInCellID( startId-1 ); pointInCellID > endId ; pointInCellID--)
      {
        // push back point
        double *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );
        point[0] = p[0]; point[1] = p[1]; point[2] = p[2];
        singleTract.push_back( point );

      }

      /* endId must be included if endId and endI+1 lie on the same side of the
               plane defined by endRoi*/


      p = fiberPolyData->GetPoint( pointsInCell[ endId ] );
      p0[0] = p[0];      p0[1] = p[1];      p0[2] = p[2];

      p = fiberPolyData->GetPoint( pointsInCell[ endId+1 ] );
      p1[0] = p[0];      p1[1] = p[1];      p1[2] = p[2];

      mitk::Point3D pLast;
      pLast[0] = p0[0];      pLast[1] = p0[1];      pLast[2] = p0[2];

      mitk::Point3D pBeforeLast;
      pBeforeLast[0] = p1[0];      pBeforeLast[1] = p1[1];      pBeforeLast[2] = p1[2];

      bool lastOnPositive = endGeometry2D->IsAbove(pLast);
      bool secondLastOnPositive = endGeometry2D->IsAbove(pBeforeLast);
      normal = endGeometry2D->GetNormal();



      onPlane[0] = endCenter[0];
      onPlane[1] = endCenter[1];
      onPlane[2] = endCenter[2];

      if(! (lastOnPositive ^ secondLastOnPositive) )
      {
        /* endId and endId+1 lie on the same side of the plane, so we need
           need endId-1 to calculate the intersection with the planar figure.
           this should exist since we know that the fiber crosses the planar figure
           endId is also part of the tract and can be inserted here */

        p = fiberPolyData->GetPoint( pointsInCell[ endId ] );
        point[0] = p[0]; point[1] = p[1]; point[2] = p[2];
        singleTract.push_back( point );

        p = fiberPolyData->GetPoint( pointsInCell[ endId-1 ] );
      }

      d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );

      newPoint = (p0-p1);

      point[0] = d*newPoint[0] + p0[0];
      point[1] = d*newPoint[1] + p0[1];
      point[2] = d*newPoint[2] + p0[2];

      singleTract.push_back(point);

    }


    tracts.push_back(singleTract);


  }


  //todo: Make number of samples selectable by user
  TractContainerType resampledTracts = ParameterizeTracts(tracts, number);

  // Now we have the resampled tracts. Next we should use these points to read out the values



  PlotFiberBundles(resampledTracts, img, avg);
  m_CurrentTracts = resampledTracts;
}

void QmitkTbssRoiAnalysisWidget::PlotFiberBetweenRois(mitk::FiberBundleX *fib, mitk::Image* img,
                                mitk::PlanarFigure* startRoi, mitk::PlanarFigure* endRoi, bool avg, int number)
{

  if(fib == NULL || img == NULL || startRoi == NULL || endRoi == NULL)
    return;


  m_Fib = fib;
  m_CurrentImage = img;
  m_CurrentStartRoi = startRoi;
  m_CurrentEndRoi = endRoi;


  DoPlotFiberBundles(fib, img, startRoi, endRoi, avg, number);
}



void QmitkTbssRoiAnalysisWidget::ModifyPlot(int number, bool avg)
{
  if(m_Fib == NULL || m_CurrentImage == NULL || m_CurrentStartRoi == NULL || m_CurrentEndRoi == NULL)
    return;

  DoPlotFiberBundles(m_Fib, m_CurrentImage, m_CurrentStartRoi, m_CurrentEndRoi, avg, number);


}



TractContainerType QmitkTbssRoiAnalysisWidget::ParameterizeTracts(TractContainerType tracts, int number)
{
  TractContainerType resampledTracts;



  for(TractContainerType::iterator it = tracts.begin(); it != tracts.end(); ++it)
  {
    TractType resampledTract;
    TractType tract = *it;

    // Calculate the total length
    float totalLength = 0;

    if(tract.size() < 2)
      continue;

    PointType p0 = tract.at(0);
    for(int i = 1; i<tract.size(); i++)
    {
      PointType p1 = tract.at(i);
      float length = p0.EuclideanDistanceTo(p1);
      totalLength += length;
      p0 = p1;
    }

    float stepSize = totalLength / number;



    p0 = tract.at(0);
    PointType p1 = tract.at(1);
    int tractCounter = 2;
    float distance = p0.EuclideanDistanceTo(p1);
    float locationBetween = 0;

    for(float position = 0;
        position <= totalLength+0.001 && resampledTract.size() <= (number+1);
        position+=stepSize)
    {

      /* In case we walked to far we need to find the next segment we are on and on what relative position on that
         tract we are on. Small correction for rounding errors */
      while(locationBetween > distance+0.001)
      {

        if(tractCounter == tract.size())
          std::cout << "problem";

        // Determine by what distance we are no on the next segment
        locationBetween = locationBetween - distance;
        p0 = p1;
        p1 = tract.at(tractCounter);
        tractCounter++;

        distance = p0.EuclideanDistanceTo(p1);

      }

      // Direction
      PointType::VectorType direction = p1-p0;
      direction.Normalize();

      PointType newSample = p0 + direction*locationBetween;
      resampledTract.push_back(newSample);


      locationBetween += stepSize;


    }

    resampledTracts.push_back(resampledTract);


  }
  return resampledTracts;
}


mitk::Point3D QmitkTbssRoiAnalysisWidget::GetPositionInWorld(int index)
{
  TractContainerType tractsAtIndex;

  float xSum = 0.0;
  float ySum = 0.0;
  float zSum = 0.0;
  for(TractContainerType::iterator it = m_CurrentTracts.begin();
      it!=m_CurrentTracts.end(); ++it)
  {
    TractType tract = *it;
    PointType p = tract.at(index);
    xSum += p[0];
    ySum += p[1];
    zSum += p[2];
  }

  int number = m_CurrentTracts.size();

  float xPos = xSum / number;
  float yPos = ySum / number;
  float zPos = zSum / number;


  mitk::Point3D pos;
  pos[0] = xPos;
  pos[1] = yPos;
  pos[2] = zPos;

  return pos;
}


std::vector< std::vector<double> > QmitkTbssRoiAnalysisWidget::CalculateGroupProfiles(std::string preprocessed)
{
  MITK_INFO << "make profiles!";
  std::vector< std::vector<double> > profiles;

  //No results were preprocessed, so they must be calculated now.
  if(preprocessed == "")
  {
  // Iterate through the 4th dim (corresponding to subjects)
  // and create a profile for every subject
    int size = m_Projections->GetVectorLength();
    for(int s=0; s<size; s++)
    {
      // Iterate trough the roi
      std::vector<double> profile;
      RoiType::iterator it;
      it = m_Roi.begin();
      while(it != m_Roi.end())
      {
        itk::Index<3> ix = *it;
        profile.push_back(m_Projections->GetPixel(ix).GetElement(s));
        it++;
      }
      int pSize = profile.size();
      profiles.push_back(profile);
    }
  }
  else{ // Use preprocessed results
    std::ifstream file(preprocessed.c_str());
    if(file.is_open())
    {
      std::string line;
      while(getline(file,line))
      {
        std::vector<std::string> tokens;
        Tokenize(line, tokens);
        std::vector<std::string>::iterator it;
        it = tokens.begin();
        std::vector< double > profile;
        while(it != tokens.end())
        {
          std::string s = *it;
          profile.push_back (atof( s.c_str() ) );
          ++it;
        }
        profiles.push_back(profile);
      }
    }
  }


  m_IndividualProfiles = profiles;
  // Calculate the averages

  // Here a check could be build in to check whether all profiles have
  // the same length, but this should normally be the case if the input
  // data were corrected with the TBSS Module.

  std::vector< std::vector<double> > groupProfiles;

  std::vector< std::pair<std::string, int> >::iterator it;
  it = m_Groups.begin();

  int c = 0; //the current profile number

  int nprof = profiles.size();

  while(it != m_Groups.end() && profiles.size() > 0)
  {
    std::pair<std::string, int> p = *it;
    int size = p.second;

    //initialize a vector of the right length with zeroes
    std::vector<double> averageProfile;
    for(int i=0; i<profiles.at(0).size(); i++)
    {
      averageProfile.push_back(0.0);
    }

    // Average the right number of profiles

    for(int i=0; i<size; i++)
    {
      for(int j=0; j<averageProfile.size(); ++j)
      {
        averageProfile.at(j) = averageProfile.at(j) + profiles.at(c).at(j);
      }
      c++;
    }

    // Devide by the number of profiles to get group average
    for(int i=0; i<averageProfile.size(); i++)
    {
      averageProfile.at(i) = averageProfile.at(i) / size;
    }

    groupProfiles.push_back(averageProfile);

    ++it;
  }
  int pSize = groupProfiles.size();

  return groupProfiles;
}


void QmitkTbssRoiAnalysisWidget::DrawProfiles(std::string preprocessed)
{
  this->Clear();



  m_Vals.clear();

  std::vector<double> v1;


  std::vector <std::vector<double> > groupProfiles = CalculateGroupProfiles(preprocessed);


  std::vector<double> xAxis;
  for(int i=0; i<groupProfiles.at(0).size(); ++i)
  {
    xAxis.push_back((double)i);
  }


  // fill m_Vals. This might be used by the user to copy data to the clipboard
  for(int i=0; i<groupProfiles.size(); i++)
  {

    v1.clear();


    for(int j=0; j<groupProfiles.at(i).size(); j++)
    {
      v1.push_back(groupProfiles.at(i).at(j));
    }

    m_Vals.push_back(v1);

  }

  std::string title = m_Measure + " profiles on the ";
  title.append(m_Structure);
  this->SetPlotTitle( title.c_str() );
  QPen pen( Qt::SolidLine );
  pen.setWidth(2);



  std::vector< std::pair<std::string, int> >::iterator it;
  it = m_Groups.begin();

  int c = 0; //the current profile number

  QColor colors[4] = {Qt::green, Qt::blue, Qt::yellow, Qt::red};

  while(it != m_Groups.end() && groupProfiles.size() > 0)
  {

    std::pair< std::string, int > group = *it;

    pen.setColor(colors[c]);
    int curveId = this->InsertCurve( group.first.c_str() );
    this->SetCurveData( curveId, xAxis, groupProfiles.at(c) );


    this->SetCurvePen( curveId, pen );

    c++;
    it++;

  }


  QwtLegend *legend = new QwtLegend;
  this->SetLegend(legend, QwtPlot::RightLegend, 0.5);


  std::cout << m_Measure << std::endl;
  this->m_Plot->setAxisTitle(0, m_Measure.c_str());
  this->m_Plot->setAxisTitle(3, "Position");

  this->Replot();

}



void QmitkTbssRoiAnalysisWidget::PlotFiberBundles(TractContainerType tracts, mitk::Image *img, bool avg)
{
  this->Clear();
  std::vector<TractType>::iterator it = tracts.begin();


  // Match points on tracts. Take the smallest tract and match all others on this one


  /*

  int min = std::numeric_limits<int>::max();
  TractType smallestTract;
  while(it != tracts.end())
  {
    TractType tract = *it;
    if(tract.size()<min)
    {
      smallestTract = tract;
      min = tract.size();
    }
    ++it;
  }


  it = tracts.begin();
  while(it != tracts.end())
  {
    TractType tract = *it;
    if(tract == smallestTract)
    {
      ++it;
      continue;
    }

    // Finding correspondences between points
    std::vector<int> correspondingIndices;
    TractType correspondingPoints;

    for(int i=0; i<smallestTract.size(); i++)
    {
      PointType p = smallestTract[i];

      double minDist = std::numeric_limits<float>::max();
      int correspondingIndex = 0;
      PointType correspondingPoint;

      // Search for the point on the second tract with the smallest distance
      // to p and memorize it
      for(int j=0; j<tract.size(); j++)
      {
        PointType p2 = tract[j];
        double dist = fabs( p.EuclideanDistanceTo(p2) );
        if(dist < minDist)
        {
          correspondingIndex = j;
          correspondingPoint = p2;
          minDist = dist;
        }
      }


      correspondingIndices.push_back(correspondingIndex);
      correspondingPoints.push_back(correspondingPoint);


    }

    if(smallestTract.size() != correspondingIndices.size())
    {

      MITK_ERROR << "smallest tract and correspondingIndices have no equal size";
      continue;
    }


    std::cout << "corresponding points\n";

    for(int i=0; i<smallestTract.size(); i++)
    {
      PointType p = smallestTract[i];
      PointType p2 = correspondingPoints[i];

      std::cout << "[" << p[0] << ", " << p[1] << ", " << p[2] << ", ["
                   << p2[0] << ", " << p2[1] << ", " << p2[2] << "]\n";
    }


    std::cout << std::endl;

    ++it;
  }
*/



  // Get the values along the curves from a 3D images. should also contain info about the position on screen.
  std::vector< std::vector <double > > profiles;

  it = tracts.begin();
  while(it != tracts.end())
  {
    std::cout << "Tract\n";
    TractType tract = *it;
    TractType::iterator tractIt = tract.begin();

    std::vector<double> profile;

    while(tractIt != tract.end())
    {
      PointType p = *tractIt;
      std::cout << p[0] << ' ' << p[1] << ' ' << p[2] << '\n';


      // Get value from image
      profile.push_back( (double)img->GetPixelValueByWorldCoordinate(p) );

      ++tractIt;
    }

    profiles.push_back(profile);
    std::cout << std::endl;

    ++it;
  }


  if(profiles.size() == 0)
    return;

  m_IndividualProfiles = profiles;


  std::string title = "Fiber bundle plot";
  this->SetPlotTitle( title.c_str() );


  // initialize average profile
  std::vector<double> averageProfile;
  std::vector<double> profile = profiles.at(0); // can do this because we checked the size of profiles before
  for(int i=0; i<profile.size(); ++i)
  {
    averageProfile.push_back(0.0);
  }


  std::vector< std::vector<double> >::iterator profit = profiles.begin();

  int id=0;
  while(profit != profiles.end())
  {
    std::vector<double> profile = *profit;



    std::vector<double> xAxis;
    for(int i=0; i<profile.size(); ++i)
    {
      xAxis.push_back((double)i);
      averageProfile.at(i) += profile.at(i) / profiles.size();
    }

    int curveId = this->InsertCurve( QString::number(id).toStdString().c_str() );

    this->SetCurveData( curveId, xAxis, profile );

    ++profit;
    id++;


  }

  m_Average = averageProfile;


  if(avg)
  {

    // Draw the average profile
    std::vector<double> xAxis;
    for(int i=0; i<averageProfile.size(); ++i)
    {
      xAxis.push_back((double)i);
    }

    int curveId = this->InsertCurve( QString::number(id).toStdString().c_str() );
    this->SetCurveData( curveId, xAxis, averageProfile );




    QPen pen( Qt::SolidLine );
    pen.setWidth(3);
    pen.setColor(Qt::red);

    this->SetCurvePen( curveId, pen );



    id++;


  }

  this->Replot();





}


void QmitkTbssRoiAnalysisWidget::Boxplots()
{
  this->Clear();
}

void QmitkTbssRoiAnalysisWidget::drawBar(int x)
{

  m_Plot->detachItems(QwtPlotItem::Rtti_PlotMarker, true);


  QwtPlotMarker *mX = new QwtPlotMarker();
  //mX->setLabel(QString::fromLatin1("selected point"));
  mX->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);
  mX->setLabelOrientation(Qt::Vertical);
  mX->setLineStyle(QwtPlotMarker::VLine);
  mX->setLinePen(QPen(Qt::black, 0, Qt::SolidLine));
  mX->setXValue(x);
  mX->attach(m_Plot);


  this->Replot();

}

QmitkTbssRoiAnalysisWidget::~QmitkTbssRoiAnalysisWidget()
{
  delete m_PlotPicker;

}




