// -*- C++ -*-
// $Id: cppclass.h,v 1.3 2004/03/06 23:15:42 nrl Exp $
// 
// \file cppclass.h
// 
// Last modified: $Date: 2004/03/06 23:15:42 $
// By:            $Author: nrl $
// 
// POTENTIAL PROBLEMS:
//  * no way to know how many template args a class has. 
//    swig does not tell us. Therefore, we have to use
//    an additional "defineTemplateArgs" function.
//    (someday, we will write our own little parser, it will be more efficient).
// 
#ifndef CPPCLASS_H
#define CPPCLASS_H

#include <vector>

#include "cpptype.h"
#include "cppclassmember.h"


class CppClassTemplateInstance;


class CppClass : public CppType {
public:
  CppClass(CppClass const& cppc) { copy(cppc); }
  CppClass(std::string const& kind, std::string const& classname, int version,
	   bool isTemplate=false);
  ~CppClass();
  
  int                           size() const { return memberList_.size(); }
  CppClassMember const&         baseClass(int i) const { return baseList_[i]; }
  std::vector<CppClassMember> const&  baseList() const { return baseList_; }
  CppClassMember const&         member(int i) const { return memberList_[i]; }
  
  int                           version() const { return version_; } 
  CppClass                      instantiate(std::string const&) const;
  
  void                          copy(CppClass const&);
  CppClass&                     operator=(CppClass const& t) { copy(t); return *this; }

  //  void                     copy(CppType const&);
  //  CppClass&                operator=(CppType const& t) { copy(t); return *this; }
  
  CppTemplateInstance           readFromHeaderSpec(std::string const& str) const;
  void                          addMember(CppClassMember const&);
  void                          addBaseClass(CppClassMember const&);
  void                          defineTemplateArgs(std::vector<std::string> const&);
  
  void                          print(int verbosity=0) const;
  
private:
  int         version_;
  std::string kind_;
  std::vector<CppClassMember> baseList_;
  std::vector<CppClassMember> memberList_;
  
  void clear_();
};


#endif

