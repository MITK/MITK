'''
Created on Sep 9, 2015

@author: wirkert
'''


import logging

import luigi

import scriptpaths as sp
import tasks_mc as mc

# general output path config
sp.ROOT_FOLDER = \
        "/media/wirkert/data/Data/2015_06_01_Filtertransmittance_Spectrometer"


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    luigi.interface.setup_interface_logging()
    sch = luigi.scheduler.CentralPlannerScheduler()
    w = luigi.worker.Worker(scheduler=sch)
    NR_BATCHES_TO_CREATE = 3
    for i in range(NR_BATCHES_TO_CREATE):
        main_task = mc.CreateSpectraTask("batch", i)
        w.add(main_task)
        w.run()


