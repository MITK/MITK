#ifndef _MITK_CONTOUR_H_
#define _MITK_CONTOUR_H_

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "mitkRenderWindow.h"

#include <itkPolyLineParametricPath.h>

namespace mitk 
  {

  /**
   * This class holds stores vertices for drawing a contour 
   *
   */
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
      typedef BoundingBoxType::PointsContainerIterator              PointsContainerIterator;

      /**
       * sets whether the contour should be closed or open.
       * by default the contour is closed
       */
      itkSetMacro(Closed, bool);

      /**
       * returns if the contour is closed or opened
       */
      itkGetMacro(Closed, bool);

      /**
       * clean up the contour data
       */
      void Initialize();

      /** 
       * add a new vertex to the contour
       */
      void AddVertex(mitk::Point3D newPoint);

      /**
       * return an itk parametric path of the contour 
       */
      PathPointer GetContourPath();

      /**
       * set the current render window. This is helpful if one 
       * wants to draw the contour in one special window only.
       */
      void SetCurrentWindow(mitk::RenderWindow* rw);
 
      /**
       * returns the points to the current render window
       */
      mitk::RenderWindow* GetCurrentWindow();

      /**
       * returns the number of points stored in the contour
       */
      unsigned int GetNumberOfPoints();

      /**
       * returns the container of the contour points
       */
      PointsContainerPointer GetPoints();

      /**
       * set the contour points container.
       */
      void SetPoints(PointsContainerPointer points);

      /**
       * intherited from parent
       */
      virtual void UpdateOutputInformation();

      /**
       * intherited from parent
       */
      virtual void SetRequestedRegionToLargestPossibleRegion();

      /**
       * intherited from parent
       */
      virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

      /**
       * intherited from parent
       */
      virtual bool VerifyRequestedRegion();

      /**
       * intherited from parent
       */
      virtual void SetRequestedRegion(itk::DataObject *data);

    protected:
      Contour();
      virtual ~Contour();

    private:

      /**
       * parametric path of a contour;
       */
      PathType::Pointer m_ContourPath;

      /**
       * the current render window
       */ 
      mitk::RenderWindow* m_CurrentWindow;

      /**
       * the bounding box of the contour
       */
      BoundingBoxType::Pointer m_BoundingBox;

      /**
       * container for all contour points
       */
      BoundingBoxType::PointsContainer::Pointer m_Vertices;

      /**
       * decide whether th contour is open or closed 
       */
      bool m_Closed;
    };

  } // namespace mitk



#endif //_MITK_CONTOUR_H_
