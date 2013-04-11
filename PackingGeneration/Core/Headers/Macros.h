// Copyright (c) 2013 Vasili Baranau
// Distributed under the MIT software license
// See the accompanying file License.txt or http://opensource.org/licenses/MIT

#ifndef Core_Headers_Macros_h
#define Core_Headers_Macros_h

// The next symbols are defined in build options; they are placed here for reference.

// Determines, whether the program will work in parallel mode,
// independently of system type (posix/windows).
// Never call this define as "MPI", as MPI will report errors.
// #define PARALLEL

// #define DEBUG

//#ifdef LAPACK_AVAILABLE

// Gnu Scientific Library
// #define GSL_AVAILABLE

// #define SINGLE_PRECISION

// A useful keyword to mark overriden functions (to distinguish from virtual, which should be used for declaration)
#define OVERRIDE

// Format for scanf (see http://stackoverflow.com/questions/210590/why-does-scanf-need-lf-for-doubles-when-printf-is-okay-with-just-f)
#ifdef SINGLE_PRECISION
#define FLOAT_FORMAT "%f"
#else
#define FLOAT_FORMAT "%lf"
#endif

// A macro to disallow the copy constructor and operator= functions.
// See http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml?showone=Copy_Constructors#Copy_Constructors
// This should be used in the "private:" declarations for a class
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);             \
    void operator=(const TypeName&)

#endif /* Core_Headers_Macros_h */
