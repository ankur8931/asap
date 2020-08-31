import sys
from setuptools import setup

assert sys.version_info.major == 3 and sys.version_info.minor >= 6, \
    "The Autonomous Security Analysis and Penetration Testing repo is designed to work with Python 3.6 " \
    + " and greater. Please install it before proceeding."

setup(name='nasim',
      version='0.0.5',
      install_requires=[
          'gym',
          'numpy',
          'networkx',
          'matplotlib',
          'pyyaml',
          'prettytable',
          'pytest'
      ],
      description="A simple and fast tool for pententesting",
      author="SNAC")
