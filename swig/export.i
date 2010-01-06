%module simstring

%{
#include "export.h"
%}

%include "std_string.i"
%include "std_vector.i"
namespace std {
    %template(StringVector) vector<std::string>;
}

%include "export.h"

