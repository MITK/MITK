
// 325 : Nested structs not wrapped automatically.
// 362 : Operator= is ignored.
// 511 : Can't use keyword arguments with overloaded functions
#pragma SWIG nowarn=325,362,509,511
//
// Includes for STD-library support
//
%include <std_vector.i>
%include <std_string.i>
%include <std_map.i>
%include <std_complex.i>
// Use C99 int support
%include <stdint.i>

//
// Template definition for the most common vector types
//
namespace std {
  %template(VectorBool) vector<bool>;
  %template(VectorUInt8) vector<uint8_t>;
  %template(VectorInt8) vector<int8_t>;
  %template(VectorUInt16) vector<uint16_t>;
  %template(VectorInt16) vector<int16_t>;
  %template(VectorUInt32) vector<uint32_t>;
  %template(VectorInt32) vector<int32_t>;
  %template(VectorUInt64) vector<uint64_t>;
  %template(VectorInt64) vector<int64_t>;
  %template(VectorFloat) vector<float>;
  %template(VectorDouble) vector<double>;
  %template(VectorUIntList) vector< vector<unsigned int> >;
  %template(VectorString) vector< std::string >;

  %template(MapDoubleDouble) map<double, double>;
  %template(MapStringDouble) map<std::string, double>;

  %template(PairDoubleDouble) pair<double, double>;
  %template(PairStringDouble) pair<std::string, double>;

  %template(VectorMapStringDouble) vector< map< std::string, double> >;
  %template(VectorPairStringDouble) vector< pair< std::string, double> >;
}