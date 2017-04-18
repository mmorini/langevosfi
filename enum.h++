#ifndef ENUM_HPP
#define ENUM_HPP

#include <iostream>
#include <string>
#include <exception>

static const char ENUM_HPP_SCCS_ID[] __attribute__((used)) = "@(#)enum.h++: $Id$";

template<const char *id>
class Enum {
  static int n;
public:
  class badsize: public std::exception {
    const std::string msg;
  public:
    virtual const char *what() const noexcept {return msg.c_str();}
    badsize(const std::string &s): msg(s) {}
    badsize(std::string &&s): msg(std::forward<std::string>(s)) {}
    badsize() = delete;
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
  int uniqid() const {return val;}
  virtual double match(const Enum& m) const {
    return m==*this?1.0-1.0*(n-1)/(n*n):1.0/(n*n);
  }
  Enum& operator=(const Enum& m){
    static_cast<int&>(*this)=static_cast<int>(m);
    return *this;
  }
  Enum(const Enum &m):val(static_cast<int>(m)){}
protected:
  Enum(const int val):val(val){
    if (!n) 
      throw badsize(std::string("Size of Enum<")+std::string(id)
		    + std::string("> not set"));
  }
  virtual ~Enum(void){}
private:
  int val;
  Enum(void):val(-1){
    if (!n) 
      throw badsize(std::string("Size of Enum<")+std::string(id)
		    + std::string("> not set"));
  }
  static int number(void) {return n;}
  explicit operator int (void) const{return val;}
  explicit operator int& (void) {return val;}
  Enum operator+(const Enum &i) const {
    return Enum(static_cast<int>(*this)+static_cast<int>(i));
  }
  Enum operator-(const Enum &i) const {
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
  bool operator<(const Enum &other) const {
    return static_cast<int>(*this) < static_cast<int>(other);
  }
  bool operator>(const Enum &other) const {
    return static_cast<int>(*this) > static_cast<int>(other);
  }
  bool operator<=(const Enum &other) const {
    return static_cast<int>(*this) <= static_cast<int>(other);
  }
  bool operator>=(const Enum &other) const {
    return static_cast<int>(*this) >= static_cast<int>(other);
  }
public:
  bool operator==(const Enum &other) const {
    return static_cast<int>(*this) == static_cast<int>(other);
  }
  bool operator!=(const Enum &other) const {
    return static_cast<int>(*this) != static_cast<int>(other);
  }
  template<typename,typename> friend class Enumvector;
  template<typename,bool> friend class SelfIterator;
  template<typename,bool> friend class Range;
  // template<typename T> friend auto range(T);
};

#endif
