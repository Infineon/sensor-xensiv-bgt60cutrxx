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
    [ "XENSIV&trade; BGT60CUTRxx radar sensor library", "index.html", "index" ],
    [ "XENSIV&trade; BGT60CUTRxx radar sensor driver porting guide", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html", [
      [ "XMC&trade; XMC1400 Arduino Kit edition", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md37", null ],
      [ "Introduction", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md39", [
        [ "Scope", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md40", null ],
        [ "Driver architecture overview", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md41", null ],
        [ "Driver package file structure", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md42", null ]
      ] ],
      [ "Porting checklists", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md44", [
        [ "Hardware requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md45", [
          [ "Essential hardware connections", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md46", null ],
          [ "Power supply requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md47", null ]
        ] ],
        [ "SPI configuration requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md48", [
          [ "SPI parameters", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md49", null ]
        ] ],
        [ "GPIO requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md50", null ],
        [ "Software prerequisites", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md51", [
          [ "Required tools", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md52", null ],
          [ "Required standard libraries", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md53", null ]
        ] ]
      ] ],
      [ "Driver architecture", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md55", [
        [ "Platform-independent layer", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md56", [
          [ "Main data structure", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md57", null ],
          [ "Core API functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md58", null ],
          [ "Status codes", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md59", null ]
        ] ],
        [ "Platform-dependent Hardware Abstraction Layer (HAL)", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md60", null ],
        [ "Block diagram", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md61", null ]
      ] ],
      [ "Hardware interface specifications", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md63", [
        [ "SPI bus timing diagram", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md64", [
          [ "SPI protocol format", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md65", null ]
        ] ],
        [ "Chip-level pin connections", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md66", [
          [ "BGT60CUTRxx Shield to XMC1400 Arduino Kit connection diagram", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md67", null ]
        ] ],
        [ "Reset timing requirements", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md68", null ]
      ] ],
      [ "Platform abstraction layer implementation", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md70", [
        [ "Required functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md71", null ],
        [ "Function specifications", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md72", [
          [ "Delay functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md73", null ],
          [ "GPIO control functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md74", null ],
          [ "SPI transfer functions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md75", null ],
          [ "Assert function", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md76", null ]
        ] ]
      ] ],
      [ "XMC1400 Arduino porting example", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md77", [
        [ "Hardware setup", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md78", null ],
        [ "Pin mapping", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md79", null ],
        [ "Platform implementation files", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#platform-implementation-files", [
          [ "File: <span class=\"tt\">xensiv_bgt60cutrxx_platform_arduino.h</span>", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md80", null ],
          [ "File: <span class=\"tt\">xensiv_bgt60cutrxx_platform_arduino.cpp</span>", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md81", null ]
        ] ],
        [ "Motion detection application", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#motion-detection-application", [
          [ "File: <span class=\"tt\">radar_motion_detection.ino</span>", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md82", null ]
        ] ]
      ] ],
      [ "Build and run instructions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md84", [
        [ "Arduino IDE setup", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md85", [
          [ "Prerequisites", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md86", null ]
        ] ],
        [ "Project structure", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md87", null ],
        [ "Building and uploading", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md88", null ]
      ] ],
      [ "Testing and validation", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md90", [
        [ "Basic connectivity test", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md91", null ],
        [ "Device ID verification", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md92", null ],
        [ "ADC calibration test", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md93", null ]
      ] ],
      [ "Troubleshooting", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md95", [
        [ "Common issues and solutions", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md96", null ],
        [ "Debug checklist", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md97", null ],
        [ "Enabling debug output", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md98", null ]
      ] ],
      [ "References", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md100", [
        [ "Documentation", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md101", null ],
        [ "Related-software", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md102", null ]
      ] ],
      [ "Appendix A: Register configuration example", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md104", null ],
      [ "Appendix B: Quick start checklist", "md_docs_2porting-guide_2SW-Porting-Guide-Arduino.html#autotoc_md106", null ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"index.html"
];

const SYNCONMSG = 'click to disable panel synchronization';
const SYNCOFFMSG = 'click to enable panel synchronization';
const LISTOFALLMEMBERS = 'List of all members';