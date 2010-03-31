/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-03-13 15:04:52 +0100 (Do, 13 Mrz 2008) $
Version:   $Revision: 9098 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef MITK_DiffusionImageHeaderInformation_H
#define MITK_DiffusionImageHeaderInformation_H

#include <itkObject.h>
#include "vnl/vnl_vector_fixed.h"
#include "mitkCommon.h"
#include "itkObjectFactory.h"

namespace mitk 
{

  enum DiffusionHeaderGroup{
    DHG_NotYetGrouped,
    DHG_ZeroDiffusionWeighted,
    DHG_NonZeroDiffusionWeighted,
    DHG_Other,
  };

  class DiffusionImageHeaderInformation : public itk::Object
  {

  public:

    mitkClassMacro( DiffusionImageHeaderInformation, itk::Object );
    itkNewMacro(Self);

    typedef std::vector< std::string > FileNamesContainer;

    //DiffusionImageHeaderInformation(int nRows, int nCols, float xRes, float yRes, float xOrigin,
    //  float yOrigin, float zOrigin, float sliceThickness, float sliceSpacing,
    //  int nSliceInVolume, float xRow, float yRow, float zRow, float xCol,
    //  float yCol, float zCol, float xSlice, float ySlice, float zSlice,
    //  float bValue, vnl_vector_fixed<double, 3>  DiffusionVector, 
    //  std::string vendor, bool mosaic, DiffusionHeaderGroup headerGroup = DHG_NotYetGrouped):
    //  nRows(nRows), nCols(nCols),xRes(xRes),
    //  yRes(yRes),xOrigin(xOrigin),yOrigin(yOrigin),zOrigin(zOrigin),
    //  sliceThickness(sliceThickness),sliceSpacing(sliceSpacing),
    //  nSliceInVolume(nSliceInVolume), xRow(xRow), yRow(yRow), zRow(zRow),
    //  xCol(xCol),yCol(yCol),zCol(zCol),xSlice(xSlice),ySlice(ySlice),zSlice(zSlice),
    //  bValue(bValue), DiffusionVector(DiffusionVector), vendor(vendor),mosaic(mosaic),
    //  headerGroup(headerGroup) {}

    std::string seriesDescription;
    int seriesNumber;
    std::string patientName;
    int nRows;
    int nCols;
    float xRes;
    float yRes;
    float xOrigin;
    float yOrigin;
    float zOrigin;
    float sliceThickness;
    float sliceSpacing;
    int nSliceInVolume;
    float xRow;
    float yRow;
    float zRow; 
    float xCol; 
    float yCol; 
    float zCol; 
    float xSlice; 
    float ySlice; 
    float zSlice; 
    float bValue;
    vnl_vector_fixed<double, 3> DiffusionVector;
    std::string vendor;
    bool mosaic;
    DiffusionHeaderGroup headerGroup;

    FileNamesContainer m_DicomFilenames;

    inline bool isIdentical(const DiffusionImageHeaderInformation::Pointer other) const {
      return nRows == other->nRows &&
        nCols == other->nCols &&
        xRes == other->xRes &&
        yRes == other->yRes &&
        xOrigin == other->xOrigin &&
        yOrigin == other->yOrigin &&
        zOrigin == other->zOrigin &&
        sliceThickness == other->sliceThickness &&
        sliceSpacing == other->sliceSpacing &&
        nSliceInVolume == other->nSliceInVolume &&
        xRow == other->xRow &&
        yRow == other->yRow &&
        zRow == other->zRow &&
        xCol == other->xCol &&
        yCol == other->yCol &&
        zCol == other->zCol &&
        xSlice == other->xSlice &&
        ySlice == other->ySlice &&
        zSlice == other->zSlice;
    }

  protected:

    DiffusionImageHeaderInformation();
    virtual ~DiffusionImageHeaderInformation();

  };

} // namespace

#endif // MITK_DiffusionImageHeaderInformation_H


