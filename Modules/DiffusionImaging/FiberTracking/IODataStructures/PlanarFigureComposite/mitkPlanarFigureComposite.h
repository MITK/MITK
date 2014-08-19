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
#include <MitkFiberTrackingExports.h>
#include "mitkPlanarFigure.h"
#include "itkVectorContainer.h"
#include "mitkDataNode.h"


namespace mitk {

enum PFCompositionOperation {
    PFCOMPOSITION_AND_OPERATION,
    PFCOMPOSITION_OR_OPERATION,
    PFCOMPOSITION_NOT_OPERATION,
};




class MitkFiberTracking_EXPORT PlanarFigureComposite : public BaseData
{

    typedef itk::VectorContainer<unsigned int, BaseData::Pointer> CompositionContainer;
    typedef itk::VectorContainer<unsigned int, mitk::DataNode::Pointer> DataNodeContainer;


public:
    mitkClassMacro(PlanarFigureComposite, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual void UpdateOutputInformation();
    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion(const itk::DataObject*);

    // ///MUST HAVE IMPLEMENTATION//////
    bool SetControlPoint(unsigned int, const Point2D &, bool);
    unsigned int GetMinimumNumberOfControlPoints() const
    {
        return 0;
    }
    unsigned int GetMaximumNumberOfControlPoints() const
    {
        return 0;
    }
    // /////////////////////////

    int getNumberOfChildren();
    mitk::BaseData::Pointer getChildAt(int);
    void addPlanarFigure(BaseData::Pointer);


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

    // ///MUST HAVE IMPLEMENTATION//////
    /** \brief Generates the poly-line representation of the planar figure. */
    virtual void GeneratePolyLine();

    /** \brief Generates the poly-lines that should be drawn the same size regardless of zoom.*/
    virtual void GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight);

    /** \brief Calculates feature quantities of the planar figure. */
    virtual void EvaluateFeaturesInternal();

    virtual void PrintSelf(std::ostream &, itk::Indent) const;
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
