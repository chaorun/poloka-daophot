#ifndef COUNTEDREF__H
#define COUNTEDREF__H

#include <iostream>


//! an implementation of "smart pointers" that counts references to an object. The obejct it "points" to has to derive from RefCount. 
template <class T> class CountedRef {

  private :
    T* p;

 public:

  CountedRef() { p = NULL;}
  CountedRef(T* pp) : p(pp) { if (p) (p->refCount())++;}
  /* explicit constness violation : */
  CountedRef(const T* pp)  { p = (T*) pp;  if (p) (p->refCount())++;}

#ifndef SWIG
  void operator = (const CountedRef &Right) 
    {  
      if (p) {(p->refCount())--;if (p->refCount() == 0) delete p;}
      if (&Right) { p = Right.p; if (p) (p->refCount())++;}
    }
#endif

  CountedRef(const CountedRef &Other) { p = NULL; *this = Other;}

  operator const T*() const {return p;}
  operator T*() { return p;}
  T* operator->() { return p;}
  const T* operator->() const { return p;}
  T& operator*() { return *p;}
  const T& operator*() const { return *p;}

#ifndef SWIG
#ifndef __CINT__
  bool operator == (const CountedRef<T> &Right) const {return p == Right.p;}
  bool operator == (const T* Right) const {return p == Right;}
  bool operator != (const CountedRef<T> &Right) const {return p != Right.p;}
  bool operator != (const T* Right) const {return p != Right;}

  template <class A> operator A*() { return dynamic_cast<A*>(p);}
  template <class A> operator const A*() const { return dynamic_cast<const A*>(p);}
#endif /*SWIG */

#endif /* __CINT__ */

  virtual ~CountedRef() 
    { 
      if (!p) return;
      (p->refCount())--;
      // tentative check :
      if (p->refCount() < 0) {cerr << " problem in ~RefCount: aborting "  << endl; abort();}
      if (p->refCount() == 0) 
     	{delete p;} 
    }
};


class RefCount {
public:
  RefCount() : refcount(0) {}
  
  /* when a RefCount is copied, it means that the object itself is copied,
     hence the count sould be set to zero */

  RefCount(const RefCount &Other) 
    { 
      if (&Other) {}; // warning killer
      refcount = 0;
    }

  /* when a RefCount deleted, one must have zero references to it */
  virtual ~RefCount()
    { 
      if(refcount!=0) {
	std::cerr << "Trying to delete a RefCount with non zero number of references" << std::endl;
	std::cerr << "There is a living CountedRef in memory (in a StarList for instance)" << std::endl;
	abort();
      }
    }
  
  
  int& refCount()   { return refcount; }

  
#ifndef SWIG
  void operator = (const RefCount & Right)
    {
      if (&Right) {}; // warning killer
    }
#endif

private:
  int refcount;
};

#endif /*COUNTEDREF__H */
