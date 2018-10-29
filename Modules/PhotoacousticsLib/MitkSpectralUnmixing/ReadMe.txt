/**
	\spectral unmixing mini App (SUMA)

	\brief The SUMA is designed to enable batch processing for spectral unmixing. To use the SUMA one has to build the MitkSpectralUnmixingApp located in the PAlib. Afterwards a .bat script is available int the suberbuild/MITK-build/bin. This script takes 4 required and one optional parameter:

Required parameters
  -i, --inputFilename, input filename 		// "E:/mydata/awesome_exp/first_image.nrrd" image has to be an integer multiple of the number of wavelengths
  -o, --outputFileStruct, input save name 	// "E:/mydata/awesome_exp_unmixed/first_image_ctr" will be saved as "_HbO2_SU_.nrrd", "_Hb_SU_.nrrd" and "_sO2_.nrrd"; it is recommended to add an counter (ctr) to the savenames otherwise they will be overwritten.
  -l, --inputWavelengths, input wavelengths	// (int) 299 < wavelength < 1001 with format: int blank int blank int blank ... 
  -a, --inputAlg, input algorithm		// insert alg: "QR", "NNLS", "WLS", "SVD", "LU"
  -w, --inputWeights, input weights (optional) 	// int weights in % format and order corresponding to the wavelength: int blank int blank ... 

To costumize the SUMA for batch processing of a whole directory, you can either write a script which calls the SUMA for every image again oR insert a loop arround the actual unmixing in the code entering your file structure (see comments inside the code).

For further questions please contact Niklas Holzwarth (n.holzwarth@dkfz-heidelberg.de or niklas.holzwarth@gmail.com)