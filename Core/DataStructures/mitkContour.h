#ifndef _MITK_CONTOUR_H_
#define _MITK_CONTOUR_H_

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "mitkRenderWindow.h"

#include <itkPolyLineParametricPath.h>

namespace mitk {

class Contour : public BaseData
{
public:
  mitkClassMacro(Contour, BaseData);

  itkNewMacro(Self);

  typedef itk::PolyLineParametricPath<3>                        PathType;
  typedef PathType::Pointer                                     PathPointer;
  typedef PathType::ContinuousIndexType                         ContinuousIndexType;
  typedef PathType::InputType                                   InputType;
  typedef PathType::OutputType                                  OutputType;
  typedef PathType::OffsetType                                  OffsetType;
  typedef itk::BoundingBox<unsigned long, 3, ScalarType>        BoundingBoxType;


  //##ModelId=3F0177E901BF
	void ExecuteOperation(Operation* operation);


  void AddVertex(mitk::ITKPoint3D newPoint);

  PathType::Pointer GetContourPath();


  void SetCurrentWindow(mitk::RenderWindow* rw);

	//virtual methods, that need to be implemented
  //##ModelId=3F0177E901EE
	virtual void UpdateOutputInformation();
  //##ModelId=3F0177E901FB
	virtual void SetRequestedRegionToLargestPossibleRegion();
  //##ModelId=3F0177E901FD
	virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
  //##ModelId=3F0177E901FF
	virtual bool VerifyRequestedRegion();
  //##ModelId=3F0177E9020B
	virtual void SetRequestedRegion(itk::DataObject *data);

protected:
  //##ModelId=3F0177E901BD
	Contour();

  //##ModelId=3F0177E901BE
	virtual ~Contour();

private:

  PathType::Pointer m_ContourPath;

  mitk::RenderWindow* m_CurrentWindow;

  BoundingBoxType::Pointer m_BoundingBox;
  BoundingBoxType::PointsContainer::Pointer m_Vertices;

};

} // namespace mitk



#endif //_MITK_CONTOUR_H_
