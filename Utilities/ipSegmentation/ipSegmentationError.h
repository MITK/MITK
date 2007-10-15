/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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
#ifndef IPSEGMENTATION_ERROR_H
#define IPSEGMENTATION_ERROR_H

enum {
  ipMITKSegmentationOK = 0     /*! No errors at all. */,
  ipMITKSegmentationOUT_OF_MEMORY  /*! Not enough memory available. */,    
  ipMITKSegmentationWRONG_TYPE  /*! Segmentation image has a wrong type. */,
  ipMITKSegmentationUNDO_DISABLED  /*! Undo cannot be performed. */,
  ipMITKSegmentationPIC_NULL    /*! PIC descriptor is null. */,
  ipMITKSegmentationUNKNOWN_ERROR  /*! Some unknown error occurred. */
};

/*!
\brief Prints an error message and exits 
if an error has occurred. 
@param error error code
*/
void ipMITKSegmentationError (int error);

/*!
\brief Checks the segmentation image.
@param segmentation the segmentation image.
*/
void ipMITKSegmentationCheck (ipPicDescriptor* segmentation);

#endif
