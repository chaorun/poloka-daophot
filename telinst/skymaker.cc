#ifdef VIRTUAL_INSTRUMENTS
class SkyMaker : public VirtualInstrument { /* TYPE_SELECTOR */

public :
  string TelInstName() const { return "Skymaker";}
  string TelName ()    const { return "Simulator";}
  string InstName ()   const { return "MyComputer";}

  static VirtualInstrument *Acceptor(const FitsHeader &Head) { 
    if  (string(Head.KeyVal("SNGTYPE")) == "LONG_EXPOSURE") return new SkyMaker; 
    else return NULL;
  }

};
#endif
