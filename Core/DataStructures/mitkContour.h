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
  typedef BoundingBoxType::PointsContainer                      PointsContainer;
  typedef BoundingBoxType::PointsContainer::Pointer             PointsContainerPointer;

 	void ExecuteOperation(Operation* operation);

  void AddVertex(mitk::ITKPoint3D newPoint);

  void Initialize();

  void Continue(mitk::ITKPoint3D newPoint);

  PathType::Pointer GetContourPath();

  void SetCurrentWindow(mitk::RenderWindow* rw);
  mitk::RenderWindow* GetCurrentWindow();

  unsigned int GetNumberOfPoints();

  PointsContainerPointer GetPoints();
  void SetPoints(PointsContainerPointer points);

	//virtual methods, that need to be implemented
	virtual void UpdateOutputInformation();
	virtual void SetRequestedRegionToLargestPossibleRegion();
	virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
	virtual bool VerifyRequestedRegion();
	virtual void SetRequestedRegion(itk::DataObject *data);

protected:
	Contour();

	virtual ~Contour();

private:

  PathType::Pointer m_ContourPath;

  mitk::RenderWindow* m_CurrentWindow;

  BoundingBoxType::Pointer m_BoundingBox;
  BoundingBoxType::PointsContainer::Pointer m_Vertices;

};

} // namespace mitk



#endif //_MITK_CONTOUR_H_
