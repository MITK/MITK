'''
Created on Sep 9, 2015

@author: wirkert
'''


import logging
import datetime

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
    logging.basicConfig(filename='in_silico' + str(datetime.datetime.now()) +
                         '.log', level=logging.INFO)
    ch = logging.StreamHandler()
    ch.setLevel(logging.INFO)
    logger = logging.getLogger()
    logger.addHandler(ch)
    luigi.interface.setup_interface_logging()

    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    BATCH_NUMBERS = np.arange(0, 100, 1)
    for i in BATCH_NUMBERS:
        main_task = tasks_mc.CreateSpectraTask("ipcai_less_generic", i, 1000,
                                               mcfac.LessGenericMcFactory())
        w.add(main_task)
        w.run()

