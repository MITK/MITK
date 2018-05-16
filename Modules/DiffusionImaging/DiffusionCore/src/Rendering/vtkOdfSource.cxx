#include "vtkOdfSource.h"

#include "vtkCellArray.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkPoints.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkObjectFactory.h"
#include "vtkDoubleArray.h"
#include "vtkCellData.h"
#include <vtkLookupTable.h>
#include <limits>
#include <vtkPointData.h>

vtkStandardNewMacro(vtkOdfSource);

vtkOdfSource::vtkOdfSource()
  : r(0)
  , g(0)
  , b(0)
  , UseCustomColor(false)
{
  Scale = 1;
  lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->SetRange(0,1);
  lut->Build();
  this->SetNumberOfInputPorts(0);
}

vtkOdfSource::~vtkOdfSource()
{

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
  vtkPolyData *output = vtkPolyData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));

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
    Odf = Odf.MaxNormalize();
    colorOdf = Odf;
  }

  vtkCellArray* polys = TemplateOdf->GetPolys();
  output->SetPolys(polys);
  polys->InitTraversal();
  vtkSmartPointer<vtkPoints> newPoints = vtkSmartPointer<vtkPoints>::New();
  int numPoints = TemplateOdf->GetPoints()->GetNumberOfPoints();
  newPoints->Allocate(numPoints);

  vtkSmartPointer<vtkUnsignedCharArray> point_colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
  point_colors->Allocate(output->GetNumberOfPoints() * 4);
  point_colors->SetNumberOfComponents(4);
  point_colors->SetName("ODF_COLORS");
  unsigned char rgba[4];
  double rgb[3];

  for(int j=0; j<numPoints; j++)
  {
    double p[3];
    TemplateOdf->GetPoints()->GetPoint(j,p);
    double val = Odf.GetElement(j);
    p[0] *= val*Scale*AdditionalScale*0.5;
    p[1] *= val*Scale*AdditionalScale*0.5;
    p[2] *= val*Scale*AdditionalScale*0.5;
    newPoints->InsertNextPoint(p);

    if (UseCustomColor)
    {
      rgba[0] = (unsigned char)r;
      rgba[1] = (unsigned char)g;
      rgba[2] = (unsigned char)b;
    }
    else
    {
      double color_val = colorOdf.GetElement(j);
      lut->GetColor(1-color_val, rgb);
      rgba[0] = (unsigned char)(255.0*rgb[0]);
      rgba[1] = (unsigned char)(255.0*rgb[1]);
      rgba[2] = (unsigned char)(255.0*rgb[2]);
    }
    rgba[3] = 255;
    point_colors->InsertTypedTuple(j, rgba);
  }
  output->SetPoints(newPoints);
  output->GetPointData()->AddArray(point_colors);
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

  outInfo->Set(vtkAlgorithm::CAN_HANDLE_PIECE_REQUEST(),0);

  return 1;
}
