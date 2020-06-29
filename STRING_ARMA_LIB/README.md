### STRINGARMA. Photovoltaic facilities evaluation under mismatched situations

 Copyright (c) 2017 Joan Ferran Salaet Pereira
 Copyright (c) 2020 Josep Garreta Betriu
 Copyright (c) (2017-2020) Universitat Politecnica de Catalunya (UPC) 

---
 
### Contents

1.  Introduction

2.  Distribution License

3.  Compilers and External Dependencies

4.  Windows: Installation

5.  Documentation

---

### 1. Introduction

Whenever a PV panel is affected by environment conditions such as temperature, 
irradiance, dust or soiling, it is said that the panel is then working under
mismatched conditions.

Stringarma library is able to calculate the I-V characteristic or the state of 
any component of a PV panel under mismatched conditions, which structure is 
introduced through an external file by the user.

In order to solve the systems of non-linear equations that appear in the process,
the Armadillo C++ Linear Algebra Library is used during an iterative method.
Armadillo uses other third party libraries: LAPACK and BLAS, which are also 
included in this folder. The information related to the licenses of these third
party libraries can be found in the 'NOTICE.txt' file in this folder.

Authors:
  * Joan Ferran Salaet Pereira
  * Josep Garreta Betriu

---

### 2: Distribution License

Stringarma is licensed under the General Public License GNU (VERSION 3).
A copy of the License is included in the "COPYING.txt" file.

---

### 3: Compilers and External Dependencies

Stringarma library has been compiled using the GCC (MinGWx64) compiler 
on Windows 10.

The library is dependant of third party libraries. It has been compiled with 
the Armadillo library. However, the functionalities of Armadillo used in this 
library are dependent on other libraries: LAPACK and BLAS, which are also 
included in this folder and must be properly linked to your project.
The information related to the licenses of these third party libraries can 
be found in the 'NOTICE.txt' file in this folder.

---

### 4: Windows: Installation

The installation is comprised of 2 steps:

* Step 1:
  Copy the entire "include" folder to a convenient location
  and tell your compiler to use that location for header files
  (in addition to the locations it uses already).
  Alternatively, you can use the "include" folder directly.

* Step 2:
  Configure your compiler to link with STRINGARMA, LAPACK and 
  BLAS libraries, which are located inside the lib folder. In case
  the linking is not successful, try adding the libraries to the 
  location of your project. Alternatively, you can add the location
  of the libraries in the PATH of Windows.

---

### 5: Documentation

The documentation for Stringarma functions and classes is available in
the 'Doc' folder.

The documentation is available in HTML format, which can be viewed with 
a web browser, and PDF.

---