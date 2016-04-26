
import logging
import time
import os

from mc.sim import get_diffuse_reflectance


def create_spectrum(tissue_model, sim_wrapper, wavelengths):
        """
        Create a whole spectrum from one instance (dataframe_row) using our
        tissue model at wavelength wavelength.

        Args:
            tissue_model: the model which should be used to generate the
                spectrum
            sim_wrapper: the simulation which should be used to generate the
                reflectances
            wavelengths: the wavelengths which shall be simulated

        Returns: the simulated reflectances
        """
        start = time.time()
        # map the _wavelengths array to reflectance list

        def wavelength_to_reflectance(wavelength):
            # helper function to determine the reflectance for a given
            # wavelength using the current model and simulation
            tissue_model.set_wavelength(wavelength)
            tissue_model.create_mci_file()
            sim_wrapper.run_simulation()
            simulation_path = os.path.split(sim_wrapper.mcml_executable)[0]
            return get_diffuse_reflectance(os.path.join(simulation_path,
                                                        tissue_model.
                                                        get_mco_filename()))
        reflectances = map(wavelength_to_reflectance, wavelengths)
        end = time.time()
        # success!
        logging.info("successfully ran simulation in " +
                     "{:.2f}".format(end - start) + " seconds")
        return reflectances
