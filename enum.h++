#ifndef ENUM_HPP
#define ENUM_HPP

#include <ostream>
#include <string>
#include <sstream>
#include <exception>
#include <H5Cpp.h>

namespace Enum {

static const char ENUM_HPP_SCCS_ID[] __attribute__((used)) = "@(#)enum.h++: $Id$";

template<const char *id>
class Enum {
  static int n;
public:
  class badsize: public std::exception {
    const std::string msg;
  public:
    virtual const char *what() const noexcept override {return msg.c_str();}
    badsize(const std::string &s): msg(s) {}
    badsize(std::string &&s): msg(std::forward<std::string>(s)) {}
    constexpr badsize() = delete;
  };
    
  static void setn(const int newn) {
    if (n)
      throw badsize(std::string("Size of Enum<")+std::string(id)
		    + std::string("> already set"));
    else
      n = newn;
  }
  static int getn() { return n; }
  friend inline std::ostream& operator<< (std::ostream& o, const Enum& e) {
    return o << std::string(id) << static_cast<int>(e);
  }
  constexpr int uniqid() const {return val;}
  virtual double match(const Enum& m) const {
    return m==*this?1.0-1.0*(n-1)/(n*n):1.0/(n*n);
  }
  virtual Enum& operator=(const Enum& m){
    static_cast<int&>(*this)=static_cast<int>(m);
    return *this;
  }
  constexpr Enum(const Enum &m):val(static_cast<int>(m)){}
  explicit Enum(const int val):val(val){
    if (!n) 
      throw badsize(std::string("Size of Enum<")+std::string(id)
		    + std::string("> not set"));
  }
  static const H5::DataType& DataType(void) {
    static H5::EnumType retval(H5::PredType::NATIVE_UINT);
    static bool inited(false);
    if (!inited) {
      std::ostringstream enumname;
      for (unsigned int i: SelfIterator::range(n)) {
	enumname << Enum(i);
	retval.insert(enumname.str().c_str(),&i);
	enumname.str("");
      }
      inited = true;
    }
    return retval;
  } 
protected:
  virtual ~Enum(void) = default; // virtual to force derived classes to have virtual destructors
private:
  int val;
  Enum(void):val(-1){
    if (!n) 
      throw badsize(std::string("Size of Enum<")+std::string(id)
		    + std::string("> not set"));
  }
  static int number(void) {return n;}
  explicit operator int& (void) {return val;}
  /*
  constexpr Enum operator+(const Enum &i) const {
    return Enum(static_cast<int>(*this)+static_cast<int>(i));
  }
  constexpr Enum operator-(const Enum &i) const {
    return Enum(static_cast<int>(*this)-static_cast<int>(i));
  }
  */
  constexpr Enum operator+(const int i) const {
    return Enum(static_cast<int>(*this)+static_cast<int>(i));
  }
  constexpr Enum operator-(const int i) const {
    return Enum(static_cast<int>(*this)-static_cast<int>(i));
  }
  Enum& operator++(void) {
    static_cast<int&>(*this)++; return *this;
  }
  Enum operator++(int) {
    auto ret(*this);
    static_cast<int&>(*this)++;
    return ret;
  }
  Enum& operator--(void) {
    static_cast<int&>(*this)--; return *this;
  }
  Enum operator--(int) {
    auto ret(*this);
    static_cast<int&>(*this)--;
    return ret;}
  Enum& operator+=(const Enum &i) {
    static_cast<int&>(*this)+=static_cast<int>(i);
    return *this;
  }
  Enum& operator-=(const Enum &i) {
    static_cast<int&>(*this)+=static_cast<int>(i);
    return *this;
  }
public:
  // This conversion operatore was private in Tanmoy's original code, but needs to be 
  // public if we want to be able to interpret the Enum as a sequence of bits
  explicit constexpr operator int (void) const {return val;}  

  // These comparison operators were private in Tanmoy's original code, but need to be
  // public if we want to use Enums in a map key
  constexpr bool operator<(const Enum &other) const {
    return static_cast<int>(*this) < static_cast<int>(other);
  }
  constexpr bool operator>(const Enum &other) const {
    return static_cast<int>(*this) > static_cast<int>(other);
  }
  constexpr bool operator<=(const Enum &other) const {
    return static_cast<int>(*this) <= static_cast<int>(other);
  }
  constexpr bool operator>=(const Enum &other) const {
    return static_cast<int>(*this) >= static_cast<int>(other);
  }

  constexpr bool operator==(const Enum &other) const {
    return static_cast<int>(*this) == static_cast<int>(other);
  }
  constexpr bool operator!=(const Enum &other) const {
    return static_cast<int>(*this) != static_cast<int>(other);
  }
  // template<typename,typename> friend class Enumvector;
  template<typename,bool> friend class SelfIterator::SelfIterator;
  template<typename,bool> friend class SelfIterator::Range;
  // template<typename T> friend auto range(T);
};

}
#endif
