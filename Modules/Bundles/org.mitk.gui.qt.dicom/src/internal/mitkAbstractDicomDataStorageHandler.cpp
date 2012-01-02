/*=========================================================================
Program: Medical Imaging & Interaction Toolkit
Module: $RCSfile$
Language: C++
Date: $Date$
Version: $Revision$
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/


#include "mitkAbstractDicomDataStorageHandler.h"

namespace mitk {

    AbstractDicomDataStorageHandler::AbstractDicomDataStorageHandler()
    {
    }

    AbstractDicomDataStorageHandler::~AbstractDicomDataStorageHandler()
    {
    }


    void AbstractDicomDataStorageHandler::SuspendModel()
    {
        //m_DICOMModel.setDatabase(m_EmptyDatabase);
    }

    void AbstractDicomDataStorageHandler::ResumeModel()
    {
        //m_DICOMModel.setDatabase(m_DICOMDatabase->database());
    }

    void AbstractDicomDataStorageHandler::ResetModel()
    {
        m_DICOMModel.reset();
    }

    void AbstractDicomDataStorageHandler::SetDatabase(std::string databaseDirectory)
    {
    }

    const QSharedPointer<ctkDICOMDatabase> AbstractDicomDataStorageHandler::GetDatabase()
    {
    }

    const QSharedPointer<ctkDICOMIndexer> AbstractDicomDataStorageHandler::GetIndexer()
    {
    }

    const QSharedPointer<ctkDICOMModel> AbstractDicomDataStorageHandler::GetModel()
    {
    }

}