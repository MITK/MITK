# -*- coding: utf-8 -*-
"""
Created on Fri Aug  7 18:41:50 2015

@author: wirkert
"""

from setuptools import setup, find_packages

setup(name='MITK-MSI',
      version='0.1',
      description='Multi spectral imaging (MSI) utilities',
      author='Sebastian Wirkert',
      author_email='s.wirkert@dkfz-heidelberg.de',
      license='BSD',
      packages=find_packages(exclude=['test*']),
      package_dir={},
      package_data={'data': ['*.txt', '*.mci', '*.nrrd']},
      install_requires=['numpy>=1.10.2', 'scipy', 'scikit-learn>=0.17',
                        'SimpleITK>=0.9.0', 'subprocess32',
                        'pypng', 'pandas>0.17', 'libtiff'],
      entry_points={}  # for scripts, add later
      )
