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

#include "mitkFiberBundle.h"

#include <mitkPlanarCircle.h>
#include <mitkPlanarPolygon.h>
#include <mitkPlanarFigureComposite.h>
#include "mitkImagePixelReadAccessor.h"
#include <mitkPixelTypeMultiplex.h>

#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkPolyLine.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkIdFilter.h>
#include <vtkClipPolyData.h>
#include <vtkPlane.h>
#include <vtkDoubleArray.h>
#include <vtkKochanekSpline.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSpline.h>
#include <vtkPolygon.h>
#include <vtkCleanPolyData.h>
#include <boost/progress.hpp>
#include <vtkTransformPolyDataFilter.h>
#include <mitkTransferFunction.h>
#include <vtkLookupTable.h>
#include <mitkLookupTable.h>
#include <vtkCardinalSpline.h>
#include <vtkAppendPolyData.h>

const char* mitk::FiberBundle::FIBER_ID_ARRAY = "Fiber_IDs";

mitk::FiberBundle::FiberBundle( vtkPolyData* fiberPolyData )
  : m_NumFibers(0)
{
  m_FiberWeights = vtkSmartPointer<vtkFloatArray>::New();
  m_FiberWeights->SetName("FIBER_WEIGHTS");

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  if (fiberPolyData != nullptr)
    m_FiberPolyData = fiberPolyData;
  else
  {
    this->m_FiberPolyData->SetPoints(vtkSmartPointer<vtkPoints>::New());
    this->m_FiberPolyData->SetLines(vtkSmartPointer<vtkCellArray>::New());
  }

  this->UpdateFiberGeometry();
  this->GenerateFiberIds();
  this->ColorFibersByOrientation();
}

mitk::FiberBundle::~FiberBundle()
{

}

mitk::FiberBundle::Pointer mitk::FiberBundle::GetDeepCopy()
{
  mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(m_FiberPolyData);
  newFib->SetFiberColors(this->m_FiberColors);
  newFib->SetFiberWeights(this->m_FiberWeights);
  return newFib;
}

vtkSmartPointer<vtkPolyData> mitk::FiberBundle::GeneratePolyDataByIds(std::vector<long> fiberIds, vtkSmartPointer<vtkFloatArray> weights)
{
  vtkSmartPointer<vtkPolyData> newFiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> newLineSet = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> newPointSet = vtkSmartPointer<vtkPoints>::New();
  weights->SetNumberOfValues(fiberIds.size());

  int counter = 0;
  auto finIt = fiberIds.begin();
  while ( finIt != fiberIds.end() )
  {
    if (*finIt < 0 || *finIt>GetNumFibers()){
      MITK_INFO << "FiberID can not be negative or >NumFibers!!! check id Extraction!" << *finIt;
      break;
    }

    vtkSmartPointer<vtkCell> fiber = m_FiberIdDataSet->GetCell(*finIt);//->DeepCopy(fiber);
    vtkSmartPointer<vtkPoints> fibPoints = fiber->GetPoints();
    vtkSmartPointer<vtkPolyLine> newFiber = vtkSmartPointer<vtkPolyLine>::New();
    newFiber->GetPointIds()->SetNumberOfIds( fibPoints->GetNumberOfPoints() );

    for(int i=0; i<fibPoints->GetNumberOfPoints(); i++)
    {
      newFiber->GetPointIds()->SetId(i, newPointSet->GetNumberOfPoints());
      newPointSet->InsertNextPoint(fibPoints->GetPoint(i)[0], fibPoints->GetPoint(i)[1], fibPoints->GetPoint(i)[2]);
    }

    weights->InsertValue(counter, this->GetFiberWeight(*finIt));
    newLineSet->InsertNextCell(newFiber);
    ++finIt;
    ++counter;
  }

  newFiberPolyData->SetPoints(newPointSet);
  newFiberPolyData->SetLines(newLineSet);
  return newFiberPolyData;
}

// merge two fiber bundles
mitk::FiberBundle::Pointer mitk::FiberBundle::AddBundles(std::vector< mitk::FiberBundle::Pointer > fibs)
{
  vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

  // add current fiber bundle
  vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();

  int num_weights = this->GetNumFibers();
  for (auto fib : fibs)
    num_weights += fib->GetNumFibers();
  weights->SetNumberOfValues(num_weights);

  unsigned int counter = 0;
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double p[3];
      points->GetPoint(j, p);

      vtkIdType id = vNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    weights->InsertValue(counter, this->GetFiberWeight(i));
    vNewLines->InsertNextCell(container);
    counter++;
  }

  for (auto fib : fibs)
  {
    // add new fiber bundle
    for (int i=0; i<fib->GetFiberPolyData()->GetNumberOfCells(); i++)
    {
      vtkCell* cell = fib->GetFiberPolyData()->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
      for (int j=0; j<numPoints; j++)
      {
        double p[3];
        points->GetPoint(j, p);

        vtkIdType id = vNewPoints->InsertNextPoint(p);
        container->GetPointIds()->InsertNextId(id);
      }
      weights->InsertValue(counter, fib->GetFiberWeight(i));
      vNewLines->InsertNextCell(container);
      counter++;
    }
  }

  // initialize PolyData
  vNewPolyData->SetPoints(vNewPoints);
  vNewPolyData->SetLines(vNewLines);

  // initialize fiber bundle
  mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(vNewPolyData);
  newFib->SetFiberWeights(weights);
  return newFib;
}

// merge two fiber bundles
mitk::FiberBundle::Pointer mitk::FiberBundle::AddBundle(mitk::FiberBundle* fib)
{
  if (fib==nullptr)
    return this->GetDeepCopy();

  MITK_INFO << "Adding fibers";

  vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

  // add current fiber bundle
  vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
  weights->SetNumberOfValues(this->GetNumFibers()+fib->GetNumFibers());

  unsigned int counter = 0;
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double p[3];
      points->GetPoint(j, p);

      vtkIdType id = vNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    weights->InsertValue(counter, this->GetFiberWeight(i));
    vNewLines->InsertNextCell(container);
    counter++;
  }

  // add new fiber bundle
  for (int i=0; i<fib->GetFiberPolyData()->GetNumberOfCells(); i++)
  {
    vtkCell* cell = fib->GetFiberPolyData()->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double p[3];
      points->GetPoint(j, p);

      vtkIdType id = vNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    weights->InsertValue(counter, fib->GetFiberWeight(i));
    vNewLines->InsertNextCell(container);
    counter++;
  }

  // initialize PolyData
  vNewPolyData->SetPoints(vNewPoints);
  vNewPolyData->SetLines(vNewLines);

  // initialize fiber bundle
  mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(vNewPolyData);
  newFib->SetFiberWeights(weights);
  return newFib;
}

// Only retain fibers with a weight larger than the specified threshold
mitk::FiberBundle::Pointer mitk::FiberBundle::FilterByWeights(float weight_thr, bool invert)
{
  vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

  std::vector<float> weights;

  for (int i=0; i<this->GetNumFibers(); i++)
  {
    if ( (invert && this->GetFiberWeight(i)>weight_thr) || (!invert && this->GetFiberWeight(i)<=weight_thr))
      continue;

    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double p[3];
      points->GetPoint(j, p);

      vtkIdType id = vNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    vNewLines->InsertNextCell(container);
    weights.push_back(this->GetFiberWeight(i));
  }

  // initialize PolyData
  vNewPolyData->SetPoints(vNewPoints);
  vNewPolyData->SetLines(vNewLines);

  // initialize fiber bundle
  mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(vNewPolyData);
  for (unsigned int i=0; i<weights.size(); ++i)
    newFib->SetFiberWeight(i, weights.at(i));
  return newFib;
}

// Only retain a subsample of the fibers
mitk::FiberBundle::Pointer mitk::FiberBundle::SubsampleFibers(float factor)
{
  vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

  int new_num_fibs = this->GetNumFibers()*factor;
  MITK_INFO << "Subsampling fibers with factor " << factor << "(" << new_num_fibs << "/" << this->GetNumFibers() << ")";

  // add current fiber bundle
  vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
  weights->SetNumberOfValues(new_num_fibs);

  std::vector< int > ids;
  for (int i=0; i<this->GetNumFibers(); i++)
    ids.push_back(i);
  std::random_shuffle(ids.begin(), ids.end());

  unsigned int counter = 0;
  for (int i=0; i<new_num_fibs; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(ids.at(i));
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double p[3];
      points->GetPoint(j, p);

      vtkIdType id = vNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    weights->InsertValue(counter, this->GetFiberWeight(ids.at(i)));
    vNewLines->InsertNextCell(container);
    counter++;
  }

  // initialize PolyData
  vNewPolyData->SetPoints(vNewPoints);
  vNewPolyData->SetLines(vNewLines);

  // initialize fiber bundle
  mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(vNewPolyData);
  newFib->SetFiberWeights(weights);
  return newFib;
}

// subtract two fiber bundles
mitk::FiberBundle::Pointer mitk::FiberBundle::SubtractBundle(mitk::FiberBundle* fib)
{
  if (fib==nullptr)
    return this->GetDeepCopy();

  MITK_INFO << "Subtracting fibers";
  vtkSmartPointer<vtkPolyData> vNewPolyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkCellArray> vNewLines = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPoints> vNewPoints = vtkSmartPointer<vtkPoints>::New();

  std::vector< std::vector< itk::Point<float, 3> > > points1;
  for( int i=0; i<m_NumFibers; i++ )
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (points==nullptr || numPoints<=0)
      continue;

    itk::Point<float, 3> start = GetItkPoint(points->GetPoint(0));
    itk::Point<float, 3> end = GetItkPoint(points->GetPoint(numPoints-1));

    points1.push_back( {start, end} );
  }

  std::vector< std::vector< itk::Point<float, 3> > > points2;
  for( int i=0; i<fib->GetNumFibers(); i++ )
  {
    vtkCell* cell = fib->GetFiberPolyData()->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (points==nullptr || numPoints<=0)
      continue;

    itk::Point<float, 3> start = GetItkPoint(points->GetPoint(0));
    itk::Point<float, 3> end = GetItkPoint(points->GetPoint(numPoints-1));

    points2.push_back( {start, end} );
  }

  //  int progress = 0;
  std::vector< int > ids;
#pragma omp parallel for
  for (int i=0; i<(int)points1.size(); i++)
  {
    //#pragma omp critical
    //    {
    //      progress++;
    //      std::cout << (int)(100*(float)progress/points1.size()) << "%" << '\r';
    //      cout.flush();
    //    }

    bool match = false;
    for (unsigned int j=0; j<points2.size(); j++)
    {
      auto v1 = points1.at(i);
      auto v2 = points2.at(j);

      float dist=0;
      for (unsigned int c=0; c<v1.size(); c++)
      {
        float d = v1[c][0]-v2[c][0];
        dist += d*d;

        d = v1[c][1]-v2[c][1];
        dist += d*d;

        d = v1[c][2]-v2[c][2];
        dist += d*d;
      }
      dist /= v1.size();

      if (dist<mitk::eps)
      {
        match = true;
        break;
      }

      dist=0;
      for (unsigned int c=0; c<v1.size(); c++)
      {
        float d = v1[v1.size()-1-c][0]-v2[c][0];
        dist += d*d;

        d = v1[v1.size()-1-c][1]-v2[c][1];
        dist += d*d;

        d = v1[v1.size()-1-c][2]-v2[c][2];
        dist += d*d;
      }
      dist /= v1.size();

      if (dist<mitk::eps)
      {
        match = true;
        break;
      }
    }

#pragma omp critical
    if (!match)
      ids.push_back(i);
  }

  for( int i : ids )
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (points==nullptr || numPoints<=0)
      continue;

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for( int j=0; j<numPoints; j++)
    {
      vtkIdType id = vNewPoints->InsertNextPoint(points->GetPoint(j));
      container->GetPointIds()->InsertNextId(id);
    }
    vNewLines->InsertNextCell(container);
  }
  if(vNewLines->GetNumberOfCells()==0)
    return mitk::FiberBundle::New();
  // initialize PolyData
  vNewPolyData->SetPoints(vNewPoints);
  vNewPolyData->SetLines(vNewLines);

  // initialize fiber bundle
  return mitk::FiberBundle::New(vNewPolyData);
}

itk::Point<float, 3> mitk::FiberBundle::GetItkPoint(double point[3])
{
  itk::Point<float, 3> itkPoint;
  itkPoint[0] = point[0];
  itkPoint[1] = point[1];
  itkPoint[2] = point[2];
  return itkPoint;
}

/*
 * set PolyData (additional flag to recompute fiber geometry, default = true)
 */
void mitk::FiberBundle::SetFiberPolyData(vtkSmartPointer<vtkPolyData> fiberPD, bool updateGeometry)
{
  if (fiberPD == nullptr)
    this->m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  else
    m_FiberPolyData->DeepCopy(fiberPD);

  m_NumFibers = m_FiberPolyData->GetNumberOfLines();

  if (updateGeometry)
    UpdateFiberGeometry();
  GenerateFiberIds();
  ColorFibersByOrientation();
}

/*
 * return vtkPolyData
 */
vtkSmartPointer<vtkPolyData> mitk::FiberBundle::GetFiberPolyData() const
{
  return m_FiberPolyData;
}

void mitk::FiberBundle::ColorFibersByLength(bool opacity, bool normalize)
{
  if (m_MaxFiberLength<=0)
    return;

  int numOfPoints = this->GetNumberOfPoints();

  //colors and alpha value for each single point, RGBA = 4 components
  unsigned char rgba[4] = {0,0,0,0};
  int componentSize = 4;
  m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_FiberColors->Allocate(numOfPoints * componentSize);
  m_FiberColors->SetNumberOfComponents(componentSize);
  m_FiberColors->SetName("FIBER_COLORS");

  int numOfFibers = m_FiberPolyData->GetNumberOfLines();
  if (numOfFibers < 1)
    return;

  mitk::LookupTable::Pointer mitkLookup = mitk::LookupTable::New();
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetTableRange(0.0, 0.8);
  lookupTable->Build();
  mitkLookup->SetVtkLookupTable(lookupTable);
  mitkLookup->SetType(mitk::LookupTable::JET);

  unsigned int count = 0;
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();

    float l = m_FiberLengths.at(i)/m_MaxFiberLength;
    if (!normalize)
    {
      l = m_FiberLengths.at(i)/255.0;
      if (l > 1.0)
        l = 1.0;
    }
    for (int j=0; j<numPoints; j++)
    {
      double color[3];
      lookupTable->GetColor(1.0 - l, color);

      rgba[0] = (unsigned char) (255.0 * color[0]);
      rgba[1] = (unsigned char) (255.0 * color[1]);
      rgba[2] = (unsigned char) (255.0 * color[2]);
      if (opacity)
        rgba[3] = (unsigned char) (255.0 * l);
      else
        rgba[3] = (unsigned char) (255.0);
      m_FiberColors->InsertTypedTuple(cell->GetPointId(j), rgba);
      count++;
    }
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ColorFibersByOrientation()
{
  //===== FOR WRITING A TEST ========================
  //  colorT size == tupelComponents * tupelElements
  //  compare color results
  //  to cover this code 100% also PolyData needed, where colorarray already exists
  //  + one fiber with exactly 1 point
  //  + one fiber with 0 points
  //=================================================

  vtkPoints* extrPoints = nullptr;
  extrPoints = m_FiberPolyData->GetPoints();
  int numOfPoints = 0;
  if (extrPoints!=nullptr)
    numOfPoints = extrPoints->GetNumberOfPoints();

  //colors and alpha value for each single point, RGBA = 4 components
  unsigned char rgba[4] = {0,0,0,0};
  int componentSize = 4;
  m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_FiberColors->Allocate(numOfPoints * componentSize);
  m_FiberColors->SetNumberOfComponents(componentSize);
  m_FiberColors->SetName("FIBER_COLORS");

  int numOfFibers = m_FiberPolyData->GetNumberOfLines();
  if (numOfFibers < 1)
    return;

  /* extract single fibers of fiberBundle */
  vtkCellArray* fiberList = m_FiberPolyData->GetLines();
  fiberList->InitTraversal();
  for (int fi=0; fi<numOfFibers; ++fi) {

    vtkIdType* idList; // contains the point id's of the line
    vtkIdType pointsPerFiber; // number of points for current line
    fiberList->GetNextCell(pointsPerFiber, idList);

    /* single fiber checkpoints: is number of points valid */
    if (pointsPerFiber > 1)
    {
      /* operate on points of single fiber */
      for (int i=0; i <pointsPerFiber; ++i)
      {
        /* process all points elastV[0]ept starting and endpoint for calculating color value take current point, previous point and next point */
        if (i<pointsPerFiber-1 && i > 0)
        {
          /* The color value of the current point is influenced by the previous point and next point. */
          vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
          vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[i+1])[0], extrPoints->GetPoint(idList[i+1])[1], extrPoints->GetPoint(idList[i+1])[2]);
          vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(idList[i-1])[0], extrPoints->GetPoint(idList[i-1])[1], extrPoints->GetPoint(idList[i-1])[2]);

          vnl_vector_fixed< double, 3 > diff1;
          diff1 = currentPntvtk - nextPntvtk;

          vnl_vector_fixed< double, 3 > diff2;
          diff2 = currentPntvtk - prevPntvtk;

          vnl_vector_fixed< double, 3 > diff;
          diff = (diff1 - diff2) / 2.0;
          diff.normalize();

          rgba[0] = (unsigned char) (255.0 * std::fabs(diff[0]));
          rgba[1] = (unsigned char) (255.0 * std::fabs(diff[1]));
          rgba[2] = (unsigned char) (255.0 * std::fabs(diff[2]));
          rgba[3] = (unsigned char) (255.0);
        }
        else if (i==0)
        {
          /* First point has no previous point, therefore only diff1 is taken */

          vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
          vnl_vector_fixed< double, 3 > nextPntvtk(extrPoints->GetPoint(idList[i+1])[0], extrPoints->GetPoint(idList[i+1])[1], extrPoints->GetPoint(idList[i+1])[2]);

          vnl_vector_fixed< double, 3 > diff1;
          diff1 = currentPntvtk - nextPntvtk;
          diff1.normalize();

          rgba[0] = (unsigned char) (255.0 * std::fabs(diff1[0]));
          rgba[1] = (unsigned char) (255.0 * std::fabs(diff1[1]));
          rgba[2] = (unsigned char) (255.0 * std::fabs(diff1[2]));
          rgba[3] = (unsigned char) (255.0);
        }
        else if (i==pointsPerFiber-1)
        {
          /* Last point has no next point, therefore only diff2 is taken */
          vnl_vector_fixed< double, 3 > currentPntvtk(extrPoints->GetPoint(idList[i])[0], extrPoints->GetPoint(idList[i])[1],extrPoints->GetPoint(idList[i])[2]);
          vnl_vector_fixed< double, 3 > prevPntvtk(extrPoints->GetPoint(idList[i-1])[0], extrPoints->GetPoint(idList[i-1])[1], extrPoints->GetPoint(idList[i-1])[2]);

          vnl_vector_fixed< double, 3 > diff2;
          diff2 = currentPntvtk - prevPntvtk;
          diff2.normalize();

          rgba[0] = (unsigned char) (255.0 * std::fabs(diff2[0]));
          rgba[1] = (unsigned char) (255.0 * std::fabs(diff2[1]));
          rgba[2] = (unsigned char) (255.0 * std::fabs(diff2[2]));
          rgba[3] = (unsigned char) (255.0);
        }
        m_FiberColors->InsertTypedTuple(idList[i], rgba);
      }
    }
    else if (pointsPerFiber == 1)
    {
      /* a single point does not define a fiber (use vertex mechanisms instead */
      continue;
    }
    else
    {
      MITK_DEBUG << "Fiber with 0 points detected... please check your tractography algorithm!" ;
      continue;
    }
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ColorFibersByCurvature(bool, bool normalize)
{
  double window = 5;

  //colors and alpha value for each single point, RGBA = 4 components
  unsigned char rgba[4] = {0,0,0,0};
  int componentSize = 4;
  m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * componentSize);
  m_FiberColors->SetNumberOfComponents(componentSize);
  m_FiberColors->SetName("FIBER_COLORS");

  mitk::LookupTable::Pointer mitkLookup = mitk::LookupTable::New();
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetTableRange(0.0, 0.8);
  lookupTable->Build();
  mitkLookup->SetVtkLookupTable(lookupTable);
  mitkLookup->SetType(mitk::LookupTable::JET);

  std::vector< double > values;
  double min = 1;
  double max = 0;
  MITK_INFO << "Coloring fibers by curvature";
  boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    ++disp;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    // calculate curvatures
    for (int j=0; j<numPoints; j++)
    {
      double dist = 0;
      int c = j;
      std::vector< vnl_vector_fixed< float, 3 > > vectors;
      vnl_vector_fixed< float, 3 > meanV; meanV.fill(0.0);
      while(dist<window/2 && c>1)
      {
        double p1[3];
        points->GetPoint(c-1, p1);
        double p2[3];
        points->GetPoint(c, p2);

        vnl_vector_fixed< float, 3 > v;
        v[0] = p2[0]-p1[0];
        v[1] = p2[1]-p1[1];
        v[2] = p2[2]-p1[2];
        dist += v.magnitude();
        v.normalize();
        vectors.push_back(v);
        meanV += v;
        c--;
      }
      c = j;
      dist = 0;
      while(dist<window/2 && c<numPoints-1)
      {
        double p1[3];
        points->GetPoint(c, p1);
        double p2[3];
        points->GetPoint(c+1, p2);

        vnl_vector_fixed< float, 3 > v;
        v[0] = p2[0]-p1[0];
        v[1] = p2[1]-p1[1];
        v[2] = p2[2]-p1[2];
        dist += v.magnitude();
        v.normalize();
        vectors.push_back(v);
        meanV += v;
        c++;
      }
      meanV.normalize();

      double dev = 0;
      for (unsigned int c=0; c<vectors.size(); c++)
      {
        double angle = dot_product(meanV, vectors.at(c));
        if (angle>1.0)
          angle = 1.0;
        if (angle<-1.0)
          angle = -1.0;
        dev += acos(angle)*180/itk::Math::pi;
      }
      if (vectors.size()>0)
        dev /= vectors.size();

      dev = 1.0-dev/180.0;
      values.push_back(dev);
      if (dev<min)
        min = dev;
      if (dev>max)
        max = dev;
    }
  }
  unsigned int count = 0;
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    for (int j=0; j<numPoints; j++)
    {
      double color[3];
      double dev = values.at(count);
      if (normalize)
        dev = (dev-min)/(max-min);
      else if (dev>1)
        dev = 1;
      lookupTable->GetColor(dev, color);

      rgba[0] = (unsigned char) (255.0 * color[0]);
      rgba[1] = (unsigned char) (255.0 * color[1]);
      rgba[2] = (unsigned char) (255.0 * color[2]);
      rgba[3] = (unsigned char) (255.0);
      m_FiberColors->InsertTypedTuple(cell->GetPointId(j), rgba);
      count++;
    }
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::SetFiberOpacity(vtkDoubleArray* FAValArray)
{
  for(long i=0; i<m_FiberColors->GetNumberOfTuples(); i++)
  {
    double faValue = FAValArray->GetValue(i);
    faValue = faValue * 255.0;
    m_FiberColors->SetComponent(i,3, (unsigned char) faValue );
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ResetFiberOpacity()
{
  for(long i=0; i<m_FiberColors->GetNumberOfTuples(); i++)
    m_FiberColors->SetComponent(i,3, 255.0 );
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::ColorFibersByScalarMap(mitk::Image::Pointer FAimage, bool opacity, bool normalize)
{
  mitkPixelTypeMultiplex3( ColorFibersByScalarMap, FAimage->GetPixelType(), FAimage, opacity, normalize );
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

template <typename TPixel>
void mitk::FiberBundle::ColorFibersByScalarMap(const mitk::PixelType, mitk::Image::Pointer image, bool opacity, bool normalize)
{
  m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * 4);
  m_FiberColors->SetNumberOfComponents(4);
  m_FiberColors->SetName("FIBER_COLORS");

  mitk::ImagePixelReadAccessor<TPixel,3> readimage(image, image->GetVolumeData(0));

  unsigned char rgba[4] = {0,0,0,0};
  vtkPoints* pointSet = m_FiberPolyData->GetPoints();

  mitk::LookupTable::Pointer mitkLookup = mitk::LookupTable::New();
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetTableRange(0.0, 0.8);
  lookupTable->Build();
  mitkLookup->SetVtkLookupTable(lookupTable);
  mitkLookup->SetType(mitk::LookupTable::JET);

  double min = 9999999;
  double max = -9999999;
  for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
  {
    Point3D px;
    px[0] = pointSet->GetPoint(i)[0];
    px[1] = pointSet->GetPoint(i)[1];
    px[2] = pointSet->GetPoint(i)[2];
    double pixelValue = readimage.GetPixelByWorldCoordinates(px);
    if (pixelValue>max)
      max = pixelValue;
    if (pixelValue<min)
      min = pixelValue;
  }

  for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
  {
    Point3D px;
    px[0] = pointSet->GetPoint(i)[0];
    px[1] = pointSet->GetPoint(i)[1];
    px[2] = pointSet->GetPoint(i)[2];
    double pixelValue = readimage.GetPixelByWorldCoordinates(px);

    if (normalize)
      pixelValue = (pixelValue-min)/(max-min);
    else if (pixelValue>1)
      pixelValue = 1;

    double color[3];
    lookupTable->GetColor(1-pixelValue, color);

    rgba[0] = (unsigned char) (255.0 * color[0]);
    rgba[1] = (unsigned char) (255.0 * color[1]);
    rgba[2] = (unsigned char) (255.0 * color[2]);
    if (opacity)
      rgba[3] = (unsigned char) (255.0 * pixelValue);
    else
      rgba[3] = (unsigned char) (255.0);
    m_FiberColors->InsertTypedTuple(i, rgba);
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}


void mitk::FiberBundle::ColorFibersByFiberWeights(bool opacity, bool normalize)
{
  m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * 4);
  m_FiberColors->SetNumberOfComponents(4);
  m_FiberColors->SetName("FIBER_COLORS");

  mitk::LookupTable::Pointer mitkLookup = mitk::LookupTable::New();
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  lookupTable->SetTableRange(0.0, 0.8);
  lookupTable->Build();
  mitkLookup->SetVtkLookupTable(lookupTable);
  mitkLookup->SetType(mitk::LookupTable::JET);

  unsigned char rgba[4] = {0,0,0,0};
  unsigned int counter = 0;

  float max = -999999;
  float min = 999999;
  for (int i=0; i<m_NumFibers; i++)
  {
    double weight = this->GetFiberWeight(i);
    if (weight>max)
      max = weight;
    if (weight<min)
      min = weight;
  }
  if (fabs(max-min)<0.00001)
  {
    max = 1;
    min = 0;
  }

  for (int i=0; i<m_NumFibers; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    double weight = this->GetFiberWeight(i);

    for (int j=0; j<numPoints; j++)
    {
      float v = weight;
      if (normalize)
        v = (v-min)/(max-min);
      else if (v>1)
        v = 1;
      double color[3];
      lookupTable->GetColor(1-v, color);

      rgba[0] = (unsigned char) (255.0 * color[0]);
      rgba[1] = (unsigned char) (255.0 * color[1]);
      rgba[2] = (unsigned char) (255.0 * color[2]);
      if (opacity)
        rgba[3] = (unsigned char) (255.0 * v);
      else
        rgba[3] = (unsigned char) (255.0);

      m_FiberColors->InsertTypedTuple(counter, rgba);
      counter++;
    }
  }

  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::SetFiberColors(float r, float g, float b, float alpha)
{
  m_FiberColors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  m_FiberColors->Allocate(m_FiberPolyData->GetNumberOfPoints() * 4);
  m_FiberColors->SetNumberOfComponents(4);
  m_FiberColors->SetName("FIBER_COLORS");

  unsigned char rgba[4] = {0,0,0,0};
  for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
  {
    rgba[0] = (unsigned char) r;
    rgba[1] = (unsigned char) g;
    rgba[2] = (unsigned char) b;
    rgba[3] = (unsigned char) alpha;
    m_FiberColors->InsertTypedTuple(i, rgba);
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

void mitk::FiberBundle::GenerateFiberIds()
{
  if (m_FiberPolyData == nullptr)
    return;

  vtkSmartPointer<vtkIdFilter> idFiberFilter = vtkSmartPointer<vtkIdFilter>::New();
  idFiberFilter->SetInputData(m_FiberPolyData);
  idFiberFilter->CellIdsOn();
  //  idFiberFilter->PointIdsOn(); // point id's are not needed
  idFiberFilter->SetIdsArrayName(FIBER_ID_ARRAY);
  idFiberFilter->FieldDataOn();
  idFiberFilter->Update();

  m_FiberIdDataSet = idFiberFilter->GetOutput();

}

float mitk::FiberBundle::GetOverlap(ItkUcharImgType* mask, bool do_resampling)
{
  vtkSmartPointer<vtkPolyData> PolyData = m_FiberPolyData;
  mitk::FiberBundle::Pointer fibCopy = this;
  if (do_resampling)
  {
    float minSpacing = 1;
    if(mask->GetSpacing()[0]<mask->GetSpacing()[1] && mask->GetSpacing()[0]<mask->GetSpacing()[2])
      minSpacing = mask->GetSpacing()[0];
    else if (mask->GetSpacing()[1] < mask->GetSpacing()[2])
      minSpacing = mask->GetSpacing()[1];
    else
      minSpacing = mask->GetSpacing()[2];

    fibCopy = this->GetDeepCopy();
    fibCopy->ResampleLinear(minSpacing/5);
    PolyData = fibCopy->GetFiberPolyData();
  }

  MITK_INFO << "Calculating overlap";
  int inside = 0;
  int outside = 0;
  boost::progress_display disp(m_NumFibers);
  for (int i=0; i<m_NumFibers; i++)
  {
    ++disp;
    vtkCell* cell = PolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    for (int j=0; j<numPoints; j++)
    {
      double* p = points->GetPoint(j);
      itk::Point<float, 3> itkP;
      itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
      itk::Index<3> idx;
      mask->TransformPhysicalPointToIndex(itkP, idx);

      if ( mask->GetLargestPossibleRegion().IsInside(idx) && mask->GetPixel(idx) != 0 )
        inside++;
      else
        outside++;
    }
  }

  if (inside+outside==0)
    outside = 1;
  return (float)inside/(inside+outside);
}

mitk::FiberBundle::Pointer mitk::FiberBundle::RemoveFibersOutside(ItkUcharImgType* mask, bool invert)
{
  float minSpacing = 1;
  if(mask->GetSpacing()[0]<mask->GetSpacing()[1] && mask->GetSpacing()[0]<mask->GetSpacing()[2])
    minSpacing = mask->GetSpacing()[0];
  else if (mask->GetSpacing()[1] < mask->GetSpacing()[2])
    minSpacing = mask->GetSpacing()[1];
  else
    minSpacing = mask->GetSpacing()[2];

  mitk::FiberBundle::Pointer fibCopy = this->GetDeepCopy();
  fibCopy->ResampleLinear(minSpacing/10);
  vtkSmartPointer<vtkPolyData> PolyData =fibCopy->GetFiberPolyData();

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  vtkSmartPointer<vtkFloatArray> newFiberWeights = vtkSmartPointer<vtkFloatArray>::New();
  newFiberWeights->SetName("FIBER_WEIGHTS");
  newFiberWeights->SetNumberOfValues(m_NumFibers);

  MITK_INFO << "Cutting fibers";
  boost::progress_display disp(m_NumFibers);
  for (int i=0; i<m_NumFibers; i++)
  {
    ++disp;

    vtkCell* cell = PolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    if (numPoints>1)
    {
      int newNumPoints = 0;
      for (int j=0; j<numPoints; j++)
      {
        double* p = points->GetPoint(j);

        itk::Point<float, 3> itkP;
        itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
        itk::Index<3> idx;
        mask->TransformPhysicalPointToIndex(itkP, idx);

        if ( mask->GetPixel(idx) != 0 && mask->GetLargestPossibleRegion().IsInside(idx) && !invert )
        {
          vtkIdType id = vtkNewPoints->InsertNextPoint(p);
          container->GetPointIds()->InsertNextId(id);
          newNumPoints++;
        }
        else if ( (!mask->GetLargestPossibleRegion().IsInside(idx) || mask->GetPixel(idx) == 0) && invert )
        {
          vtkIdType id = vtkNewPoints->InsertNextPoint(p);
          container->GetPointIds()->InsertNextId(id);
          newNumPoints++;
        }
        else if (newNumPoints>1)
        {
          vtkNewCells->InsertNextCell(container);
          if (newFiberWeights->GetSize()<=vtkNewCells->GetNumberOfCells())
            newFiberWeights->Resize(vtkNewCells->GetNumberOfCells()*2);
          newFiberWeights->SetValue(vtkNewCells->GetNumberOfCells(), fibCopy->GetFiberWeight(i));

          newNumPoints = 0;
          container = vtkSmartPointer<vtkPolyLine>::New();
        }
      }

      if (newNumPoints>1)
      {
        vtkNewCells->InsertNextCell(container);
        if (newFiberWeights->GetSize()<=vtkNewCells->GetNumberOfCells())
          newFiberWeights->Resize(vtkNewCells->GetNumberOfCells()*2);

        newFiberWeights->SetValue(vtkNewCells->GetNumberOfCells(), fibCopy->GetFiberWeight(i));
      }
    }

  }

  if (vtkNewCells->GetNumberOfCells()<=0)
    return nullptr;
  newFiberWeights->Resize(vtkNewCells->GetNumberOfCells());

  vtkSmartPointer<vtkPolyData> newPolyData = vtkSmartPointer<vtkPolyData>::New();
  newPolyData->SetPoints(vtkNewPoints);
  newPolyData->SetLines(vtkNewCells);
  mitk::FiberBundle::Pointer newFib = mitk::FiberBundle::New(newPolyData);
  newFib->SetFiberWeights(newFiberWeights);
  newFib->Compress(0.1);
  return newFib;
}

mitk::FiberBundle::Pointer mitk::FiberBundle::ExtractFiberSubset(DataNode* roi, DataStorage* storage)
{
  if (roi==nullptr || !(dynamic_cast<PlanarFigure*>(roi->GetData()) || dynamic_cast<PlanarFigureComposite*>(roi->GetData())) )
    return nullptr;

  std::vector<long> tmp = ExtractFiberIdSubset(roi, storage);

  if (tmp.size()<=0)
    return mitk::FiberBundle::New();
  vtkSmartPointer<vtkFloatArray> weights = vtkSmartPointer<vtkFloatArray>::New();
  vtkSmartPointer<vtkPolyData> pTmp = GeneratePolyDataByIds(tmp, weights);
  mitk::FiberBundle::Pointer fib = mitk::FiberBundle::New(pTmp);
  fib->SetFiberWeights(weights);
  return fib;
}

std::vector<long> mitk::FiberBundle::ExtractFiberIdSubset(DataNode *roi, DataStorage* storage)
{
  std::vector<long> result;
  if (roi==nullptr || roi->GetData()==nullptr)
    return result;

  mitk::PlanarFigureComposite::Pointer pfc = dynamic_cast<mitk::PlanarFigureComposite*>(roi->GetData());
  if (!pfc.IsNull()) // handle composite
  {
    DataStorage::SetOfObjects::ConstPointer children = storage->GetDerivations(roi);
    if (children->size()==0)
      return result;

    switch (pfc->getOperationType())
    {
    case 0: // AND
    {
      MITK_INFO << "AND";
      result = this->ExtractFiberIdSubset(children->ElementAt(0), storage);
      std::vector<long>::iterator it;
      for (unsigned int i=1; i<children->Size(); ++i)
      {
        std::vector<long> inRoi = this->ExtractFiberIdSubset(children->ElementAt(i), storage);

        std::vector<long> rest(std::min(result.size(),inRoi.size()));
        it = std::set_intersection(result.begin(), result.end(), inRoi.begin(), inRoi.end(), rest.begin() );
        rest.resize( it - rest.begin() );
        result = rest;
      }
      break;
    }
    case 1: // OR
    {
      MITK_INFO << "OR";
      result = ExtractFiberIdSubset(children->ElementAt(0), storage);
      std::vector<long>::iterator it;
      for (unsigned int i=1; i<children->Size(); ++i)
      {
        it = result.end();
        std::vector<long> inRoi = ExtractFiberIdSubset(children->ElementAt(i), storage);
        result.insert(it, inRoi.begin(), inRoi.end());
      }

      // remove duplicates
      sort(result.begin(), result.end());
      it = unique(result.begin(), result.end());
      result.resize( it - result.begin() );
      break;
    }
    case 2: // NOT
    {
      MITK_INFO << "NOT";
      for(long i=0; i<this->GetNumFibers(); i++)
        result.push_back(i);

      std::vector<long>::iterator it;
      for (unsigned int i=0; i<children->Size(); ++i)
      {
        std::vector<long> inRoi = ExtractFiberIdSubset(children->ElementAt(i), storage);

        std::vector<long> rest(result.size()-inRoi.size());
        it = std::set_difference(result.begin(), result.end(), inRoi.begin(), inRoi.end(), rest.begin() );
        rest.resize( it - rest.begin() );
        result = rest;
      }
      break;
    }
    }
  }
  else if ( dynamic_cast<mitk::PlanarFigure*>(roi->GetData()) )  // actual extraction
  {
    if ( dynamic_cast<mitk::PlanarPolygon*>(roi->GetData()) )
    {
      mitk::PlanarFigure::Pointer planarPoly = dynamic_cast<mitk::PlanarFigure*>(roi->GetData());

      //create vtkPolygon using controlpoints from planarFigure polygon
      vtkSmartPointer<vtkPolygon> polygonVtk = vtkSmartPointer<vtkPolygon>::New();
      for (unsigned int i=0; i<planarPoly->GetNumberOfControlPoints(); ++i)
      {
        itk::Point<double,3> p = planarPoly->GetWorldControlPoint(i);
        vtkIdType id = polygonVtk->GetPoints()->InsertNextPoint(p[0], p[1], p[2] );
        polygonVtk->GetPointIds()->InsertNextId(id);
      }

      MITK_INFO << "Extracting with polygon";
      boost::progress_display disp(m_NumFibers);
      for (int i=0; i<m_NumFibers; i++)
      {
        ++disp ;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        for (int j=0; j<numPoints-1; j++)
        {
          // Inputs
          double p1[3] = {0,0,0};
          points->GetPoint(j, p1);
          double p2[3] = {0,0,0};
          points->GetPoint(j+1, p2);
          double tolerance = 0.001;

          // Outputs
          double t = 0; // Parametric coordinate of intersection (0 (corresponding to p1) to 1 (corresponding to p2))
          double x[3] = {0,0,0}; // The coordinate of the intersection
          double pcoords[3] = {0,0,0};
          int subId = 0;

          int iD = polygonVtk->IntersectWithLine(p1, p2, tolerance, t, x, pcoords, subId);
          if (iD!=0)
          {
            result.push_back(i);
            break;
          }
        }
      }
    }
    else if ( dynamic_cast<mitk::PlanarCircle*>(roi->GetData()) )
    {
      mitk::PlanarFigure::Pointer planarFigure = dynamic_cast<mitk::PlanarFigure*>(roi->GetData());
      Vector3D planeNormal = planarFigure->GetPlaneGeometry()->GetNormal();
      planeNormal.Normalize();

      //calculate circle radius
      mitk::Point3D V1w = planarFigure->GetWorldControlPoint(0); //centerPoint
      mitk::Point3D V2w  = planarFigure->GetWorldControlPoint(1); //radiusPoint

      double radius = V1w.EuclideanDistanceTo(V2w);
      radius *= radius;

      MITK_INFO << "Extracting with circle";
      boost::progress_display disp(m_NumFibers);
      for (int i=0; i<m_NumFibers; i++)
      {
        ++disp ;
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        vtkPoints* points = cell->GetPoints();

        for (int j=0; j<numPoints-1; j++)
        {
          // Inputs
          double p1[3] = {0,0,0};
          points->GetPoint(j, p1);
          double p2[3] = {0,0,0};
          points->GetPoint(j+1, p2);

          // Outputs
          double t = 0; // Parametric coordinate of intersection (0 (corresponding to p1) to 1 (corresponding to p2))
          double x[3] = {0,0,0}; // The coordinate of the intersection

          int iD = vtkPlane::IntersectWithLine(p1,p2,planeNormal.GetDataPointer(),V1w.GetDataPointer(),t,x);

          if (iD!=0)
          {
            double dist = (x[0]-V1w[0])*(x[0]-V1w[0])+(x[1]-V1w[1])*(x[1]-V1w[1])+(x[2]-V1w[2])*(x[2]-V1w[2]);
            if( dist <= radius)
            {
              result.push_back(i);
              break;
            }
          }
        }
      }
    }
    return result;
  }

  return result;
}

void mitk::FiberBundle::UpdateFiberGeometry()
{
  vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
  cleaner->SetInputData(m_FiberPolyData);
  cleaner->PointMergingOff();
  cleaner->Update();
  m_FiberPolyData = cleaner->GetOutput();

  m_FiberLengths.clear();
  m_MeanFiberLength = 0;
  m_MedianFiberLength = 0;
  m_LengthStDev = 0;
  m_NumFibers = m_FiberPolyData->GetNumberOfCells();

  if (m_FiberColors==nullptr || m_FiberColors->GetNumberOfTuples()!=m_FiberPolyData->GetNumberOfPoints())
    this->ColorFibersByOrientation();

  if (m_FiberWeights->GetNumberOfValues()!=m_NumFibers)
  {
    m_FiberWeights = vtkSmartPointer<vtkFloatArray>::New();
    m_FiberWeights->SetName("FIBER_WEIGHTS");
    m_FiberWeights->SetNumberOfValues(m_NumFibers);
    this->SetFiberWeights(1);
  }

  if (m_NumFibers<=0) // no fibers present; apply default geometry
  {
    m_MinFiberLength = 0;
    m_MaxFiberLength = 0;
    mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
    geometry->SetImageGeometry(false);
    float b[] = {0, 1, 0, 1, 0, 1};
    geometry->SetFloatBounds(b);
    SetGeometry(geometry);
    return;
  }
  double b[6];
  m_FiberPolyData->GetBounds(b);

  // calculate statistics
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int p = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();
    float length = 0;
    for (int j=0; j<p-1; j++)
    {
      double p1[3];
      points->GetPoint(j, p1);
      double p2[3];
      points->GetPoint(j+1, p2);

      float dist = std::sqrt((p1[0]-p2[0])*(p1[0]-p2[0])+(p1[1]-p2[1])*(p1[1]-p2[1])+(p1[2]-p2[2])*(p1[2]-p2[2]));
      length += dist;
    }
    m_FiberLengths.push_back(length);
    m_MeanFiberLength += length;
    if (i==0)
    {
      m_MinFiberLength = length;
      m_MaxFiberLength = length;
    }
    else
    {
      if (length<m_MinFiberLength)
        m_MinFiberLength = length;
      if (length>m_MaxFiberLength)
        m_MaxFiberLength = length;
    }
  }
  m_MeanFiberLength /= m_NumFibers;

  std::vector< float > sortedLengths = m_FiberLengths;
  std::sort(sortedLengths.begin(), sortedLengths.end());
  for (int i=0; i<m_NumFibers; i++)
    m_LengthStDev += (m_MeanFiberLength-sortedLengths.at(i))*(m_MeanFiberLength-sortedLengths.at(i));
  if (m_NumFibers>1)
    m_LengthStDev /= (m_NumFibers-1);
  else
    m_LengthStDev = 0;
  m_LengthStDev = std::sqrt(m_LengthStDev);
  m_MedianFiberLength = sortedLengths.at(m_NumFibers/2);

  mitk::Geometry3D::Pointer geometry = mitk::Geometry3D::New();
  geometry->SetFloatBounds(b);
  this->SetGeometry(geometry);

  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

float mitk::FiberBundle::GetFiberWeight(unsigned int fiber) const
{
  return m_FiberWeights->GetValue(fiber);
}

void mitk::FiberBundle::SetFiberWeights(float newWeight)
{
  for (int i=0; i<m_FiberWeights->GetNumberOfValues(); i++)
    m_FiberWeights->SetValue(i, newWeight);
}

void mitk::FiberBundle::SetFiberWeights(vtkSmartPointer<vtkFloatArray> weights)
{
  if (m_NumFibers!=weights->GetNumberOfValues())
  {
    MITK_INFO << "Weights array not equal to number of fibers! " << weights->GetNumberOfValues() << " vs " << m_NumFibers;
    return;
  }

  for (int i=0; i<weights->GetNumberOfValues(); i++)
    m_FiberWeights->SetValue(i, weights->GetValue(i));

  m_FiberWeights->SetName("FIBER_WEIGHTS");
}

void mitk::FiberBundle::SetFiberWeight(unsigned int fiber, float weight)
{
  m_FiberWeights->SetValue(fiber, weight);
}

void mitk::FiberBundle::SetFiberColors(vtkSmartPointer<vtkUnsignedCharArray> fiberColors)
{
  for(long i=0; i<m_FiberPolyData->GetNumberOfPoints(); ++i)
  {
    unsigned char source[4] = {0,0,0,0};
    fiberColors->GetTypedTuple(i, source);

    unsigned char target[4] = {0,0,0,0};
    target[0] = source[0];
    target[1] = source[1];
    target[2] = source[2];
    target[3] = source[3];
    m_FiberColors->InsertTypedTuple(i, target);
  }
  m_UpdateTime3D.Modified();
  m_UpdateTime2D.Modified();
}

itk::Matrix< double, 3, 3 > mitk::FiberBundle::TransformMatrix(itk::Matrix< double, 3, 3 > m, double rx, double ry, double rz)
{
  rx = rx*itk::Math::pi/180;
  ry = ry*itk::Math::pi/180;
  rz = rz*itk::Math::pi/180;

  itk::Matrix< double, 3, 3 > rotX; rotX.SetIdentity();
  rotX[1][1] = cos(rx);
  rotX[2][2] = rotX[1][1];
  rotX[1][2] = -sin(rx);
  rotX[2][1] = -rotX[1][2];

  itk::Matrix< double, 3, 3 > rotY; rotY.SetIdentity();
  rotY[0][0] = cos(ry);
  rotY[2][2] = rotY[0][0];
  rotY[0][2] = sin(ry);
  rotY[2][0] = -rotY[0][2];

  itk::Matrix< double, 3, 3 > rotZ; rotZ.SetIdentity();
  rotZ[0][0] = cos(rz);
  rotZ[1][1] = rotZ[0][0];
  rotZ[0][1] = -sin(rz);
  rotZ[1][0] = -rotZ[0][1];

  itk::Matrix< double, 3, 3 > rot = rotZ*rotY*rotX;

  m = rot*m;

  return m;
}

itk::Point<float, 3> mitk::FiberBundle::TransformPoint(vnl_vector_fixed< double, 3 > point, double rx, double ry, double rz, double tx, double ty, double tz)
{
  rx = rx*itk::Math::pi/180;
  ry = ry*itk::Math::pi/180;
  rz = rz*itk::Math::pi/180;

  vnl_matrix_fixed< double, 3, 3 > rotX; rotX.set_identity();
  rotX[1][1] = cos(rx);
  rotX[2][2] = rotX[1][1];
  rotX[1][2] = -sin(rx);
  rotX[2][1] = -rotX[1][2];

  vnl_matrix_fixed< double, 3, 3 > rotY; rotY.set_identity();
  rotY[0][0] = cos(ry);
  rotY[2][2] = rotY[0][0];
  rotY[0][2] = sin(ry);
  rotY[2][0] = -rotY[0][2];

  vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
  rotZ[0][0] = cos(rz);
  rotZ[1][1] = rotZ[0][0];
  rotZ[0][1] = -sin(rz);
  rotZ[1][0] = -rotZ[0][1];

  vnl_matrix_fixed< double, 3, 3 > rot = rotZ*rotY*rotX;

  mitk::BaseGeometry::Pointer geom = this->GetGeometry();
  mitk::Point3D center = geom->GetCenter();

  point[0] -= center[0];
  point[1] -= center[1];
  point[2] -= center[2];
  point = rot*point;
  point[0] += center[0]+tx;
  point[1] += center[1]+ty;
  point[2] += center[2]+tz;
  itk::Point<float, 3> out; out[0] = point[0]; out[1] = point[1]; out[2] = point[2];
  return out;
}


void mitk::FiberBundle::TransformFibers(itk::ScalableAffineTransform< mitk::ScalarType >::Pointer transform)
{
  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  for (int i=0; i<m_NumFibers; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      itk::Point<float, 3> p = GetItkPoint(points->GetPoint(j));
      p = transform->TransformPoint(p);
      vtkIdType id = vtkNewPoints->InsertNextPoint(p.GetDataPointer());
      container->GetPointIds()->InsertNextId(id);
    }
    vtkNewCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::TransformFibers(double rx, double ry, double rz, double tx, double ty, double tz)
{
  rx = rx*itk::Math::pi/180;
  ry = ry*itk::Math::pi/180;
  rz = rz*itk::Math::pi/180;

  vnl_matrix_fixed< double, 3, 3 > rotX; rotX.set_identity();
  rotX[1][1] = cos(rx);
  rotX[2][2] = rotX[1][1];
  rotX[1][2] = -sin(rx);
  rotX[2][1] = -rotX[1][2];

  vnl_matrix_fixed< double, 3, 3 > rotY; rotY.set_identity();
  rotY[0][0] = cos(ry);
  rotY[2][2] = rotY[0][0];
  rotY[0][2] = sin(ry);
  rotY[2][0] = -rotY[0][2];

  vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
  rotZ[0][0] = cos(rz);
  rotZ[1][1] = rotZ[0][0];
  rotZ[0][1] = -sin(rz);
  rotZ[1][0] = -rotZ[0][1];

  vnl_matrix_fixed< double, 3, 3 > rot = rotZ*rotY*rotX;

  mitk::BaseGeometry::Pointer geom = this->GetGeometry();
  mitk::Point3D center = geom->GetCenter();

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  for (int i=0; i<m_NumFibers; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double* p = points->GetPoint(j);
      vnl_vector_fixed< double, 3 > dir;
      dir[0] = p[0]-center[0];
      dir[1] = p[1]-center[1];
      dir[2] = p[2]-center[2];
      dir = rot*dir;
      dir[0] += center[0]+tx;
      dir[1] += center[1]+ty;
      dir[2] += center[2]+tz;
      vtkIdType id = vtkNewPoints->InsertNextPoint(dir.data_block());
      container->GetPointIds()->InsertNextId(id);
    }
    vtkNewCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::RotateAroundAxis(double x, double y, double z)
{
  x = x*itk::Math::pi/180;
  y = y*itk::Math::pi/180;
  z = z*itk::Math::pi/180;

  vnl_matrix_fixed< double, 3, 3 > rotX; rotX.set_identity();
  rotX[1][1] = cos(x);
  rotX[2][2] = rotX[1][1];
  rotX[1][2] = -sin(x);
  rotX[2][1] = -rotX[1][2];

  vnl_matrix_fixed< double, 3, 3 > rotY; rotY.set_identity();
  rotY[0][0] = cos(y);
  rotY[2][2] = rotY[0][0];
  rotY[0][2] = sin(y);
  rotY[2][0] = -rotY[0][2];

  vnl_matrix_fixed< double, 3, 3 > rotZ; rotZ.set_identity();
  rotZ[0][0] = cos(z);
  rotZ[1][1] = rotZ[0][0];
  rotZ[0][1] = -sin(z);
  rotZ[1][0] = -rotZ[0][1];

  mitk::BaseGeometry::Pointer geom = this->GetGeometry();
  mitk::Point3D center = geom->GetCenter();

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  for (int i=0; i<m_NumFibers; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double* p = points->GetPoint(j);
      vnl_vector_fixed< double, 3 > dir;
      dir[0] = p[0]-center[0];
      dir[1] = p[1]-center[1];
      dir[2] = p[2]-center[2];
      dir = rotZ*rotY*rotX*dir;
      dir[0] += center[0];
      dir[1] += center[1];
      dir[2] += center[2];
      vtkIdType id = vtkNewPoints->InsertNextPoint(dir.data_block());
      container->GetPointIds()->InsertNextId(id);
    }
    vtkNewCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::ScaleFibers(double x, double y, double z, bool subtractCenter)
{
  MITK_INFO << "Scaling fibers";
  boost::progress_display disp(m_NumFibers);

  mitk::BaseGeometry* geom = this->GetGeometry();
  mitk::Point3D c = geom->GetCenter();

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  for (int i=0; i<m_NumFibers; i++)
  {
    ++disp ;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double* p = points->GetPoint(j);
      if (subtractCenter)
      {
        p[0] -= c[0]; p[1] -= c[1]; p[2] -= c[2];
      }
      p[0] *= x;
      p[1] *= y;
      p[2] *= z;
      if (subtractCenter)
      {
        p[0] += c[0]; p[1] += c[1]; p[2] += c[2];
      }
      vtkIdType id = vtkNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    vtkNewCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::TranslateFibers(double x, double y, double z)
{
  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  for (int i=0; i<m_NumFibers; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double* p = points->GetPoint(j);
      p[0] += x;
      p[1] += y;
      p[2] += z;
      vtkIdType id = vtkNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    vtkNewCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::MirrorFibers(unsigned int axis)
{
  if (axis>2)
    return;

  MITK_INFO << "Mirroring fibers";
  boost::progress_display disp(m_NumFibers);

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  for (int i=0; i<m_NumFibers; i++)
  {
    ++disp;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints; j++)
    {
      double* p = points->GetPoint(j);
      p[axis] = -p[axis];
      vtkIdType id = vtkNewPoints->InsertNextPoint(p);
      container->GetPointIds()->InsertNextId(id);
    }
    vtkNewCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::RemoveDir(vnl_vector_fixed<double,3> dir, double threshold)
{
  dir.normalize();
  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    ++disp ;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    // calculate curvatures
    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    bool discard = false;
    for (int j=0; j<numPoints-1; j++)
    {
      double p1[3];
      points->GetPoint(j, p1);
      double p2[3];
      points->GetPoint(j+1, p2);

      vnl_vector_fixed< double, 3 > v1;
      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];
      if (v1.magnitude()>0.001)
      {
        v1.normalize();

        if (fabs(dot_product(v1,dir))>threshold)
        {
          discard = true;
          break;
        }
      }
    }
    if (!discard)
    {
      for (int j=0; j<numPoints; j++)
      {
        double p1[3];
        points->GetPoint(j, p1);

        vtkIdType id = vtkNewPoints->InsertNextPoint(p1);
        container->GetPointIds()->InsertNextId(id);
      }
      vtkNewCells->InsertNextCell(container);
    }
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);

  this->SetFiberPolyData(m_FiberPolyData, true);

  //    UpdateColorCoding();
  //    UpdateFiberGeometry();
}

bool mitk::FiberBundle::ApplyCurvatureThreshold(float minRadius, bool deleteFibers)
{
  if (minRadius<0)
    return true;

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  MITK_INFO << "Applying curvature threshold";
  boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    ++disp ;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    // calculate curvatures
    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    for (int j=0; j<numPoints-2; j++)
    {
      double p1[3];
      points->GetPoint(j, p1);
      double p2[3];
      points->GetPoint(j+1, p2);
      double p3[3];
      points->GetPoint(j+2, p3);

      vnl_vector_fixed< float, 3 > v1, v2, v3;

      v1[0] = p2[0]-p1[0];
      v1[1] = p2[1]-p1[1];
      v1[2] = p2[2]-p1[2];

      v2[0] = p3[0]-p2[0];
      v2[1] = p3[1]-p2[1];
      v2[2] = p3[2]-p2[2];

      v3[0] = p1[0]-p3[0];
      v3[1] = p1[1]-p3[1];
      v3[2] = p1[2]-p3[2];

      float a = v1.magnitude();
      float b = v2.magnitude();
      float c = v3.magnitude();
      float r = a*b*c/std::sqrt((a+b+c)*(a+b-c)*(b+c-a)*(a-b+c)); // radius of triangle via Heron's formula (area of triangle)

      vtkIdType id = vtkNewPoints->InsertNextPoint(p1);
      container->GetPointIds()->InsertNextId(id);

      if (deleteFibers && r<minRadius)
        break;

      if (r<minRadius)
      {
        j += 2;
        vtkNewCells->InsertNextCell(container);
        container = vtkSmartPointer<vtkPolyLine>::New();
      }
      else if (j==numPoints-3)
      {
        id = vtkNewPoints->InsertNextPoint(p2);
        container->GetPointIds()->InsertNextId(id);
        id = vtkNewPoints->InsertNextPoint(p3);
        container->GetPointIds()->InsertNextId(id);
        vtkNewCells->InsertNextCell(container);
      }
    }
  }

  if (vtkNewCells->GetNumberOfCells()<=0)
    return false;

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
  return true;
}

bool mitk::FiberBundle::RemoveShortFibers(float lengthInMM)
{
  MITK_INFO << "Removing short fibers";
  if (lengthInMM<=0 || lengthInMM<m_MinFiberLength)
  {
    MITK_INFO << "No fibers shorter than " << lengthInMM << " mm found!";
    return true;
  }

  if (lengthInMM>m_MaxFiberLength)    // can't remove all fibers
  {
    MITK_WARN << "Process aborted. No fibers would be left!";
    return false;
  }

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();
  float min = m_MaxFiberLength;

  boost::progress_display disp(m_NumFibers);
  for (int i=0; i<m_NumFibers; i++)
  {
    ++disp;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (m_FiberLengths.at(i)>=lengthInMM)
    {
      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
      for (int j=0; j<numPoints; j++)
      {
        double* p = points->GetPoint(j);
        vtkIdType id = vtkNewPoints->InsertNextPoint(p);
        container->GetPointIds()->InsertNextId(id);
      }
      vtkNewCells->InsertNextCell(container);
      if (m_FiberLengths.at(i)<min)
        min = m_FiberLengths.at(i);
    }
  }

  if (vtkNewCells->GetNumberOfCells()<=0)
    return false;

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
  return true;
}

bool mitk::FiberBundle::RemoveLongFibers(float lengthInMM)
{
  if (lengthInMM<=0 || lengthInMM>m_MaxFiberLength)
    return true;

  if (lengthInMM<m_MinFiberLength)    // can't remove all fibers
    return false;

  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  MITK_INFO << "Removing long fibers";
  boost::progress_display disp(m_NumFibers);
  for (int i=0; i<m_NumFibers; i++)
  {
    ++disp;
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    if (m_FiberLengths.at(i)<=lengthInMM)
    {
      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
      for (int j=0; j<numPoints; j++)
      {
        double* p = points->GetPoint(j);
        vtkIdType id = vtkNewPoints->InsertNextPoint(p);
        container->GetPointIds()->InsertNextId(id);
      }
      vtkNewCells->InsertNextCell(container);
    }
  }

  if (vtkNewCells->GetNumberOfCells()<=0)
    return false;

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkNewPoints);
  m_FiberPolyData->SetLines(vtkNewCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
  return true;
}

void mitk::FiberBundle::ResampleSpline(float pointDistance, double tension, double continuity, double bias )
{
  if (pointDistance<=0)
    return;

  vtkSmartPointer<vtkPoints> vtkSmoothPoints = vtkSmartPointer<vtkPoints>::New(); //in smoothpoints the interpolated points representing a fiber are stored.

  //in vtkcells all polylines are stored, actually all id's of them are stored
  vtkSmartPointer<vtkCellArray> vtkSmoothCells = vtkSmartPointer<vtkCellArray>::New(); //cellcontainer for smoothed lines

  MITK_INFO << "Smoothing fibers";
  vtkSmartPointer<vtkFloatArray> newFiberWeights = vtkSmartPointer<vtkFloatArray>::New();
  newFiberWeights->SetName("FIBER_WEIGHTS");
  newFiberWeights->SetNumberOfValues(m_NumFibers);

  std::vector< vtkSmartPointer<vtkPolyLine> > resampled_streamlines;
  resampled_streamlines.resize(m_NumFibers);

  boost::progress_display disp(m_NumFibers);
#pragma omp parallel for
  for (int i=0; i<m_NumFibers; i++)
  {
    vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
    float length = 0;
#pragma omp critical
    {
      length = m_FiberLengths.at(i);
      ++disp;
      vtkCell* cell = m_FiberPolyData->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();
      for (int j=0; j<numPoints; j++)
        newPoints->InsertNextPoint(points->GetPoint(j));
    }

    int sampling = std::ceil(length/pointDistance);

    vtkSmartPointer<vtkKochanekSpline> xSpline = vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkKochanekSpline> ySpline = vtkSmartPointer<vtkKochanekSpline>::New();
    vtkSmartPointer<vtkKochanekSpline> zSpline = vtkSmartPointer<vtkKochanekSpline>::New();
    xSpline->SetDefaultBias(bias); xSpline->SetDefaultTension(tension); xSpline->SetDefaultContinuity(continuity);
    ySpline->SetDefaultBias(bias); ySpline->SetDefaultTension(tension); ySpline->SetDefaultContinuity(continuity);
    zSpline->SetDefaultBias(bias); zSpline->SetDefaultTension(tension); zSpline->SetDefaultContinuity(continuity);

    vtkSmartPointer<vtkParametricSpline> spline = vtkSmartPointer<vtkParametricSpline>::New();
    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(newPoints);

    vtkSmartPointer<vtkParametricFunctionSource> functionSource = vtkSmartPointer<vtkParametricFunctionSource>::New();
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(sampling);
    functionSource->SetVResolution(sampling);
    functionSource->SetWResolution(sampling);
    functionSource->Update();

    vtkPolyData* outputFunction = functionSource->GetOutput();
    vtkPoints* tmpSmoothPnts = outputFunction->GetPoints(); //smoothPoints of current fiber

    vtkSmartPointer<vtkPolyLine> smoothLine = vtkSmartPointer<vtkPolyLine>::New();

#pragma omp critical
    {
      for (int j=0; j<tmpSmoothPnts->GetNumberOfPoints(); j++)
      {
        vtkIdType id = vtkSmoothPoints->InsertNextPoint(tmpSmoothPnts->GetPoint(j));
        smoothLine->GetPointIds()->InsertNextId(id);
      }

      resampled_streamlines[i] = smoothLine;
    }
  }

  for (auto container : resampled_streamlines)
  {
    vtkSmoothCells->InsertNextCell(container);
  }

  m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
  m_FiberPolyData->SetPoints(vtkSmoothPoints);
  m_FiberPolyData->SetLines(vtkSmoothCells);
  this->SetFiberPolyData(m_FiberPolyData, true);
}

void mitk::FiberBundle::ResampleSpline(float pointDistance)
{
  ResampleSpline(pointDistance, 0, 0, 0 );
}

unsigned long mitk::FiberBundle::GetNumberOfPoints() const
{
  unsigned long points = 0;
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    points += cell->GetNumberOfPoints();
  }
  return points;
}

void mitk::FiberBundle::Compress(float error)
{
  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  MITK_INFO << "Compressing fibers";
  unsigned long numRemovedPoints = 0;
  boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
  vtkSmartPointer<vtkFloatArray> newFiberWeights = vtkSmartPointer<vtkFloatArray>::New();
  newFiberWeights->SetName("FIBER_WEIGHTS");
  newFiberWeights->SetNumberOfValues(m_NumFibers);

#pragma omp parallel for
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {

    std::vector< vnl_vector_fixed< double, 3 > > vertices;
    float weight = 1;

#pragma omp critical
    {
      ++disp;
      weight = m_FiberWeights->GetValue(i);
      vtkCell* cell = m_FiberPolyData->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      for (int j=0; j<numPoints; j++)
      {
        double cand[3];
        points->GetPoint(j, cand);
        vnl_vector_fixed< double, 3 > candV;
        candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2];
        vertices.push_back(candV);
      }
    }

    // calculate curvatures
    int numPoints = vertices.size();
    std::vector< int > removedPoints; removedPoints.resize(numPoints, 0);
    removedPoints[0]=-1; removedPoints[numPoints-1]=-1;

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    int remCounter = 0;

    bool pointFound = true;
    while (pointFound)
    {
      pointFound = false;
      double minError = error;
      int removeIndex = -1;

      for (unsigned int j=0; j<vertices.size(); j++)
      {
        if (removedPoints[j]==0)
        {
          vnl_vector_fixed< double, 3 > candV = vertices.at(j);

          int validP = -1;
          vnl_vector_fixed< double, 3 > pred;
          for (int k=j-1; k>=0; k--)
            if (removedPoints[k]<=0)
            {
              pred = vertices.at(k);
              validP = k;
              break;
            }
          int validS = -1;
          vnl_vector_fixed< double, 3 > succ;
          for (int k=j+1; k<numPoints; k++)
            if (removedPoints[k]<=0)
            {
              succ = vertices.at(k);
              validS = k;
              break;
            }

          if (validP>=0 && validS>=0)
          {
            double a = (candV-pred).magnitude();
            double b = (candV-succ).magnitude();
            double c = (pred-succ).magnitude();
            double s=0.5*(a+b+c);
            double hc=(2.0/c)*sqrt(fabs(s*(s-a)*(s-b)*(s-c)));

            if (hc<minError)
            {
              removeIndex = j;
              minError = hc;
              pointFound = true;
            }
          }
        }
      }

      if (pointFound)
      {
        removedPoints[removeIndex] = 1;
        remCounter++;
      }
    }

    for (int j=0; j<numPoints; j++)
    {
      if (removedPoints[j]<=0)
      {
#pragma omp critical
        {
          vtkIdType id = vtkNewPoints->InsertNextPoint(vertices.at(j).data_block());
          container->GetPointIds()->InsertNextId(id);
        }
      }
    }

#pragma omp critical
    {
      newFiberWeights->SetValue(vtkNewCells->GetNumberOfCells(), weight);
      numRemovedPoints += remCounter;
      vtkNewCells->InsertNextCell(container);
    }
  }

  if (vtkNewCells->GetNumberOfCells()>0)
  {
    MITK_INFO << "Removed points: " << numRemovedPoints;
    SetFiberWeights(newFiberWeights);
    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
  }
}

void mitk::FiberBundle::ResampleToNumPoints(unsigned int targetPoints)
{
  if (targetPoints<2)
    mitkThrow() << "Minimum two points required for resampling!";

  MITK_INFO << "Resampling fibers (number of points " << targetPoints << ")";

  bool unequal_fibs = true;
  while (unequal_fibs)
  {
    vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

    vtkSmartPointer<vtkFloatArray> newFiberWeights = vtkSmartPointer<vtkFloatArray>::New();
    newFiberWeights->SetName("FIBER_WEIGHTS");
    newFiberWeights->SetNumberOfValues(m_NumFibers);

    unequal_fibs = false;
//#pragma omp parallel for
    for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
    {

      std::vector< vnl_vector_fixed< double, 3 > > vertices;
      float weight = 1;

      double seg_len = 0;

//#pragma omp critical
      {
        weight = m_FiberWeights->GetValue(i);
        vtkCell* cell = m_FiberPolyData->GetCell(i);
        int numPoints = cell->GetNumberOfPoints();
        if ((unsigned int)numPoints!=targetPoints)
          seg_len = this->GetFiberLength(i)/(targetPoints-1);;
        vtkPoints* points = cell->GetPoints();

        for (int j=0; j<numPoints; j++)
        {
          double cand[3];
          points->GetPoint(j, cand);
          vnl_vector_fixed< double, 3 > candV;
          candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2];
          vertices.push_back(candV);
        }
      }

      vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
      vnl_vector_fixed< double, 3 > lastV = vertices.at(0);

//#pragma omp critical
      {
        vtkIdType id = vtkNewPoints->InsertNextPoint(lastV.data_block());
        container->GetPointIds()->InsertNextId(id);
      }
      for (unsigned int j=1; j<vertices.size(); j++)
      {
        vnl_vector_fixed< double, 3 > vec = vertices.at(j) - lastV;
        double new_dist = vec.magnitude();

        if (new_dist >= seg_len && seg_len>0)
        {
          vnl_vector_fixed< double, 3 > newV = lastV;
          if ( new_dist-seg_len <= mitk::eps )
          {
            vec.normalize();
            newV += vec * seg_len;
          }
          else
          {
            // intersection between sphere (radius 'pointDistance', center 'lastV') and line (direction 'd' and point 'p')
            vnl_vector_fixed< double, 3 > p = vertices.at(j-1);
            vnl_vector_fixed< double, 3 > d = vertices.at(j) - p;

            double a = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
            double b = 2 * (d[0] * (p[0] - lastV[0]) + d[1] * (p[1] - lastV[1]) + d[2] * (p[2] - lastV[2]));
            double c = (p[0] - lastV[0])*(p[0] - lastV[0]) + (p[1] - lastV[1])*(p[1] - lastV[1]) + (p[2] - lastV[2])*(p[2] - lastV[2]) - seg_len*seg_len;

            double v1 =(-b + std::sqrt(b*b-4*a*c))/(2*a);
            double v2 =(-b - std::sqrt(b*b-4*a*c))/(2*a);

            if (v1>0)
              newV = p + d * v1;
            else if (v2>0)
              newV = p + d * v2;
            else
              MITK_INFO << "ERROR1 - linear resampling";

            j--;
          }

//#pragma omp critical
          {
            vtkIdType id = vtkNewPoints->InsertNextPoint(newV.data_block());
            container->GetPointIds()->InsertNextId(id);
          }
          lastV = newV;
        }
        else if ( (j==vertices.size()-1 && new_dist>0.0001) || seg_len==0)
        {
//#pragma omp critical
          {
            vtkIdType id = vtkNewPoints->InsertNextPoint(vertices.at(j).data_block());
            container->GetPointIds()->InsertNextId(id);
          }
        }
      }

//#pragma omp critical
      {
        newFiberWeights->SetValue(vtkNewCells->GetNumberOfCells(), weight);
        vtkNewCells->InsertNextCell(container);
        if (container->GetNumberOfPoints()!=targetPoints)
          unequal_fibs = true;
      }
    }

    if (vtkNewCells->GetNumberOfCells()>0)
    {
      SetFiberWeights(newFiberWeights);
      m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
      m_FiberPolyData->SetPoints(vtkNewPoints);
      m_FiberPolyData->SetLines(vtkNewCells);
      this->SetFiberPolyData(m_FiberPolyData, true);
    }
  }
}

void mitk::FiberBundle::ResampleLinear(double pointDistance)
{
  vtkSmartPointer<vtkPoints> vtkNewPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> vtkNewCells = vtkSmartPointer<vtkCellArray>::New();

  MITK_INFO << "Resampling fibers (linear)";
  boost::progress_display disp(m_FiberPolyData->GetNumberOfCells());
  vtkSmartPointer<vtkFloatArray> newFiberWeights = vtkSmartPointer<vtkFloatArray>::New();
  newFiberWeights->SetName("FIBER_WEIGHTS");
  newFiberWeights->SetNumberOfValues(m_NumFibers);

  std::vector< vtkSmartPointer<vtkPolyLine> > resampled_streamlines;
  resampled_streamlines.resize(m_FiberPolyData->GetNumberOfCells());

#pragma omp parallel for
  for (int i=0; i<m_FiberPolyData->GetNumberOfCells(); i++)
  {

    std::vector< vnl_vector_fixed< double, 3 > > vertices;

#pragma omp critical
    {
      ++disp;
      vtkCell* cell = m_FiberPolyData->GetCell(i);
      int numPoints = cell->GetNumberOfPoints();
      vtkPoints* points = cell->GetPoints();

      for (int j=0; j<numPoints; j++)
      {
        double cand[3];
        points->GetPoint(j, cand);
        vnl_vector_fixed< double, 3 > candV;
        candV[0]=cand[0]; candV[1]=cand[1]; candV[2]=cand[2];
        vertices.push_back(candV);
      }
    }

    vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
    vnl_vector_fixed< double, 3 > lastV = vertices.at(0);

#pragma omp critical
    {
      vtkIdType id = vtkNewPoints->InsertNextPoint(lastV.data_block());
      container->GetPointIds()->InsertNextId(id);
    }
    for (unsigned int j=1; j<vertices.size(); j++)
    {
      vnl_vector_fixed< double, 3 > vec = vertices.at(j) - lastV;
      double new_dist = vec.magnitude();

      if (new_dist >= pointDistance)
      {
        vnl_vector_fixed< double, 3 > newV = lastV;
        if ( new_dist-pointDistance <= mitk::eps )
        {
          vec.normalize();
          newV += vec * pointDistance;
        }
        else
        {
          // intersection between sphere (radius 'pointDistance', center 'lastV') and line (direction 'd' and point 'p')
          vnl_vector_fixed< double, 3 > p = vertices.at(j-1);
          vnl_vector_fixed< double, 3 > d = vertices.at(j) - p;

          double a = d[0]*d[0] + d[1]*d[1] + d[2]*d[2];
          double b = 2 * (d[0] * (p[0] - lastV[0]) + d[1] * (p[1] - lastV[1]) + d[2] * (p[2] - lastV[2]));
          double c = (p[0] - lastV[0])*(p[0] - lastV[0]) + (p[1] - lastV[1])*(p[1] - lastV[1]) + (p[2] - lastV[2])*(p[2] - lastV[2]) - pointDistance*pointDistance;

          double v1 =(-b + std::sqrt(b*b-4*a*c))/(2*a);
          double v2 =(-b - std::sqrt(b*b-4*a*c))/(2*a);

          if (v1>0)
            newV = p + d * v1;
          else if (v2>0)
            newV = p + d * v2;
          else
            MITK_INFO << "ERROR1 - linear resampling";

          j--;
        }

#pragma omp critical
        {
          vtkIdType id = vtkNewPoints->InsertNextPoint(newV.data_block());
          container->GetPointIds()->InsertNextId(id);
        }
        lastV = newV;
      }
      else if (j==vertices.size()-1 && new_dist>0.0001)
      {
#pragma omp critical
        {
          vtkIdType id = vtkNewPoints->InsertNextPoint(vertices.at(j).data_block());
          container->GetPointIds()->InsertNextId(id);
        }
      }
    }

#pragma omp critical
    {
      resampled_streamlines[i] = container;
    }
  }

  for (auto container : resampled_streamlines)
  {
    vtkNewCells->InsertNextCell(container);
  }

  if (vtkNewCells->GetNumberOfCells()>0)
  {
    m_FiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    m_FiberPolyData->SetPoints(vtkNewPoints);
    m_FiberPolyData->SetLines(vtkNewCells);
    this->SetFiberPolyData(m_FiberPolyData, true);
  }
}

// reapply selected colorcoding in case PolyData structure has changed
bool mitk::FiberBundle::Equals(mitk::FiberBundle* fib, double eps)
{
  if (fib==nullptr)
  {
    MITK_INFO << "Reference bundle is nullptr!";
    return false;
  }

  if (m_NumFibers!=fib->GetNumFibers())
  {
    MITK_INFO << "Unequal number of fibers!";
    MITK_INFO << m_NumFibers << " vs. " << fib->GetNumFibers();
    return false;
  }

  for (int i=0; i<m_NumFibers; i++)
  {
    vtkCell* cell = m_FiberPolyData->GetCell(i);
    int numPoints = cell->GetNumberOfPoints();
    vtkPoints* points = cell->GetPoints();

    vtkCell* cell2 = fib->GetFiberPolyData()->GetCell(i);
    int numPoints2 = cell2->GetNumberOfPoints();
    vtkPoints* points2 = cell2->GetPoints();

    if (numPoints2!=numPoints)
    {
      MITK_INFO << "Unequal number of points in fiber " << i << "!";
      MITK_INFO << numPoints2 << " vs. " << numPoints;
      return false;
    }

    for (int j=0; j<numPoints; j++)
    {
      double* p1 = points->GetPoint(j);
      double* p2 = points2->GetPoint(j);
      if (fabs(p1[0]-p2[0])>eps || fabs(p1[1]-p2[1])>eps || fabs(p1[2]-p2[2])>eps)
      {
        MITK_INFO << "Unequal points in fiber " << i << " at position " << j << "!";
        MITK_INFO << "p1: " << p1[0] << ", " << p1[1] << ", " << p1[2];
        MITK_INFO << "p2: " << p2[0] << ", " << p2[1] << ", " << p2[2];
        return false;
      }
    }
  }

  return true;
}

void mitk::FiberBundle::PrintSelf(std::ostream &os, itk::Indent indent) const
{
  os << this->GetNameOfClass() << ":\n";
  os << indent << "Number of fibers: " << this->GetNumFibers() << std::endl;
  os << indent << "Min. fiber length: " << this->GetMinFiberLength() << std::endl;
  os << indent << "Max. fiber length: " << this->GetMaxFiberLength() << std::endl;
  os << indent << "Mean fiber length: " << this->GetMeanFiberLength() << std::endl;
  os << indent << "Median fiber length: " << this->GetMedianFiberLength() << std::endl;
  os << indent << "STDEV fiber length: " << this->GetLengthStDev() << std::endl;
  os << indent << "Number of points: " << this->GetNumberOfPoints() << std::endl;

  os << indent << "Extent x: " << this->GetGeometry()->GetExtentInMM(0) << "mm" << std::endl;
  os << indent << "Extent y: " << this->GetGeometry()->GetExtentInMM(1) << "mm" << std::endl;
  os << indent << "Extent z: " << this->GetGeometry()->GetExtentInMM(2) << "mm" << std::endl;
  os << indent << "Diagonal: " << this->GetGeometry()->GetDiagonalLength()  << "mm" << std::endl;

  if (m_FiberWeights!=nullptr)
  {
    std::vector< float > weights;
    for (int i=0; i<m_FiberWeights->GetSize(); i++)
      weights.push_back(m_FiberWeights->GetValue(i));

    std::sort(weights.begin(), weights.end());

    os << indent << "\nFiber weight statistics" << std::endl;
    os << indent << "Min: " << weights.front() << std::endl;
    os << indent << "1% quantile: " << weights.at(weights.size()*0.01) << std::endl;
    os << indent << "5% quantile: " << weights.at(weights.size()*0.05) << std::endl;
    os << indent << "25% quantile: " << weights.at(weights.size()*0.25) << std::endl;
    os << indent << "Median: " << weights.at(weights.size()*0.5) << std::endl;
    os << indent << "75% quantile: " << weights.at(weights.size()*0.75) << std::endl;
    os << indent << "95% quantile: " << weights.at(weights.size()*0.95) << std::endl;
    os << indent << "99% quantile: " << weights.at(weights.size()*0.99) << std::endl;
    os << indent << "Max: " << weights.back() << std::endl;
  }
  else
    os << indent << "\n\nNo fiber weight array found." << std::endl;

  Superclass::PrintSelf(os, indent);
}

/* ESSENTIAL IMPLEMENTATION OF SUPERCLASS METHODS */
void mitk::FiberBundle::UpdateOutputInformation()
{

}
void mitk::FiberBundle::SetRequestedRegionToLargestPossibleRegion()
{

}
bool mitk::FiberBundle::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}
bool mitk::FiberBundle::VerifyRequestedRegion()
{
  return true;
}
void mitk::FiberBundle::SetRequestedRegion(const itk::DataObject* )
{

}
