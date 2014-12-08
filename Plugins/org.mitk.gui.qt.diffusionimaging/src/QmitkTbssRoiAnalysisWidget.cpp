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
#include "mitkImagePixelReadAccessor.h"
#include "mitkPixelTypeMultiplex.h"

#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

#include <vtkCellArray.h>

QmitkTbssRoiAnalysisWidget::QmitkTbssRoiAnalysisWidget( QWidget * parent )
  : QmitkPlotWidget(parent)
{
  m_PlotPicker = new QwtPlotPicker(m_Plot->canvas());
  m_PlotPicker->setStateMachine(new QwtPickerDragPointMachine());
  m_PlotPicker->setTrackerMode(QwtPicker::ActiveOnly);

  m_PlottingFiberBundle = false;
}



void QmitkTbssRoiAnalysisWidget::DoPlotFiberBundles(mitk::FiberBundleX *fib, mitk::Image* img,
                                                    mitk::PlanarFigure* startRoi, mitk::PlanarFigure* endRoi, bool avg, int number)
{

  TractContainerType tracts = CreateTracts(fib, startRoi, endRoi);

  TractContainerType resampledTracts = ParameterizeTracts(tracts, number);

  // Now we have the resampled tracts. Next we should use these points to read out the values



  mitkPixelTypeMultiplex3(PlotFiberBundles,img->GetImageDescriptor()->GetChannelTypeById(0),resampledTracts, img, avg);
  m_CurrentTracts = resampledTracts;
}


TractContainerType QmitkTbssRoiAnalysisWidget::CreateTracts(mitk::FiberBundleX *fib,
                                                            mitk::PlanarFigure *startRoi,
                                                            mitk::PlanarFigure *endRoi)
{
    mitk::PlaneGeometry* startGeometry2D = const_cast<mitk::PlaneGeometry*>(startRoi->GetPlaneGeometry());
    mitk::PlaneGeometry* endGeometry2D = const_cast<mitk::PlaneGeometry*>(endRoi->GetPlaneGeometry());


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

      mitk::ScalarType minDistStart = std::numeric_limits<mitk::ScalarType>::max();
      mitk::ScalarType minDistEnd = std::numeric_limits<mitk::ScalarType>::max();



      for( int pointInCellID( 0 ); pointInCellID < numPointsInCell ; pointInCellID++)
      {


        mitk::ScalarType *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );

        mitk::Point3D point;
        point[0] = p[0];
        point[1] = p[1];
        point[2] = p[2];

        mitk::ScalarType distanceToStart = point.EuclideanDistanceTo(startCenter);
        mitk::ScalarType distanceToEnd = point.EuclideanDistanceTo(endCenter);

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
        mitk::ScalarType *p = fiberPolyData->GetPoint( pointsInCell[ startId ] );
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


        mitk::ScalarType d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );
        mitk::Vector3D newPoint = (p0-p1);

        point[0] = d*newPoint[0] + p0[0];
        point[1] = d*newPoint[1] + p0[1];
        point[2] = d*newPoint[2] + p0[2];

        singleTract.push_back(point);

        if(! (secondOnPositive ^ startOnPositive) )
        {
          /* StartId and startId+1 lie on the same side of the plane
             so startId is also part of the ROI*/

          mitk::ScalarType *start = fiberPolyData->GetPoint( pointsInCell[startId] );
          point[0] = start[0]; point[1] = start[1]; point[2] = start[2];
          singleTract.push_back(point);
        }



        for( int pointInCellID( startId+1 ); pointInCellID < endId ; pointInCellID++)
        {
          // push back point
          mitk::ScalarType *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );
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
        mitk::ScalarType *p = fiberPolyData->GetPoint( pointsInCell[ startId ] );
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


        mitk::ScalarType d = ( (onPlane-p0)*normal) / ( (p0-p1) * normal );
        mitk::Vector3D newPoint = (p0-p1);


        point[0] = d*newPoint[0] + p0[0];
        point[1] = d*newPoint[1] + p0[1];
        point[2] = d*newPoint[2] + p0[2];


        singleTract.push_back(point);

        if(! (secondOnPositive ^ startOnPositive) )
        {
          /* StartId and startId+1 lie on the same side of the plane
             so startId is also part of the ROI*/

          mitk::ScalarType *start = fiberPolyData->GetPoint( pointsInCell[startId] );
          point[0] = start[0]; point[1] = start[1]; point[2] = start[2];
          singleTract.push_back(point);
        }



        for( int pointInCellID( startId-1 ); pointInCellID > endId ; pointInCellID--)
        {
          // push back point
          mitk::ScalarType *p = fiberPolyData->GetPoint( pointsInCell[ pointInCellID ] );
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

    return tracts;
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
  if(m_Fib == NULL || m_CurrentTbssImage == NULL || m_CurrentStartRoi == NULL || m_CurrentEndRoi == NULL)
    return;

  if(m_PlottingFiberBundle)
  {
    DoPlotFiberBundles(m_Fib, m_CurrentImage, m_CurrentStartRoi, m_CurrentEndRoi, avg, number);
  }
  else
  {
    PlotFiber4D(m_CurrentTbssImage, m_Fib, m_CurrentStartRoi, m_CurrentEndRoi, number);
  }
}



TractContainerType QmitkTbssRoiAnalysisWidget::ParameterizeTracts(TractContainerType tracts, int number)
{
  TractContainerType resampledTracts;



  for(TractContainerType::iterator it = tracts.begin(); it != tracts.end(); ++it)
  {
    TractType resampledTract;
    TractType tract = *it;

    // Calculate the total length
    mitk::ScalarType totalLength = 0;

    if(tract.size() < 2)
      continue;

    PointType p0 = tract.at(0);
    for(unsigned int i = 1; i<tract.size(); i++)
    {
      PointType p1 = tract.at(i);
      mitk::ScalarType length = p0.EuclideanDistanceTo(p1);
      totalLength += length;
      p0 = p1;
    }

    mitk::ScalarType stepSize = totalLength / number;



    p0 = tract.at(0);
    PointType p1 = tract.at(1);
    unsigned int tractCounter = 2;
    mitk::ScalarType distance = p0.EuclideanDistanceTo(p1);
    mitk::ScalarType locationBetween = 0;

    for(mitk::ScalarType position = 0;
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


  mitk::ScalarType xSum = 0.0;
  mitk::ScalarType ySum = 0.0;
  mitk::ScalarType zSum = 0.0;
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

  mitk::ScalarType xPos = xSum / number;
  mitk::ScalarType yPos = ySum / number;
  mitk::ScalarType zPos = zSum / number;


  mitk::Point3D pos;
  pos[0] = xPos;
  pos[1] = yPos;
  pos[2] = zPos;

  return pos;
}


std::vector< std::vector<mitk::ScalarType> > QmitkTbssRoiAnalysisWidget::CalculateGroupProfiles()
{
  MITK_INFO << "make profiles!";
  std::vector< std::vector<mitk::ScalarType> > profiles;


  int size = m_Projections->GetVectorLength();
  for(int s=0; s<size; s++)
  {
    // Iterate trough the roi
    std::vector<mitk::ScalarType> profile;
    RoiType::iterator it;
    it = m_Roi.begin();
    while(it != m_Roi.end())
    {
      itk::Index<3> ix = *it;
      profile.push_back(m_Projections->GetPixel(ix).GetElement(s));
      it++;
    }

    profiles.push_back(profile);
  }



  m_IndividualProfiles = profiles;
  // Calculate the averages

  // Here a check could be build in to check whether all profiles have
  // the same length, but this should normally be the case if the input
  // data were corrected with the TBSS Module.

  std::vector< std::vector<mitk::ScalarType> > groupProfiles;

  std::vector< std::pair<std::string, int> >::iterator it;
  it = m_Groups.begin();

  int c = 0; //the current profile number

  while(it != m_Groups.end() && profiles.size() > 0)
  {
    std::pair<std::string, int> p = *it;
    int size = p.second;

    //initialize a vector of the right length with zeroes
    std::vector<mitk::ScalarType> averageProfile;
    for(unsigned int i=0; i<profiles.at(0).size(); i++)
    {
      averageProfile.push_back(0.0);
    }

    // Average the right number of profiles

    for(int i=0; i<size; i++)
    {
      for(unsigned int j=0; j<averageProfile.size(); ++j)
      {
        averageProfile.at(j) = averageProfile.at(j) + profiles.at(c).at(j);
      }
      c++;
    }

    // Divide by the number of profiles to get group average
    for(unsigned int i=0; i<averageProfile.size(); i++)
    {
      averageProfile.at(i) = averageProfile.at(i) / size;
    }

    groupProfiles.push_back(averageProfile);

    ++it;
  }

  return groupProfiles;
}


void QmitkTbssRoiAnalysisWidget::DrawProfiles()
{
  std::vector <std::vector<mitk::ScalarType> > groupProfiles = CalculateGroupProfiles();
  Plot(groupProfiles);
}

void QmitkTbssRoiAnalysisWidget::Plot(std::vector <std::vector<mitk::ScalarType> > groupProfiles)
{
    this->Clear();
    m_Vals.clear();
    std::vector<mitk::ScalarType> v1;


    std::vector<mitk::ScalarType> xAxis;
    for(unsigned int i=0; i<groupProfiles.at(0).size(); ++i)
    {
      xAxis.push_back((mitk::ScalarType)i);
    }


    // fill m_Vals. This might be used by the user to copy data to the clipboard
    for(unsigned int i=0; i<groupProfiles.size(); i++)
    {

      v1.clear();


      for(unsigned int j=0; j<groupProfiles.at(i).size(); j++)
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


std::vector< std::vector<mitk::ScalarType> > QmitkTbssRoiAnalysisWidget::CalculateGroupProfilesFibers(mitk::TbssImage::Pointer tbssImage,
                                                                                            mitk::FiberBundleX *fib,
                                                                                            mitk::PlanarFigure* startRoi,
                                                                                            mitk::PlanarFigure* endRoi,
                                                                                            int number)
{
    TractContainerType tracts = CreateTracts(fib, startRoi, endRoi);

    TractContainerType resampledTracts = ParameterizeTracts(tracts, number);

    int nTracts = resampledTracts.size();

    this->Clear();


    // For every group we have m fibers * n subjects of profiles to fill

    std::vector< std::vector<mitk::ScalarType> > profiles;

    // calculate individual profiles by going through all n subjects
    int size = m_Projections->GetVectorLength();
    for(int s=0; s<size; s++)
    {

      // Iterate through all tracts

      for(int t=0; t<nTracts; t++)
      {
        // Iterate trough the tract
        std::vector<mitk::ScalarType> profile;
        TractType::iterator it = resampledTracts[t].begin();
        while(it != resampledTracts[t].end())
        {
          PointType p = *it;
          PointType index;
          tbssImage->GetGeometry()->WorldToIndex(p, index);

          itk::Index<3> ix;
          ix[0] = index[0];
          ix[1] = index[1];
          ix[2] = index[2];
          // Get value from image

          profile.push_back(m_Projections->GetPixel(ix).GetElement(s));

          it++;
        }
        profiles.push_back(profile);
      }


    }

    m_IndividualProfiles = profiles;

    // Now create the group averages (every group contains m fibers * n_i group members

    std::vector< std::pair<std::string, int> >::iterator it;
    it = m_Groups.begin();
    int c = 0; //the current profile number

    // Calculate the group averages
    std::vector< std::vector<mitk::ScalarType> > groupProfiles;

    while(it != m_Groups.end() && profiles.size() > 0)
    {
      std::pair<std::string, int> p = *it;
      int size = p.second;

      //initialize a vector of the right length with zeroes
      std::vector<mitk::ScalarType> averageProfile;
      for(unsigned int i=0; i<profiles.at(0).size(); i++)
      {
        averageProfile.push_back(0.0);
      }

      // Average the right number of profiles

      for(int i=0; i<size*nTracts; i++)
      {
        for(unsigned int j=0; j<averageProfile.size(); ++j)
        {
          averageProfile.at(j) = averageProfile.at(j) + profiles.at(c).at(j);
        }
        c++;
      }

      // Divide by the number of profiles to get group average
      for(unsigned int i=0; i<averageProfile.size(); i++)
      {
        averageProfile.at(i) = averageProfile.at(i) / (size*nTracts);
      }

      groupProfiles.push_back(averageProfile);

      ++it;
    }

    return groupProfiles;
}


void QmitkTbssRoiAnalysisWidget::PlotFiber4D(mitk::TbssImage::Pointer tbssImage,
                                             mitk::FiberBundleX *fib,
                                             mitk::PlanarFigure* startRoi,
                                             mitk::PlanarFigure* endRoi,
                                             int number)
{
  m_PlottingFiberBundle = false;
  m_Fib = fib;
  m_CurrentStartRoi = startRoi;
  m_CurrentEndRoi = endRoi;
  m_CurrentTbssImage = tbssImage;
  std::vector <std::vector<mitk::ScalarType> > groupProfiles = CalculateGroupProfilesFibers(tbssImage, fib, startRoi, endRoi, number);
  Plot(groupProfiles);

}


template <typename T>
void QmitkTbssRoiAnalysisWidget::PlotFiberBundles(const mitk::PixelType ptype, TractContainerType tracts, mitk::Image *img, bool avg)
{

  m_PlottingFiberBundle = true;

  this->Clear();
  std::vector<TractType>::iterator it = tracts.begin();


  std::vector< std::vector <mitk::ScalarType > > profiles;

  mitk::ImagePixelReadAccessor<T,3> imAccess(img,img->GetVolumeData(0));

  it = tracts.begin();
  while(it != tracts.end())
  {

    TractType tract = *it;
    TractType::iterator tractIt = tract.begin();

    std::vector<mitk::ScalarType> profile;

    while(tractIt != tract.end())
    {
      PointType p = *tractIt;

      // Get value from image
      profile.push_back( (mitk::ScalarType) imAccess.GetPixelByWorldCoordinates(p) );

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
  std::vector<mitk::ScalarType> averageProfile;
  std::vector<mitk::ScalarType> profile = profiles.at(0); // can do this because we checked the size of profiles before
  for(unsigned int i=0; i<profile.size(); ++i)
  {
    averageProfile.push_back(0.0);
  }


  std::vector< std::vector<mitk::ScalarType> >::iterator profit = profiles.begin();

  int id=0;
  while(profit != profiles.end())
  {
    std::vector<mitk::ScalarType> profile = *profit;



    std::vector<mitk::ScalarType> xAxis;
    for(unsigned int i=0; i<profile.size(); ++i)
    {
      xAxis.push_back((mitk::ScalarType)i);
      averageProfile.at(i) += profile.at(i) / profiles.size();
    }

    int curveId = this->InsertCurve( "" );

    this->SetCurveData( curveId, xAxis, profile );

    ++profit;
    id++;


  }

  m_Average = averageProfile;


  if(avg)
  {

    // Draw the average profile
    std::vector<mitk::ScalarType> xAxis;
    for(unsigned int i=0; i<averageProfile.size(); ++i)
    {
      xAxis.push_back((mitk::ScalarType)i);
    }

    int curveId = this->InsertCurve( "" );
    this->SetCurveData( curveId, xAxis, averageProfile );




    QPen pen( Qt::SolidLine );
    pen.setWidth(3);
    pen.setColor(Qt::red);

    this->SetCurvePen( curveId, pen );



    id++;


  }

  this->Replot();





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




