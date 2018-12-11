# Timing DAQ repository

- [Quick start](#quick-start)
  * [Installation](#installation)
  * [Examples](#examples)
- [Command line options](#command-line-options)
  * [VME additional options:](#vme-additional-options-)
  * [DRS additional options:](#drs-additional-options-)
- [Configuration file](#configuration-file)
  * [General settings](#general-settings)
  * [Channel specific settings](#channel-specific-settings)
  * [Available fitting algorithms](#available-fitting-algorithms)
- [Automation](#automation)


The main aim of this code is to convert raw binary data recorded from SiPMs from different devices and analyze them in a coherent and standardised way.
The main executable applications will be created by the istallation in the main folder. One for each available device is present under the form: ``<device_name>Dat2Root``.

## Quick start
If you are not interested in the details just use the following commands

### Installation
Requirements: ROOT (v6.9 or greater), a c++ compiler.
To install copy paste the following:

```
git clone https://github.com/CaltechPrecisionTiming/TimingDAQ
make -j8
```

If willing to run on CMSLPC cluster, all the requirements to run are satisfied running:
```
source /cvmfs/sft.cern.ch/lcg/views/LCG_89/x86_64-slc6-gcc62-opt/setup.sh
source  /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.10.02-19565/x86_64-slc6-gcc62-opt/bin/thisroot.sh
```

If willing to run on LXPLUS cluster, all the requirements to run are satisfied running:
```
source TimingDAQ/setup_lxplus.sh
```

### DRS Example

``./DRSDat2Root --input_file=<your_file>.dat --config=config/DRS_Na22.config --N_evts=10000``

Example plot produced with the flag ``--draw_debug_pulses`` in the FNAL test beam of March 2018.

![alt text](https://github.com/CaltechPrecisionTiming/TimingDAQ/blob/master/pulses_imgs/readme_example_pulse.png)

### Run VME reRECO

First of all be sure to copy all the files you need (Track, VME binary and NimPlus) in the correct folder.

```
./VMEDat2Root --input_file=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/RAW/VMEbin/RawDataSaver0CMSVMETiming_Run485.dat --pixel_input_file=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/RAW/Tracks/Run485_CMSTiming_converted.root --output_file=/eos/cms/store/group/phys_susy/razor/Timing/Mar2018FNAL/OTSDAQ/CMSTiming/reRECO_180419/DataCMSVMETiming_Run485.root --config=./config/FNALTestbeam_1803/VME_BTLreRECO_180419.config --N_evt_expected=6859
```

Or you can use the automated version
```
./automation/run_reRECO.sh 482 485 -F -f -np
```

## Command line options
All the command line option must be passed using the sintax ``--<opt>=<val>``. For all options, ``--<opt>`` is equal to ``--<opt>=true``.
The following option are common to all the devices.

Compulsory arguments don't have default value

| Argument | Default | Notes|
| -------- | :-------: | ---- |
|``input_file``|    |Binary input file with the data. Must end in ``.dat``.|
|``config`` |      |Configuration file (see below).|
|``verbose`` |  false |Activate verbosity.|
|``output_file``| ``<input>.root`` | Output root file containing a singe tree called ``pulse``. If not provided, the default option is to have the same name of the input file with the ``.root`` extension.|
|``N_evts`` | 0 | Event number at which stop reading the  input file. If 0, continue until ``eof`` is reached.|
|``start_evt`` | 0 | Event at which start analysing and writing in the TTree.|
|``save_raw`` | false | Boolean value indicating whether to save or not the raw ADC counts.|
|``save_meas``| false | Boolean value indicating whether to save or not the full time and voltage vector for each event.|
|``draw_debug_pulses`` | false | If true all the pulses are saved as ``.png``. Alternatively, ``--draw_debug_pulses=.<ext>`` will save the TCanvas in the given extension.|
|``N_evt_expected`` | -1 | If positive, number of event compared to the loaded ones. If positive and not matching, don't write the tree.|

### VME additional options:
| Argument | Default | Notes|
| -------- | :-------: | ---- |
|``pixel_input_file``| "" | If not empty, the TFile with the tree of the pixel event is opened and the content analysed and added to the output file.|
|``calibration_file`` | ``calibration/v1740`` | Load the calibration from the locations ``<calibration_file>_bd1_group_[0-3]_[offset-dV].txt``. If ``ZEROS``, put all calibration values to 0.|
|``Max_corruption`` | 10 | Max number of detected corruption before stopping the acquisition.|
|``NSkip<N>`` | 0 or 1 | Number of event to skip during the corruption number ``<N>``. By default skip 1 if corruption detected at the end of event header and 0 if detected in the middle.|

### DRS additional options:

No additional options.

## Configuration file

Specify how the analysis should be run. The first part is supposed to specify general settings. The second one is supposed to set the channel specific configuration.

All the lines starting with ``#`` are ignored.
### General settings

| Line | Default | Notes|
| -------- | :-------: | ---- |
| ``Baseline <start> <stop>`` | ``Baseline 5 150`` | Specify the start and stop time which delimit the constant fit to compute the baseline. Time units are the ones specified in the time field of the waveform.
| ``ConstantFraction <f_1> .. <f_n>`` | ``ConstantFraction 15 30 45`` | List of fractions [%] at which the rising time has to be computed and saved in the tree. |
| ``ConstantThreshold <t_1> .. <t_n>`` | ``ConstantThreshold `` | List of threshold [mV] at which the rising time has to be computed and saved in the tree. Default none.|
| ``z_DUT <z_1> .. <z_n>`` | ``z_DUT -50. 50.`` | List of the z position of the detectors under test where the x-y position has to be computed. Values are in mm. If the ``pixel_input_file = "" ``, this configuration is ignored. |

### Channel specific settings

To activate a channel is sufficient to have line that starts with the channel number. The channel number must be followed by other 5 parameters:

``<ch_num> <pol> <amp> <att> <algo> <weies_filter>``

An example line is:

``2   +  1.  4. Re20-70+LP2+G  0.``

Recommended algorithm are ``G40`` for Photek pulses and ``LP2`` for SiPMs pulses.

The meaning of each parameter is explained below:

| Parameter | Values | Notes|
| -------- | :-----: | ------|
|``ch_num``| ``unsigned int`` | Number of the channel to be activated.|
|``pol``| ``+``, ``-`` or ``.`` | Polarity of the waveform. Pulses are supposed to have the peak below the baseline. Adding ``.`` allow automatic polarity switch (e.g. ``+.``).|
|``amp`` | ``float`` | Value of the amplification applied [dB]. |
|``att`` | ``float`` | Value of the attenuation applied [dB]. |
|``algo`` | ``string`` | Algorithms to be applied to fit the rising edge to compute the constant fraction times. |
|``weies_filter`` | ``float`` | Not implemented yet. |

### Available fitting algorithms

The following list include all the algorithms that can be performed. If more than one algorithm is deisred, the flags shouls be separed by the character ``+``.

* ``None`` : No fit is run. Equivalent to have all the channels not fittable.
* ``G<frac>`` : Fit with a gaussian. The range is defined by ``frac``:  takes all the point near the peak that are above ``frac`` of the peak amplitude. If only ``G`` is given, it is interpreted as ``G40``.
* ``LP<deg>`` : Fit the rising edge with a polynomial of degree ``deg`` in the proximity of the closest point to the constant fraction.
* ``IL`` : Time given by the line passing from the 2 closest point to the constant fraction (threshold) specified.
* ``FL`` : Time given by the line passing from the 3 closest point to the constant fraction (threshold) specified.
* ``SPL`` : Time given by the SPline interpolation given by ``TGraph::Eval(v, 0, "S")```
* ``Re<f_start>-<f_stop>`` : Linear fit from the point closer to the fraction ``f_start``
* ``TOT`` : WS transform (sinc interpolation) and TimeOverThreshold for all fraction and threshold

## Automation

This folder collect all the scripts that try to automise the processing of data.

An example line is:

``python automation/DecodeVME.py 130 132``


TODO: Finish this section
