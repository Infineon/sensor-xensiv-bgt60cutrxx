# XENSIV&trade; BGT60CUTRxx Radar Sensor Release Notes

This library provides functions for interfacing with the XENSIV&trade; BGT60CUTRxx 60GHz FMCW radar sensors.

## What's included?

- APIs for initializing the driver
- APIs for configuring the interrupt behavior
- APIs for reading the sensor data
- APIs for reading the motion detection data
- APIs for configuring and processing clustering of detected targets
- APIs for configuring and processing clutter localization
- Production test code for the XENSIV™ BGT60CUTRxx
- APIs for deep sleep and wake up of the sensor

## What Changed?

### 1.1.7

#### Add
- **FIFO read**: `radar_fifo_check_integrity()`, which reads the FIFO event and reports whether the radar frame can be trusted, without performing any I/O of its own

#### Fix
- **FIFO read**: `radar_fifo_read()` validates its pointer arguments and returns `XENSIV_BGT60CUTRXX_STATUS_INVALID_POINTER`, like the neighboring FIFO functions
- **FIFO read**: `radar_fifo_read()` leaves the FIFO threshold interrupt flag pending when the FIFO read fails, and propagates the error unchanged instead of flattening it to a device error

### v1.1.0 (public release)

In this version, the **deep sleep** feature for the XENSIV™ BGT60CUTRxx has been added. This feature allows the sensor to enter a low-power state, reducing power consumption when the sensor is not actively in use. Additionally, the **wake up** feature has been implemented to bring the sensor back to its operational state from deep sleep.


### Add
- **Power mode parameter**: added to the initialization function to select either active or deep sleep mode
- **Power mode parameter**: flag to enable deep sleep mode after ADC Calibration
- **Clutter localization**: deep sleep mode is now also supported in the clutter localization
- **Clutter localization**: interface to write the exclusion zones to the sensor
- **Deep sleep**: interface function to put the sensor into deep sleep mode
- **Wake up**: interface function to wake up the sensor from deep sleep mode
 

### v1.0.0 (public release)

This is the public release of the Xensiv cmos radar driver to demonstrate Infineon's radar capabilities by implementing the on-chip feature for motion detection on the CSK hardware platform

### v0.5.2 (internal release)

In this version, **production tests** for the XENSIV&trade; BGT60CUTRxx have been improved. Additionally, on the driver the source code has been formatted according to a coding style formatting tool to improve code readability and maintainability.

The driver has now been integrated into the repository which is used for the ModusToolbox&trade; code examples. This allows easier maintenance and faster updates of the driver in the future.

#### Change
- Improved antenna test settings: Increased number of samples, bandwidth, and other enhancements.
- Added compiler warning to antenna test regarding the impact of using the default calibration matrix.
- Changed start frequency in RX channel gain test to 61 GHz.
- Deprecated custom implementation of math functions in noise floor test.

### v0.5.1 (internal release)

#### Fix
- Frequency calculation for max current production test
- clutter localization: activation and deactivation of clutter localization

#### Change
- xensiv_bgt60cutrxx_noise_floor_test: improve runtime by moving calculation of sine wave outside of loop
- Formatting: adjusted line length to max 120 characters, applying coding style formatting tool


### v0.5.0 (internal release)

In this version the **clustering** feature for the XENSIV&trade; BGT60CUTRxx has been added. The clustering feature groups detected targets based on their range, azimuth, and elevation dimensions.

#### Add
- **Clustering**: structures and interface functions to configure and process clustering of detected targets


### v0.4.0 (internal release)

In this version the **EOL tests** for the XENSIV&trade; BGT60CUTRxx have been added. 
Furthermore, the **clutter localization** feature has been finalized and additional variables for **motion detection** have been added.

#### Add
- **Motion detection**: variable that indicates the number of detected targets in the current sequence
- **Motion detection**: flags that indicate which sequence number has triggered an interrupt (sequence 1 or sequence 2)
- **Clutter localization**: neighboring zones are now excluded in addition to the clutter detection zones
- **Clutter localization**: feature is periodically enabled to update the clutter zones
- **Clutter localization**: timeout feature to reset the clutter zones if no motion is detected for a certain time
- **Platform dependent code for Linux**: platform assert statement added to check for NULL pointers in functions
- **Production test code** for XENSIV&trade; BGT60CUTRxx

#### Fix
- **Motion detection**: fixed wrong assignment of azimuth and elevation angle with their corresponding E- and H-plane in the function radar_motion_detection_process_targets()

### v0.3.0 (internal release)

#### Add
- Interface function to set the IO voltage level either to 1.2V or 1.8V
- Interface function to set the crystal frequency either to 40MHz or 80MHz
- Interface to read the calibration parameters from the sensor
- error code for range zone error
- variable that stores which sequence has triggered an interrupt
- clutter localization and masking
- platform code for Linux

#### Change
- xensiv_bgt60cutrxx_init: added parameter to select if 40MHz or 80MHz crystal is used
- xensiv_bgt60cutrxx_init: added parameter to select the IO voltage level (1.8V or 3.3V)
- waveform_settings_t renamed to sequence_settings_t
- radar_motion_detection_init: added parameters for defining the rd and ulp sequence settings

#### Remove
- register configuration is not part of the driver anymore, because the configuration depends on the application
- radar_motion_detection_grid_set removed, because the grid is now part of the application and a pointer to the struct is used for initialization of the motion detection
- c-header file generation script removed, because the tool is now available as an executable for Windows and MacOS.

### v0.2.0 (internal release)

#### Add
- Structures and interface functions to read target registers of the motion detection hardware unit
- Interface function to decode motion detection target registers to International System (SI) units
- Several configuration files for different use case of the motion detection hardware unit
- Interface function for reading the motion detection zones from the sensor
- Interface to delay a task for micro seconds to get a more accurate timing when waiting on certain sensor events
- Mapping of interrupt names from pure numbers to human understandable names, e.g. IR0_EN_EN4 to IR0_EN_STAT4_ULP
- Compile switch to start the motion detection via interface function
- Interface function for decoding FIFO content and split the content into MTI, CFFT, RFFT, CFAR, ...


#### Change
- Debug prints are disabled via compile switch
- Interrupt handler removed from driver, because the interrupt handler must be part of the application code
- script for c-header generation removed, because the tool is no available as an executable for Windows and MacOS.

#### Fixed

#### Improved
- Motion detector renamed to motion detection because in all user manuals and specs, the hardware unit is called motion detection

### v0.1.0 (internal release)

- Initial pre-production release
- Supported devices: XENSIV&trade; BGT60CUTRxx

## Supported software and tools

This version of the XENSIV&trade; BGT60CUTRxx 60GHz radar sensor code example was validated for compatibility with the following software and tools:

| Software and tools                        | Version |
| :---                                      | :----:  |
| ModusToolbox software environment        | 3.4.0   |
| GCC compiler                              | 10.3.1  |

Minimum required ModusToolbox&trade; software environment: v3.4

© Infineon Technologies AG, 2026.
