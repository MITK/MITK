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


#ifndef MITKLookupTable_H_HEADER_INCLUDED_C1EBD53D
#define MITKLookupTable_H_HEADER_INCLUDED_C1EBD53D

#include <MitkExports.h>
#include <mitkCommon.h>
#include "vtkLookupTable.h"
#include <itkDataObject.h>
#include <itkObjectFactory.h>

class vtkColorTransferFunction;
class vtkPiecewiseFunction;

namespace mitk
{

//##
//##Documentation
//## @brief LookupTable containing a vtkLookupTable
//## @ingroup Data
//##
class MITK_CORE_EXPORT LookupTable : public itk::DataObject
{
public:
    /** 
     *@brief Some convenient typedefs. 
     */
    typedef unsigned char RawLookupTableType;

    mitkClassMacro( LookupTable, itk::DataObject );

    itkNewMacro( Self );

    /**
     * @returns the associated vtkLookupTable
     */
    virtual vtkLookupTable* GetVtkLookupTable() const;

    virtual RawLookupTableType * GetRawLookupTable() const;

    virtual void SetVtkLookupTable( vtkLookupTable* lut );

    virtual void ChangeOpacityForAll( float opacity );

    virtual void ChangeOpacity(int index, float opacity );

    
    /*!
    * \brief equality operator implementation
    */
    virtual bool operator==( const mitk::LookupTable& LookupTable ) const;

    /*!
    * \brief non equality operator implementation
    */ 
    virtual bool operator!=( const LookupTable& LookupTable ) const;

    /*!
    * \brief implementation necessary because operator made
    * private in itk::Object
    */
    virtual LookupTable& operator=( const LookupTable& LookupTable );

    /**
     * Updates the output information of the current object by calling 
     * updateOutputInformation of the data objects source object.
     */
    virtual void UpdateOutputInformation( );

    /**
     * Sets the requested Region to the largest possible region.
     * This method is not implemented, since this is the default
     * behaviour of the itk pipeline and we do not support the
     * requested-region mechanism for lookup-tables
     */
    virtual void SetRequestedRegionToLargestPossibleRegion( );

    /**
     * Checks, if the requested region lies outside of the buffered region by
     * calling verifyRequestedRegion(). 
     */
    virtual bool RequestedRegionIsOutsideOfTheBufferedRegion( );

    /**
     * Checks if the requested region is completely contained in
     * the buffered region. Since we always want to process the lookup
     * table as a whole, this method always returns true
     */
    virtual bool VerifyRequestedRegion( );

    /**
     * This method has no effect for lookup tables, since we do
     * not support the region-mechanism
     */
    virtual void SetRequestedRegion( itk::DataObject *data );

    LookupTable();
    virtual ~LookupTable();

    void CreateColorTransferFunction(vtkColorTransferFunction*& colorFunction);
    void CreateOpacityTransferFunction(vtkPiecewiseFunction*& opacityFunction);
    void CreateGradientTransferFunction(vtkPiecewiseFunction*& gradientFunction);

protected:

    void PrintSelf(std::ostream &os, itk::Indent indent) const;

    vtkLookupTable* m_LookupTable;

private:

};

} // namespace mitk



#endif /* LookupTable_H_HEADER_INCLUDED_C1EBD53D */
