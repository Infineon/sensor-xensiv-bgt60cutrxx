/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "sensor-xensiv-bgt60cutrxx", "index.html", [
    [ "XENSIV™ BGT60CUTRXX radar sensor", "index.html", "index" ],
    [ "XENSIV™ BGT60CUTRXX Radar Sensor Driver Porting Guide", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html", [
      [ "XMC1400 Arduino Kit Edition", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md17", null ],
      [ "Table of Contents", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md19", null ],
      [ "Introduction", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md21", [
        [ "1.1 Document Purpose", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md22", null ],
        [ "1.2 Driver Architecture Overview", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md23", null ],
        [ "1.3 Driver Package File Structure", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md24", null ]
      ] ],
      [ "Porting Checklist", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md26", [
        [ "2.1 Hardware Requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md27", [
          [ "Essential Hardware Connections", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md28", null ],
          [ "Power Supply Requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md29", null ]
        ] ],
        [ "2.2 SPI Configuration Requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md30", [
          [ "SPI Parameters", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md31", null ]
        ] ],
        [ "2.3 GPIO Requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md32", null ],
        [ "2.4 Software Prerequisites", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md33", [
          [ "Required Tools", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md34", null ],
          [ "Required Standard Libraries", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md35", null ]
        ] ]
      ] ],
      [ "Driver Architecture Deep Dive", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md37", [
        [ "3.1 Platform-Independent Layer", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md38", [
          [ "Main Data Structure", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md39", null ],
          [ "Core API Functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md40", null ],
          [ "Status Codes", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md41", null ]
        ] ],
        [ "3.2 Platform-Dependent Layer (HAL)", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md42", null ],
        [ "3.3 Block Diagram", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md43", null ]
      ] ],
      [ "Hardware Interface Specifications", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md45", [
        [ "4.1 SPI Bus Timing Diagram", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md46", [
          [ "SPI Protocol Format", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md47", null ]
        ] ],
        [ "4.2 Chip-Level Pin Connections", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md48", [
          [ "BGT60CUTRXX Shield to XMC1400 Arduino Kit Connection Diagram", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md49", null ]
        ] ],
        [ "4.3 Reset Timing Requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md50", null ]
      ] ],
      [ "Platform Abstraction Layer Implementation", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md52", [
        [ "5.1 Required Functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md53", null ],
        [ "5.2 Function Specifications", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md54", [
          [ "5.2.1 Delay Functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md55", null ],
          [ "5.2.2 GPIO Control Functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md56", null ],
          [ "5.2.3 SPI Transfer Functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md57", null ],
          [ "5.2.4 Assert Function", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md58", null ]
        ] ]
      ] ],
      [ "XMC1400 Arduino Porting Example", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md59", [
        [ "6.1 Hardware Setup", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md60", null ],
        [ "6.2 Pin Mapping", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md61", null ],
        [ "6.3 Platform Implementation Files", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md62", [
          [ "File: xensiv_bgt60cutrxx_platform_arduino.h", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md63", null ],
          [ "File: xensiv_bgt60cutrxx_platform_arduino.cpp", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md64", null ]
        ] ],
        [ "6.4 Motion Detection Application", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md65", [
          [ "File: radar_motion_detection.ino", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md66", null ]
        ] ]
      ] ],
      [ "Build and Run Instructions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md68", [
        [ "7.1 Arduino IDE Setup", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md69", [
          [ "Prerequisites", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md70", null ]
        ] ],
        [ "7.2 Project Structure", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md71", null ],
        [ "7.3 Building and Uploading", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md72", null ]
      ] ],
      [ "Testing and Validation", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md74", [
        [ "8.1 Basic Connectivity Test", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md75", null ],
        [ "8.2 Device ID Verification", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md76", null ],
        [ "8.3 ADC Calibration Test", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md77", null ]
      ] ],
      [ "Troubleshooting", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md79", [
        [ "9.1 Common Issues and Solutions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md80", null ],
        [ "9.2 Debug Checklist", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md81", null ],
        [ "9.3 Enabling Debug Output", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md82", null ]
      ] ],
      [ "References", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md84", [
        [ "10.1 Documentation", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md85", null ],
        [ "10.2 Related Software", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md86", null ]
      ] ],
      [ "Appendix A: Register Configuration Example", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md88", null ],
      [ "Appendix B: Quick Start Checklist", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md90", null ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"index.html"
];

var SYNCONMSG = 'click to disable panel synchronization';
var SYNCOFFMSG = 'click to enable panel synchronization';