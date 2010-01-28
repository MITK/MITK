/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef _MITK_CONTOUR_H_
#define _MITK_CONTOUR_H_

#include "mitkCommon.h"
#include "MitkExtExports.h"
#include "mitkBaseData.h"
#include <vtkRenderWindow.h>

#include <itkPolyLineParametricPath.h>

namespace mitk 
{

/**
* This class holds stores vertices for drawing a contour 
*
*/
class MitkExt_EXPORT Contour : public BaseData
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

  itkSetMacro(Selected, bool);

  itkGetMacro(Selected, bool);

  itkSetMacro(Width, float);

  itkGetMacro(Width, float);


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
  PathPointer GetContourPath() const;

  /**
  * set the current render window. This is helpful if one 
  * wants to draw the contour in one special window only.
  */
  void SetCurrentWindow(vtkRenderWindow* rw);

  /**
  * returns the points to the current render window
  */
  vtkRenderWindow* GetCurrentWindow() const;

  /**
  * returns the number of points stored in the contour
  */
  unsigned int GetNumberOfPoints() const;

  /**
  * returns the container of the contour points
  */
  PointsContainerPointer GetPoints() const;

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

  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:

  /**
  * parametric path of a contour;
  */
  PathType::Pointer m_ContourPath;

  /**
  * the current render window
  */ 
  vtkRenderWindow* m_CurrentWindow;

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

  bool m_Selected;

  float m_Width;
};

} // namespace mitk

#endif //_MITK_CONTOUR_H_
