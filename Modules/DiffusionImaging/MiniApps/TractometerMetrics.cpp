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

#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <mitkImageCast.h>
#include <mitkImageToItk.h>
#include <itkEvaluateDirectionImagesFilter.h>
#include <metaCommand.h>
#include "mitkCommandLineParser.h"
#include <itkTractsToVectorImageFilter.h>
#include <usAny.h>
#include <itkImageFileWriter.h>
#include <mitkIOUtil.h>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>
#include <itksys/SystemTools.hxx>
#include <mitkCoreObjectFactory.h>

#define _USE_MATH_DEFINES
#include <math.h>

int main(int argc, char* argv[])
{
    mitkCommandLineParser parser;

    parser.setTitle("Tractometer Metrics");
    parser.setCategory("Fiber Tracking and Processing Methods");
    parser.setDescription("");
    parser.setContributor("MBI");

    parser.setArgumentPrefix("--", "-");
    parser.addArgument("input", "i", mitkCommandLineParser::InputFile, "Input:", "input tractogram (.fib, vtk ascii file format)", us::Any(), false);
    parser.addArgument("out", "o", mitkCommandLineParser::OutputDirectory, "Output:", "output root", us::Any(), false);
    parser.addArgument("labels", "l", mitkCommandLineParser::StringList, "Label pairs:", "label pairs", false);
    parser.addArgument("labelimage", "li", mitkCommandLineParser::String, "Label image:", "label image", false);
    parser.addArgument("verbose", "v", mitkCommandLineParser::Bool, "Verbose:", "output valid, invalid and no connections as fiber bundles");

    parser.addArgument("fileID", "id", mitkCommandLineParser::String, "ID:", "optional ID field");

    map<string, us::Any> parsedArgs = parser.parseArguments(argc, argv);
    if (parsedArgs.size()==0)
        return EXIT_FAILURE;

    mitkCommandLineParser::StringContainerType labelpairs = us::any_cast<mitkCommandLineParser::StringContainerType>(parsedArgs["labels"]);

    string fibFile = us::any_cast<string>(parsedArgs["input"]);
    string labelImageFile = us::any_cast<string>(parsedArgs["labelimage"]);

    string outRoot = us::any_cast<string>(parsedArgs["out"]);

    string fileID = "";
    if (parsedArgs.count("fileID"))
        fileID = us::any_cast<string>(parsedArgs["fileID"]);

    bool verbose = false;
    if (parsedArgs.count("verbose"))
        verbose = us::any_cast<bool>(parsedArgs["verbose"]);

    try
    {
        typedef itk::Image<short, 3>    ItkShortImgType;
        typedef itk::Image<unsigned char, 3>    ItkUcharImgType;

        // load fiber bundle
        mitk::FiberBundleX::Pointer inputTractogram = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(fibFile)->GetData());

        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(labelImageFile)->GetData());
        typedef mitk::ImageToItk< ItkShortImgType > CasterType;
        CasterType::Pointer caster = CasterType::New();
        caster->SetInput(img);
        caster->Update();
        ItkShortImgType::Pointer labelImage = caster->GetOutput();

        string path = itksys::SystemTools::GetFilenamePath(labelImageFile);

        std::vector< bool > detected;
        std::vector< std::pair< int, int > > labelsvector;
        std::vector< ItkUcharImgType::Pointer > bundleMasks;
        std::vector< ItkUcharImgType::Pointer > bundleMasksCoverage;
        short max = 0;
        for (unsigned int i=0; i<labelpairs.size()-1; i+=2)
        {
            std::pair< short, short > l;
            l.first = boost::lexical_cast<short>(labelpairs.at(i));
            l.second = boost::lexical_cast<short>(labelpairs.at(i+1));
            std::cout << labelpairs.at(i);
            std::cout << labelpairs.at(i+1);
            if (l.first>max)
                max=l.first;
            if (l.second>max)
                max=l.second;

            labelsvector.push_back(l);
            detected.push_back(false);

            {
                mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(path+"/Bundle"+boost::lexical_cast<string>(labelsvector.size())+"_MASK.nrrd")->GetData());
                typedef mitk::ImageToItk< ItkUcharImgType > CasterType;
                CasterType::Pointer caster = CasterType::New();
                caster->SetInput(img);
                caster->Update();
                ItkUcharImgType::Pointer bundle = caster->GetOutput();
                bundleMasks.push_back(bundle);
            }

            {
                mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(path+"/Bundle"+boost::lexical_cast<string>(labelsvector.size())+"_MASK_COVERAGE.nrrd")->GetData());
                typedef mitk::ImageToItk< ItkUcharImgType > CasterType;
                CasterType::Pointer caster = CasterType::New();
                caster->SetInput(img);
                caster->Update();
                ItkUcharImgType::Pointer bundle = caster->GetOutput();
                bundleMasksCoverage.push_back(bundle);
            }
        }
        vnl_matrix< unsigned char > matrix; matrix.set_size(max, max); matrix.fill(0);

        vtkSmartPointer<vtkPolyData> polyData = inputTractogram->GetFiberPolyData();

        int validConnections = 0;
        int noConnection = 0;
        int validBundles = 0;
        int invalidBundles = 0;
        int invalidConnections = 0;

        ItkUcharImgType::Pointer coverage = ItkUcharImgType::New();
        coverage->SetSpacing(labelImage->GetSpacing());
        coverage->SetOrigin(labelImage->GetOrigin());
        coverage->SetDirection(labelImage->GetDirection());
        coverage->SetLargestPossibleRegion(labelImage->GetLargestPossibleRegion());
        coverage->SetBufferedRegion( labelImage->GetLargestPossibleRegion() );
        coverage->SetRequestedRegion( labelImage->GetLargestPossibleRegion() );
        coverage->Allocate();
        coverage->FillBuffer(0);

        vtkSmartPointer<vtkPoints> noConnPoints = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> noConnCells = vtkSmartPointer<vtkCellArray>::New();

        vtkSmartPointer<vtkPoints> invalidPoints = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> invalidCells = vtkSmartPointer<vtkCellArray>::New();

        vtkSmartPointer<vtkPoints> validPoints = vtkSmartPointer<vtkPoints>::New();
        vtkSmartPointer<vtkCellArray> validCells = vtkSmartPointer<vtkCellArray>::New();

        boost::progress_display disp(inputTractogram->GetNumFibers());
        for (int i=0; i<inputTractogram->GetNumFibers(); i++)
        {
            ++disp;

            vtkCell* cell = polyData->GetCell(i);
            int numPoints = cell->GetNumberOfPoints();
            vtkPoints* points = cell->GetPoints();


            if (numPoints>1)
            {
                double* start = points->GetPoint(0);
                itk::Point<float, 3> itkStart;
                itkStart[0] = start[0]; itkStart[1] = start[1]; itkStart[2] = start[2];
                itk::Index<3> idxStart;
                labelImage->TransformPhysicalPointToIndex(itkStart, idxStart);

                double* end = points->GetPoint(numPoints-1);
                itk::Point<float, 3> itkEnd;
                itkEnd[0] = end[0]; itkEnd[1] = end[1]; itkEnd[2] = end[2];
                itk::Index<3> idxEnd;
                labelImage->TransformPhysicalPointToIndex(itkEnd, idxEnd);


                if ( labelImage->GetPixel(idxStart)==0 || labelImage->GetPixel(idxEnd)==0 )
                {
                    noConnection++;

                    if (verbose)
                    {
                        vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
                        for (int j=0; j<numPoints; j++)
                        {
                            double* p = points->GetPoint(j);
                            vtkIdType id = noConnPoints->InsertNextPoint(p);
                            container->GetPointIds()->InsertNextId(id);
                        }
                        noConnCells->InsertNextCell(container);
                    }
                }
                else
                {
                    bool invalid = true;
                    for (unsigned int i=0; i<labelsvector.size(); i++)
                    {
                        bool outside = false;
                        ItkUcharImgType::Pointer bundle = bundleMasks.at(i);
                        std::pair< short, short > l = labelsvector.at(i);
                        if ( (labelImage->GetPixel(idxStart)==l.first && labelImage->GetPixel(idxEnd)==l.second) ||
                             (labelImage->GetPixel(idxStart)==l.second && labelImage->GetPixel(idxEnd)==l.first) )
                        {
                            for (int j=0; j<numPoints; j++)
                            {
                                double* p = points->GetPoint(j);

                                itk::Point<float, 3> itkP;
                                itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
                                itk::Index<3> idx;
                                bundle->TransformPhysicalPointToIndex(itkP, idx);

                                if ( !bundle->GetPixel(idx)>0 && bundle->GetLargestPossibleRegion().IsInside(idx) )
                                {
                                    outside=true;
                                }
                            }

                            if (!outside)
                            {
                                validConnections++;
                                if (detected.at(i)==false)
                                    validBundles++;
                                detected.at(i) = true;
                                invalid = false;


                                vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
                                for (int j=0; j<numPoints; j++)
                                {
                                    double* p = points->GetPoint(j);
                                    vtkIdType id = validPoints->InsertNextPoint(p);
                                    container->GetPointIds()->InsertNextId(id);

                                    itk::Point<float, 3> itkP;
                                    itkP[0] = p[0]; itkP[1] = p[1]; itkP[2] = p[2];
                                    itk::Index<3> idx;
                                    coverage->TransformPhysicalPointToIndex(itkP, idx);
                                    if ( coverage->GetLargestPossibleRegion().IsInside(idx) )
                                        coverage->SetPixel(idx, 1);
                                }
                                validCells->InsertNextCell(container);
                            }
                            break;
                        }
                    }
                    if (invalid==true)
                    {
                        invalidConnections++;
                        int x = labelImage->GetPixel(idxStart)-1;
                        int y = labelImage->GetPixel(idxEnd)-1;
                        if (x>=0 && y>0 && x<matrix.cols() && y<matrix.rows() && (matrix[x][y]==0 || matrix[y][x]==0) )
                        {
                            invalidBundles++;
                            matrix[x][y]=1;
                            matrix[y][x]=1;
                        }

                        if (verbose)
                        {
                            vtkSmartPointer<vtkPolyLine> container = vtkSmartPointer<vtkPolyLine>::New();
                            for (int j=0; j<numPoints; j++)
                            {
                                double* p = points->GetPoint(j);
                                vtkIdType id = invalidPoints->InsertNextPoint(p);
                                container->GetPointIds()->InsertNextId(id);
                            }
                            invalidCells->InsertNextCell(container);
                        }
                    }
                }
            }
        }

        if (verbose)
        {
            mitk::CoreObjectFactory::FileWriterList fileWriters = mitk::CoreObjectFactory::GetInstance()->GetFileWriters();
            vtkSmartPointer<vtkPolyData> noConnPolyData = vtkSmartPointer<vtkPolyData>::New();
            noConnPolyData->SetPoints(noConnPoints);
            noConnPolyData->SetLines(noConnCells);
            mitk::FiberBundleX::Pointer noConnFib = mitk::FiberBundleX::New(noConnPolyData);

            string ncfilename = outRoot;
            ncfilename.append("_NC.fib");

            mitk::IOUtil::SaveBaseData(noConnFib.GetPointer(), ncfilename );

            vtkSmartPointer<vtkPolyData> invalidPolyData = vtkSmartPointer<vtkPolyData>::New();
            invalidPolyData->SetPoints(invalidPoints);
            invalidPolyData->SetLines(invalidCells);
            mitk::FiberBundleX::Pointer invalidFib = mitk::FiberBundleX::New(invalidPolyData);

            string icfilename = outRoot;
            icfilename.append("_IC.fib");

            mitk::IOUtil::SaveBaseData(invalidFib.GetPointer(), icfilename );

            vtkSmartPointer<vtkPolyData> validPolyData = vtkSmartPointer<vtkPolyData>::New();
            validPolyData->SetPoints(validPoints);
            validPolyData->SetLines(validCells);
            mitk::FiberBundleX::Pointer validFib = mitk::FiberBundleX::New(validPolyData);

            string vcfilename = outRoot;
            vcfilename.append("_VC.fib");

            mitk::IOUtil::SaveBaseData(validFib.GetPointer(), vcfilename );

            {
                typedef itk::ImageFileWriter< ItkUcharImgType > WriterType;
                WriterType::Pointer writer = WriterType::New();
                writer->SetFileName(outRoot+"_ABC.nrrd");
                writer->SetInput(coverage);
                writer->Update();
            }
        }

        // calculate coverage
        int wmVoxels = 0;
        int coveredVoxels = 0;
        itk::ImageRegionIterator<ItkUcharImgType> it (coverage, coverage->GetLargestPossibleRegion());
        while(!it.IsAtEnd())
        {
            bool wm = false;
            for (unsigned int i=0; i<bundleMasksCoverage.size(); i++)
            {
                ItkUcharImgType::Pointer bundle = bundleMasksCoverage.at(i);
                if (bundle->GetPixel(it.GetIndex())>0)
                {
                    wm = true;
                    wmVoxels++;
                    break;
                }
            }
            if (wm && it.Get()>0)
                coveredVoxels++;
            ++it;
        }

        int numFibers = inputTractogram->GetNumFibers();
        double nc = (double)noConnection/numFibers;
        double vc = (double)validConnections/numFibers;
        double ic = (double)invalidConnections/numFibers;
        if (numFibers==0)
        {
            nc = 0.0;
            vc = 0.0;
            ic = 0.0;
        }
        int vb = validBundles;
        int ib = invalidBundles;
        double abc = (double)coveredVoxels/wmVoxels;

        std::cout << "NC: " << nc;
        std::cout << "VC: " << vc;
        std::cout << "IC: " << ic;
        std::cout << "VB: " << vb;
        std::cout << "IB: " << ib;
        std::cout << "ABC: " << abc;

        string logFile = outRoot;
        logFile.append("_TRACTOMETER.csv");
        ofstream file;
        file.open (logFile.c_str());
        {
            string sens = itksys::SystemTools::GetFilenameWithoutLastExtension(fibFile);
            if (!fileID.empty())
                sens = fileID;
            sens.append(",");

            sens.append(boost::lexical_cast<string>(nc));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(vc));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(ic));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(validBundles));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(invalidBundles));
            sens.append(",");

            sens.append(boost::lexical_cast<string>(abc));
            sens.append(";\n");
            file << sens;
        }
        file.close();
    }
    catch (itk::ExceptionObject e)
    {
        std::cout << e;
        return EXIT_FAILURE;
    }
    catch (std::exception e)
    {
        std::cout << e.what();
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "ERROR!?!";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
