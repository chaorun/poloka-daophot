#ifndef COUNTEDREF__H
#define COUNTEDREF__H


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

  ~CountedRef() 
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
 private:
  int refcount;
  public :
    RefCount() :refcount(0) {};
  int& refCount()       {return refcount;}
  //  int  refCount() const {return refcount;}
  
  /* when a RefCount is copied, it means that the object itself is copied,
     hence the count sould be set to zero */
  RefCount(const RefCount &Other) 
    { 
      if (&Other) {}; // warning killer
      refcount = 0;
    }
#ifndef SWIG
  void operator = (const RefCount & Right)
    {
      if (&Right) {}; // warning killer
    }
#endif


};

#endif /*COUNTEDREF__H */
