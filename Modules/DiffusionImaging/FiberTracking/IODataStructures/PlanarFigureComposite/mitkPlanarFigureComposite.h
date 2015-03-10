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

#ifndef _MITK_PLANARFIGURECOMPOSITE_H
#define _MITK_PLANARFIGURECOMPOSITE_H

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include <MitkFiberTrackingExports.h>
#include "mitkPlanarFigure.h"
#include "itkVectorContainer.h"
#include "mitkDataNode.h"


namespace mitk {

class MITKFIBERTRACKING_EXPORT PlanarFigureComposite : public BaseData
{

public:

    enum OperationType {
        AND,
        OR,
        NOT
    };

    mitkClassMacro(PlanarFigureComposite, BaseData)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual void SetRequestedRegionToLargestPossibleRegion();
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();
    virtual bool VerifyRequestedRegion();
    virtual void SetRequestedRegion(const itk::DataObject*);

    // set if this compsition is AND, OR, NOT
    void setOperationType(OperationType);
    OperationType getOperationType() const;

protected:
    PlanarFigureComposite();
    virtual ~PlanarFigureComposite();

    PlanarFigureComposite(const Self& other);

private:
    OperationType m_compOperation;
};
}

#endif
