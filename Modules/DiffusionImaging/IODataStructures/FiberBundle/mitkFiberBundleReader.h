/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkFiberBundleReader_h
#define __mitkFiberBundleReader_h

#include "mitkCommon.h"
#include "itkVectorContainer.h"
#include "mitkFileReader.h"
#include "mitkFiberBundle.h"
#include "itkSlowPolyLineParametricPath.h"

//NOTE>umbenennen in FiberBundleSource

namespace mitk
{

  /** \brief 
  */

  class FiberBundleReader : public FileReader, public BaseProcess
  {
  public:

    /** Types for the standardized TractContainer **/
    /* direct linked includes of mitkFiberBundle DataStructure */
    typedef mitk::FiberBundle::ContainerPointType   ContainerPointType; 
    typedef mitk::FiberBundle::ContainerTractType   ContainerTractType; 
    typedef mitk::FiberBundle::ContainerType        ContainerType;
    
    
    typedef mitk::FiberBundle OutputType;
 
    mitkClassMacro( FiberBundleReader, BaseProcess );
    itkNewMacro(Self);

    const char* GetFileName() const;
    void SetFileName(const char* aFileName);
    const char* GetFilePrefix() const;
    void SetFilePrefix(const char* aFilePrefix);
    const char* GetFilePattern() const;
    void SetFilePattern(const char* aFilePattern);

    static bool CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern);

//    itkGetMacro(GroupFiberBundle, FiberGroupType::Pointer);
//    itkGetMacro(TractContainer, ContainerType::Pointer);

    virtual void Update();


    static const char* XML_GEOMETRY;

    static const char* XML_MATRIX_XX;

    static const char* XML_MATRIX_XY;

    static const char* XML_MATRIX_XZ;

    static const char* XML_MATRIX_YX;

    static const char* XML_MATRIX_YY;

    static const char* XML_MATRIX_YZ;

    static const char* XML_MATRIX_ZX;

    static const char* XML_MATRIX_ZY;

    static const char* XML_MATRIX_ZZ;

    static const char* XML_ORIGIN_X;

    static const char* XML_ORIGIN_Y;

    static const char* XML_ORIGIN_Z;

    static const char* XML_SPACING_X;

    static const char* XML_SPACING_Y;

    static const char* XML_SPACING_Z;

    static const char* XML_SIZE_X;

    static const char* XML_SIZE_Y;

    static const char* XML_SIZE_Z;

    static const char* XML_FIBER_BUNDLE;

    static const char* XML_FIBER;

    static const char* XML_PARTICLE;

    static const char* XML_ID;

    static const char* XML_POS_X;

    static const char* XML_POS_Y;

    static const char* XML_POS_Z;

    static const char* VERSION_STRING;

    static const char* XML_FIBER_BUNDLE_FILE;

    static const char* XML_FILE_VERSION;

    static const char* XML_NUM_FIBERS;

    static const char* XML_NUM_PARTICLES;

  protected:

    /** Does the real work. */
    virtual void GenerateData();
    virtual void GenerateOutputInformation();

    OutputType::Pointer m_OutputCache;

    std::string m_FileName;
    std::string m_FilePrefix;
    std::string m_FilePattern;
    ContainerType::Pointer m_TractContainer;

  private:
    void operator=(const Self&); //purposely not implemented
  };

} //namespace MITK

#endif // __mitkFiberBundleReader_h
