#ifndef __vtkOdfSource_h
#define __vtkOdfSource_h

#include "MitkDiffusionImagingExports.h"

#include "vtkPolyDataAlgorithm.h"
#include "mitkCommon.h"

class MitkDiffusionImaging_EXPORT vtkOdfSource : public vtkPolyDataAlgorithm 
{
public:
  vtkTypeRevisionMacro(vtkOdfSource,vtkPolyDataAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct sphere with radius=0.5 and default resolution 8 in both Phi
  // and Theta directions. Theta ranges from (0,360) and phi (0,180) degrees.
  static vtkOdfSource *New();

  vtkSetMacro(TemplateOdf,vtkPolyData*);
  vtkGetMacro(TemplateOdf,vtkPolyData*);

  vtkSetMacro(OdfVals,vtkDoubleArray*);
  vtkGetMacro(OdfVals,vtkDoubleArray*);
  
  vtkSetMacro(Scale,double);
  vtkGetMacro(Scale,double);

protected:
  vtkOdfSource();
  ~vtkOdfSource() {}

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);
  int RequestInformation(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  vtkPolyData* TemplateOdf;
  vtkDoubleArray* OdfVals;

  double Scale;

private:
  vtkOdfSource(const vtkOdfSource&);  // Not implemented.
  void operator=(const vtkOdfSource&);  // Not implemented.
};

#endif //__vtkOdfSource_h
