%module simstring

%{
#include "export.h"
%}

%include "std_string.i"
%include "std_vector.i"
%include "exception.i"

namespace std {
    %template(StringVector) vector<std::string>;
}

%exception {
    try {
        $action
    } catch(const std::invalid_argument& e) {
        SWIG_exception(SWIG_IOError, e.what());
    } catch(const std::runtime_error& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch (const std::exception& e) {
        SWIG_exception(SWIG_RuntimeError, e.what());
    } catch(...) {
        SWIG_exception(SWIG_RuntimeError,"Unknown exception");
    }
}

%include "export.h"

