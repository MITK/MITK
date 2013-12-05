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
/*
 *  mitkPlanarFigureComposite.h
 *  mitk-all
 *
 *  Created by HAL9000 on 2/4/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _MITK_PLANARFIGURECOMPOSITE_H
#define _MITK_PLANARFIGURECOMPOSITE_H

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "FiberTrackingExports.h"
#include "mitkPlanarFigure.h"
#include "itkVectorContainer.h"
#include "mitkDataNode.h"


namespace mitk {

  enum PFCompositionOperation {
    PFCOMPOSITION_AND_OPERATION,
    PFCOMPOSITION_OR_OPERATION,
    PFCOMPOSITION_NOT_OPERATION,
  };




  class FiberTracking_EXPORT PlanarFigureComposite : public PlanarFigure
  {

    typedef itk::VectorContainer<unsigned int, PlanarFigure::Pointer> CompositionContainer;
    typedef itk::VectorContainer<unsigned int, mitk::DataNode::Pointer> DataNodeContainer;


  public:
    mitkClassMacro(PlanarFigureComposite, PlanarFigure);
    itkNewMacro( Self );

    // ///MUST HAVE IMPLEMENTATION//////
    bool SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist );
    unsigned int GetMinimumNumberOfControlPoints() const
    {
      return 0;
    }
    /** \brief Circle has 2 control points per definition. */
    unsigned int GetMaximumNumberOfControlPoints() const
    {
      return 0;
    }
    // /////////////////////////



    int getNumberOfChildren();
    mitk::PlanarFigure::Pointer getChildAt(int);
    void addPlanarFigure(PlanarFigure::Pointer);


    mitk::DataNode::Pointer getDataNodeAt(int);
    void addDataNode(mitk::DataNode::Pointer);
    void replaceDataNodeAt(int, mitk::DataNode::Pointer);

    // set if this compsition is AND, OR, NOT
    void setOperationType(PFCompositionOperation);
    PFCompositionOperation getOperationType();

    void setDisplayName(std::string);
    std::string getDisplayName();

  protected:
    PlanarFigureComposite();
    virtual ~PlanarFigureComposite();

    PlanarFigureComposite(const Self& other);

    mitkCloneMacro(Self);

    // ///MUST HAVE IMPLEMENTATION//////
    /** \brief Generates the poly-line representation of the planar figure. */
    virtual void GeneratePolyLine();

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

    /** \brief Calculates feature quantities of the planar figure. */
    virtual void EvaluateFeaturesInternal();

    virtual void PrintSelf( std::ostream &os, itk::Indent indent ) const;
    // ////////////////////




  private:
    //this vector takes planarfigures and planarfigureComosite types
    CompositionContainer::Pointer m_PFVector;
    PFCompositionOperation m_compOperation;

    DataNodeContainer::Pointer m_DNVector;
    std::string m_name;



  };

}

#endif
