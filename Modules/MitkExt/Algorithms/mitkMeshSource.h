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


#ifndef _MITKMESHSOURCE_H
#define _MITKMESHSOURCE_H

#include "itkProcessObject.h"
#include "MitkExtExports.h"
#include "mitkMesh.h"

namespace mitk
{
/**
 * @brief Superclass of all classes generating point sets (instances of class
 * mitk::Mesh) as output. 
 *
 * In itk and vtk the generated result of a ProcessObject is only guaranteed
 * to be up-to-date, when Update() of the ProcessObject or the generated
 * DataObject is called immediately before access of the data stored in the
 * DataObject. 
 * @ingroup Process
 */
class MitkExt_EXPORT MeshSource : public itk::ProcessObject
{
public:
    mitkClassMacro( MeshSource, itk::ProcessObject );

    itkNewMacro( Self );

    typedef mitk::Mesh OutputType;

    typedef OutputType::Pointer OutputTypePointer;
    
    /**
     * Allocates a new output object and returns it. Currently the
     * index idx is not evaluated.
     * @param idx the index of the output for which an object should be created
     * @returns the new object
     */
    virtual itk::DataObject::Pointer MakeOutput ( unsigned int idx );

    /**
     * Allows to set the output of the point set source. 
     * @param output the intended output of the point set source
     */
    virtual void SetOutput( OutputType* output );
    
    /**
     * Allows to set the n-th output of the point set source. 
     * @param idx The index of the output 
     * @param output The intended output of the point set source
     */
    virtual void SetOutput( unsigned int idx, OutputType* output );
    
    /**
     * Returns the output with index 0 of the point set source
     * @returns the output
     */
    virtual OutputType* GetOutput();
    
    /**
     * Returns the n'th output of the point set source
     * @param idx the index of the wanted output
     * @returns the output with index idx.
     */
    virtual OutputType* GetOutput ( unsigned int idx );
    
protected:

    MeshSource();
    
    virtual ~MeshSource();

};

}
#endif // #define _MITKMESHSOURCE_H
