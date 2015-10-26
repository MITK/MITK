'''
Created on Sep 9, 2015

@author: wirkert
'''


import logging

import numpy as np
import luigi

import scriptpaths as sp
import tasks_mc
import mc.factories as mcfac

# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2015_11_12_IPCAI_in_silico"
sp.RESULTS_FOLDER = "mc_data"

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    BATCH_NUMBERS = np.arange(0, 100, 1)
    for i in BATCH_NUMBERS:
        main_task = tasks_mc.CreateSpectraTask(
                                        "colon_muscle_tissue_test",
                                         i,
                                         1000,
                                         mcfac.ColonMuscleMcFactory())
        w.add(main_task)
        w.run()


