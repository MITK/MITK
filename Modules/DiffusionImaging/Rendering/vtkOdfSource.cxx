#include "vtkOdfSource.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkDoubleArray.h"
#include "vtkCellData.h"

#include <limits>

vtkCxxRevisionMacro(vtkOdfSource, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkOdfSource);

vtkOdfSource::vtkOdfSource()
{
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkOdfSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPoints *newPoints; 
  newPoints = vtkPoints::New();
  int numPoints = TemplateOdf->GetPoints()->GetNumberOfPoints();
  newPoints->Allocate(numPoints);
  for(int j=0; j<numPoints; j++){
    double p[3];
    TemplateOdf->GetPoints()->GetPoint(j,p);
    if(OdfVals->GetNumberOfComponents())
    {
      double val = OdfVals->GetComponent(0,j);
      //if(val >= 0.2)
      //  val = 0.4;
      p[0] *= val*Scale;
      p[1] *= val*Scale;
      p[2] *= val*Scale;
      //double tmp = p[0];
      //p[0] = p[1] * val;
      //p[1] = tmp  * -val;
      //p[2] *= val;
    }
    newPoints->InsertNextPoint(p);
  }
  output->SetPoints(newPoints);
  newPoints->Delete();

  vtkCellArray* polys = TemplateOdf->GetPolys();
  output->SetPolys(polys);

  //vtkCellArray *newPolys;
  //newPolys = vtkCellArray::New();
  //int numCells = polys->GetNumberOfCells();
  //newPolys->Allocate(numCells);
  //polys->InitTraversal();
  //vtkIdType npts; vtkIdType *pts;
  //while(polys->GetNextCell(npts,pts))
  //{
  //  newPolys->InsertNextCell(npts, pts);
  //}
  //output->SetPolys(newPolys);
  //newPolys->Delete();

  vtkDoubleArray* colors = vtkDoubleArray::New();
  int numCells = polys->GetNumberOfCells();
  colors->Allocate(numCells);
  polys->InitTraversal();
  vtkIdType cellId = 0;
  vtkIdType npts; vtkIdType *pts;
  while(polys->GetNextCell(npts,pts))
  {
    double val = 0;
    for(int i=0; i<npts; i++)
    {
      vtkIdType pointId = pts[i];
      val += OdfVals->GetComponent(0,pointId);
    }
    val /= npts;
    colors->SetComponent(0,cellId++,-(val-0.5/*level*/)*(1/0.5/*window*/));
  }
  output->GetCellData()->SetScalars(colors);
  colors->Delete();
  return 1;
}

//----------------------------------------------------------------------------
void vtkOdfSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
int vtkOdfSource::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);
  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::MAXIMUM_NUMBER_OF_PIECES(),
               -1);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_BOUNDING_BOX(),1,1,1,1,1,1);
  
  return 1;
}
