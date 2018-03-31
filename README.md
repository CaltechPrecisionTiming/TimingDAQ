# Timing DAQ repository

The main aim of this code is to convert raw binary data recorded from SiPMs from different devices and analyze them in a coherent and standardised way.
The main executable applications will be created by the istallation in the main folder. One for each available device is present under the form: ``<device_name>Dat2Root``.

## Quick start
If you are not interested in the details just use the following commands

### Installation
To install copy paste the following:

```
git clone https://github.com/CaltechPrecisionTiming/TimingDAQ
make -j8
```


### Examples

``./DRSDat2Root --input_file=<your_file>.dat --config=config/DRS_Na22.config --N_evts=10000``

## Command line options
All the command line option must be passed using the sintax ``--<opt>=<val>``. For all options, ``--<opt>`` is equal to ``--<opt>=true``.
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
|``calibration_file`` | ``calibration/v1740`` | Load the calibration from the locations ``<calibration_file>_bd1_group_[0-3]_[offset-dV].txt``. If ``ZEROS``, put all calibration values to 0.|

### DRS additional options:

No additional options.

## Configuration file

Specify how the analysis should be run. The first part is supposed to specify general settings. The second one is supposed to set the channel specific configuration.

All the lines starting with ``#`` are ignored.
### General settings

| Line | Default | Notes|
| -------- | :-------: | ---- |
| ``Baseline <start> <len>`` | ``Baseline 5 150`` | Specify the indexes which delimit the constant fit to determine the baseline. The waveform is then fitted with a constant form the sample number ``<start>`` (included) to the sample number ``<start> + <len>`` (excluded).
| ``ConstantFraction <f_1> .. <f_n>`` | ``ConstantFraction 15 30 45`` | List of all fraction [%] at which the rising time has to be computed and saved in the tree. |

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
|``pol``| ``+`` or ``-`` | Polarity of the waveform. Pulses are supposed to have the peak below the baseline.|
|``amp`` | ``float`` | Value of the amplification applied [dB]. |
|``att`` | ``float`` | Value of the attenuation applied [dB]. |
|``algo`` | ``string`` | Algorithms to be applied to fit the rising edge to compute the constant fraction times. |
|``weies_filter`` | ``float`` | Not implemented yet. |

### Available fitting algorithms

The following list include all the algorithms that can be performed. If more than one algorithm is deisred, the flags shouls be separed by the character ``+``.

* ``G<frac>`` : Fit with a gaussian. The range is defined by ``frac``:  takes all the point near the peak that are above ``frac`` of the peak amplitude. If only ``G`` is given, it is interpreted as ``G40``.
* ``PL<deg>`` : Fit the rising edge with a polynomial of degree ``deg`` in the proximity of the closest point to the constant fraction.
* ``Re<f_start>-<f_stop>`` : Linear fit from the point closer to the fraction ``f_start``

## Automation

This folder collect all the scripts that try to automise the data reconstruction.

An example line is:

``./automation/run_reco.sh 130 132``
