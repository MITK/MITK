#ifndef _BUILDFIBRES
#define _BUILDFIBRES

//#include "matrix.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include <map>
#include <vector>
#include <string.h>

using namespace std;

#define PI M_PI

#include "ParticleGrid.cpp"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyLine.h>
#include <itkVector.h>
#include <itkImage.h>

class FiberBuilder
{
public:
  Particle *particles;
  int pcnt;
  int attrcnt;
  typedef vector< Particle* > ParticleContainerType;
  typedef vector< ParticleContainerType* > FiberContainerType;

  vtkSmartPointer<vtkCellArray> m_VtkCellArray;
  vtkSmartPointer<vtkPoints>    m_VtkPoints;

  typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
  typedef itk::Image<OdfVectorType, 3>        ItkQBallImgType;
  ItkQBallImgType::Pointer                    m_ItkQBallImage;

  FiberBuilder(float *points, int numPoints, double spacing[], ItkQBallImgType::Pointer image)
  {
    m_ItkQBallImage = image;
    particles = (Particle*) malloc(sizeof(Particle)*numPoints);
    pcnt = numPoints;
    attrcnt = 10;
    for (int k = 0; k < numPoints; k++)
    {
      Particle *p = &(particles[k]);
      p->R = pVector(points[attrcnt*k]/spacing[0]-0.5, points[attrcnt*k+1]/spacing[1]-0.5,points[attrcnt*k+2]/spacing[2]-0.5);
      p->N = pVector(points[attrcnt*k+3],points[attrcnt*k+4],points[attrcnt*k+5]);
      p->cap =  points[attrcnt*k+6];
      p->len =  points[attrcnt*k+7];
      p->mID = (int) points[attrcnt*k+8];
      p->pID = (int) points[attrcnt*k+9];
      p->ID = k;
      p->label = 0;
    }
    m_VtkCellArray = vtkSmartPointer<vtkCellArray>::New();
    m_VtkPoints = vtkSmartPointer<vtkPoints>::New();
  }

  ~FiberBuilder()
  {
    free(particles);
  }

  vtkSmartPointer<vtkPolyData> iterate(int minSize)
  {
    int cur_label = 1;
    int numFibers = 0;
    for (int k = 0; k < pcnt;k++)
    {
      Particle *dp =  &(particles[k]);
      if (dp->label == 0)
      {
        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
        dp->label = cur_label;
        dp->numerator = 0;
        labelPredecessors(dp, container);
        labelSuccessors(dp, container);
        cur_label++;
        if(container->GetPointIds()->GetNumberOfIds()>minSize)
        {
          m_VtkCellArray->InsertNextCell(container);
          numFibers++;
        }
      }
    }
    vtkSmartPointer<vtkPolyData> fiberPolyData = vtkSmartPointer<vtkPolyData>::New();
    fiberPolyData->SetPoints(m_VtkPoints);
    fiberPolyData->SetLines(m_VtkCellArray);
    return fiberPolyData;
  }

  void AddPoint(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
  {
    itk::ContinuousIndex<float, 3> index;
    index[0] = dp->R[0];
    index[1] = dp->R[1];
    index[2] = dp->R[2];
    itk::Point<float> point;
    m_ItkQBallImage->TransformContinuousIndexToPhysicalPoint( index, point );
    vtkIdType id = m_VtkPoints->InsertNextPoint(point.GetDataPointer());
    container->GetPointIds()->InsertNextId(id);
  }

  void labelPredecessors(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
  {
    if (dp->mID != -1 && dp->mID!=dp->ID)
    {
      if (dp->ID!=particles[dp->mID].pID)
      {
        if (dp->ID==particles[dp->mID].mID)
        {
          int tmp = particles[dp->mID].pID;
          particles[dp->mID].pID = particles[dp->mID].mID;
          particles[dp->mID].mID = tmp;
        }
      }
      if (particles[dp->mID].label == 0)
      {
        particles[dp->mID].label = dp->label;
        particles[dp->mID].numerator = dp->numerator-1;
        labelPredecessors(&(particles[dp->mID]), container);
      }
    }

    AddPoint(dp, container);
  }

  void labelSuccessors(Particle *dp, vtkSmartPointer<vtkPolyLine> container)
  {
    AddPoint(dp, container);

    if (dp->pID != -1 && dp->pID!=dp->ID)
    {
      if (dp->ID!=particles[dp->pID].mID)
      {
        if (dp->ID==particles[dp->pID].pID)
        {
          int tmp = particles[dp->pID].pID;
          particles[dp->pID].pID = particles[dp->pID].mID;
          particles[dp->pID].mID = tmp;
        }
      }
      if (particles[dp->pID].label == 0)
      {
        particles[dp->pID].label = dp->label;
        particles[dp->pID].numerator = dp->numerator+1;
        labelSuccessors(&(particles[dp->pID]), container);
      }
    }
  }
};

#endif
