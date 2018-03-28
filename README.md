# Timing DAQ repository

The main aim of this code is to convert raw binary data recorded from SiPMs from different devices and analyze them in a coherent and standardised way.
The main executable applications will be created by the istallation in the main folder. One for each available device is present under the form: \texttt{<device_name>Dat2Root}.

## Quick start
If you are not interested in the details just use the following commands

### Installation

git clone https://github.com/CaltechPrecisionTiming/TimingDAQ
make -j8

### Examples

./DRSDat2Root --input_file=<your_file>.dat --config=config/DRS_Na22.config --N_evts=10000

## Command line option
All the command line option must be passed using the sintax \texttt{--<opt>=<val>}.
The following option are common to all the devices.

### VME additional options:

example

### DRS additional options:

TODO: instructions

## Configuration file
