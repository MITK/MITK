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
  Scale = 1;
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
int vtkOdfSource::RequestData(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **vtkNotUsed(inputVector),
  vtkInformationVector *outputVector)
{
  vtkPolyData* TemplateOdf = OdfType::GetBaseMesh();
  // get the info object
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the ouptut
  vtkPolyData *output = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  OdfType colorOdf;
  switch(Normalization)
  {
  case mitk::ODFN_MINMAX:
    Odf = Odf.MinMaxNormalize();
    colorOdf = Odf;
    break;
  case mitk::ODFN_MAX:
    Odf = Odf.MaxNormalize();
    colorOdf = Odf;
    break;
  case mitk::ODFN_NONE:
    colorOdf = Odf.MaxNormalize();
    break;
  default:
    Odf = Odf.MinMaxNormalize();
    colorOdf = Odf;
  }


  vtkIdType cellId = 0;
  vtkIdType npts; vtkIdType *pts;
  vtkPoints *newPoints;
  vtkCellArray* polys = TemplateOdf->GetPolys();
  output->SetPolys(polys);
  vtkDoubleArray* colors = vtkDoubleArray::New();
  int numCells = polys->GetNumberOfCells();
  colors->Allocate(numCells);
  polys->InitTraversal();
  newPoints = vtkPoints::New();
  int numPoints = TemplateOdf->GetPoints()->GetNumberOfPoints();
  newPoints->Allocate(numPoints);

  while(polys->GetNextCell(npts,pts))
  {
    double val = 0;
    for(int i=0; i<npts; i++)
    {
      vtkIdType pointId = pts[i];
      val += colorOdf.GetElement(pointId);
    }
    val /= npts;
    colors->SetComponent(0,cellId++, 1-val);
  }

  for(int j=0; j<numPoints; j++){
    double p[3];
    TemplateOdf->GetPoints()->GetPoint(j,p);
    double val = Odf.GetElement(j);
    p[0] *= val*Scale*AdditionalScale*0.5;
    p[1] *= val*Scale*AdditionalScale*0.5;
    p[2] *= val*Scale*AdditionalScale*0.5;
    newPoints->InsertNextPoint(p);
  }
  output->SetPoints(newPoints);
  output->GetCellData()->SetScalars(colors);
  colors->Delete();
  newPoints->Delete();
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
