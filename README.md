# Timing DAQ repository

The main aim of this code is to convert raw binary data recorded from SiPMs from different devices and analyze them in a coherent and standardised way.
The main executable applications will be created by the istallation in the main folder. One for each available device is present under the form: ``<device_name>Dat2Root``.

## Quick start
If you are not interested in the details just use the following commands

### Installation
To install copy paste the following::
git clone https://github.com/CaltechPrecisionTiming/TimingDAQ
make -j8


### Examples

``./DRSDat2Root --input_file=<your_file>.dat --config=config/DRS_Na22.config --N_evts=10000``

## Command line options
All the command line option must be passed using the sintax ``--<opt>=<val>``. For all options ``--<opt>`` is equal to ``--<opt>=true``.
The following option are common to all the devices.

Compulsory arguments don't have default value

| Argument | Default | Notes|
| -------- | :-------: | ---- |
|``input_file``|    |Binary input file with the data. Must end in ``.dat``.|
|``config`` |      |Configuration file (see below).|
|``output_file``| ``<input>.root`` | Output root file containing a singe tree called ``pulse``. If not provided, the default option is to have the same name of the input file with the ``.root`` extension.|
|``N_evts`` | 0 | Event number at which stop reading the  input file. If 0, continue until ``eof`` is reached.|
|``start_evt`` | 0 | Event at which start analysing and writing in the TTree.|
|``save_raw`` | false | Boolean value indicating whether to save or not the raw ADC counts.|
|``save_meas``| false | Boolean value indicating whether to save or not the full time and voltage vector for each event.|
|``draw_debug_pulses`` | false | If true all the pulses are saved as ``.png``. Alternatively, ``--draw_debug_pulses=.<ext>`` will save the TCanvas in the given extension.|

### VME additional options:
| Argument | Default | Notes|
| -------- | :-------: | ---- |
|``pixel_input_file``| "" | If not empty, the TFile with the tree of the pixel event is opened and the content analysed and added to the output file.|
|``calibration_file`` | ``calibration/v1740`` | Load the calibration from the locations ``<calibration_file>_bd1_group_[0-3]_[offset-dV].txt``.|

### DRS additional options:

No additional options.

## Configuration file

Specify how the analysis should be run. All the lines starting with ``#`` are ignored.
