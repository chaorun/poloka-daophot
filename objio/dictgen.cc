// -*- C++ -*-
// 
// file dictgen.cc
// 
//
#define _GNU_SOURCE
#include <getopt.h>

#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>

#include "xmlstream.h"
#include "codegen.h"
#include "cppclass.h"
#include "cppswigclassreader.h"



static struct option dictgen_options[] = {
  {"list",             0, 0, 'l'},
  {"list-members",     0, 0, 'L'},
  {"all",              0, 0, 'a'},
  {"output",           1, 0, 'o'},
  {"xmlschema-output", 1, 0, 'x'},
  {"swig-output",      1, 0, 's'},
  {"verbose",          0, 0, 'v'},
  {"help",             0, 0, 'h'},
  {0, 0, 0, 0}
};


std::string runSwig(string const& headerName, bool verbose=false);


void usage()
{
  std::cerr << "usage: dictgen [OPTIONS] header_file                          " << std::endl;
  std::cerr << " where OPTIONS can be:                                         " << std::endl;
  std::cerr << "   --list,-l             list the persistent struct and classes" << std::endl;
  std::cerr << "   --list-members,-L     list the contents of the persistent objects" << std::endl;
  std::cerr << "   --all,-a              list also the non-persistent classes  " << std::endl;
  std::cerr << "   --generate,-g         generate the persisters               " << std::endl;
  std::cerr << "   --output,-o           output files base name                " << std::endl;
  std::cerr << "   --swig-output,-s      swig original output file name        " << std::endl;
  std::cerr << "   --xmlschema-output,-x XMLSchema output file name            " << std::endl;
  std::cerr << "   --help,-h             print this message                    " << std::endl;
  exit(-1);
}



int 
main(int argc, char** argv)
{
  std::string xmlInputFileName;
  std::string xmlOutputFileName;
  
  int list=0;
  int verbose=0;
  int list_members=0;
  int generate_persisters=0;
  int generate_xmlschema=0;
  int generate_swig_stuff=0;
  
  std::string headerName;
  std::string outputFileName="";
  std::string swigOutputFileName="";
  std::string xmlSchemaOutputFileName="";
  
  
  char c;
  int opt_idx=0;
  while( (c=getopt_long(argc, argv, "hvlLao:x:s:", 
			dictgen_options, &opt_idx)) != -1 ) 
    switch(c) {
    case 'v':
      verbose=1;
      break;
    case 'l':
      list=1;
      break;
    case 'L':
      list=1;
      list_members=1;
      break;
    case 'o':
      outputFileName=optarg;
      generate_persisters=1;
      break;
    case 'x':
      generate_xmlschema=1;
      xmlSchemaOutputFileName=optarg;
      break;
    case 's':
      generate_swig_stuff=1;
      swigOutputFileName=optarg;
      break;
    case 'h':
      usage();
      break;
    default:
      usage();
    }
  if(optind>=argc) usage();
  headerName=argv[optind];
  
  std::string swig_xml_output_filename;
  swig_xml_output_filename = runSwig(headerName, verbose>0);
  
  CppSwigClassReader dr(swig_xml_output_filename);
  
  codegen cg(headerName);
  if(generate_persisters) {
    outputFileName = outputFileName.substr(0,outputFileName.find_last_of("."));
    cg.openOutputFiles(outputFileName);
  }
  
  int i,sz=dr.size();
  for(i=0;i<sz;i++) {
    CppClass c = dr.read();
    if(list && !list_members) 
      c.print(0);
    if(list_members) 
      c.print(1);
    if(generate_persisters)
      cg.generatePersister(c);
  }
  
  if(swig_xml_output_filename!="" && !generate_swig_stuff) 
    remove(swig_xml_output_filename.c_str());
  
  if(generate_swig_stuff)
    rename(swig_xml_output_filename.c_str(), swigOutputFileName.c_str());
  
  if(generate_persisters)
    cg.closeOutputFiles();
}



std::string  runSwig(string const& headerName, bool verbose)
{
  std::string cmd, swig_xml_output_name, config_file_name;
  std::stringstream sstrm;
  sstrm << getpid();
  
  // actually, we need to generate a real config file
  config_file_name = "dictgen_" + sstrm.str() + ".swig_config";
  ofstream ofs_config(config_file_name.c_str());
  ofs_config << "%module test" << std::endl
	     << "%{" << std::endl
	     << "%}" << std::endl
	     << "%define CLASS_VERSION(className,id) " 
	     << "static const unsigned short __version__=id; template<class ZZ1,class ZZ2> friend class persister;" << std::endl
	     << "%enddef" << std::endl
	     << "%include \"" << headerName << "\"" << std::endl;
  ofs_config.close();
  
  swig_xml_output_name = "dictgen_" + sstrm.str() + (std::string)".xml";  
  cmd = "swig -c++ -xml -w401 -module persist -o " 
    + swig_xml_output_name + " " + config_file_name;
  if(verbose) std::cout << cmd << std::endl;
  
  system(cmd.c_str());
  
  remove(config_file_name.c_str());
  return swig_xml_output_name;
}

