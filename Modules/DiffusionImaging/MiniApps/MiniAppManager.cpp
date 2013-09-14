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

#include "MiniAppManager.h"
#include <itkMultiThreader.h>

MiniAppManager* MiniAppManager::GetInstance()
{
    static MiniAppManager instance;
    return &instance;
}

// Attention: Name of the miniApp must be the last argument!!!
// it will be cut off from the rest of the arguments and then
// the app will be run
int MiniAppManager::RunMiniApp(int argc, char* argv[])
{
    itk::MultiThreader::SetGlobalDefaultNumberOfThreads(itk::MultiThreader::GetGlobalMaximumNumberOfThreads());
    try
    {
        std::string nameOfMiniApp;
        std::map< std::string, MiniAppFunction >::iterator it = m_Functions.begin();

        if( argc < 2)
        {
            std::cout << "Please choose the mini app to execute: " << std::endl;

            for(int i=0; it != m_Functions.end(); ++i,++it)
            {
                std::cout << "(" << i << ")" << " " << it->first << std::endl;
            }
            std::cout << "Please select: ";
            int choose;
            std::cin >> choose;

            it = m_Functions.begin();
            std::advance(it, choose);
            if( it != m_Functions.end() )
                nameOfMiniApp = it->first;
        }
        else
        {
            nameOfMiniApp = argv[1];
            //--argc;
        }

        it = m_Functions.find(nameOfMiniApp);
        if(it == m_Functions.end())
        {
            std::ostringstream s; s << "MiniApp (" << nameOfMiniApp << ") not found!";
            throw std::invalid_argument(s.str().c_str());
        }

        MITK_INFO << "Start " << nameOfMiniApp << " ..";
        MiniAppFunction func = it->second;
        return func( argc, argv );
    }

    catch(std::exception& e)
    {
        MITK_ERROR << e.what();
    }

    catch(...)
    {
        MITK_ERROR << "Unknown error occurred";
    }

    return EXIT_FAILURE;
}

/////////////////////
// MiniAppFunction //
/////////////////////
MiniAppManager::MiniAppFunction
MiniAppManager::AddFunction(const std::string& name, MiniAppFunction func)
{
    m_Functions.insert( std::pair<std::string, MiniAppFunction>(name, func) );
    return func;
}
