#include "fileElem.h"
#include <iostream>
#include <stdexcept>

// Boost filesystem
#include <boost/version.hpp>
#if (BOOST_VERSION >= 105000)
  #define BOOST_FILESYSTEM_VERSION 3
#else
  // Needed for boost >=1.44
  #define BOOST_FILESYSTEM_VERSION 2
#endif

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace ukb {
  using namespace std;

  namespace fs = boost::filesystem;

  static inline string path_string(const boost::filesystem::path & p) {

#if (BOOST_VERSION < 103400)
	return p.native_file_string();
#elif (BOOST_VERSION < 105000)
	return p.file_string();
#else
	return p.generic_string();
#endif
  }

  template<class Dir> string dir_string(const Dir & p) {
#if (BOOST_VERSION < 103400)
	// Dir is a path object
	return p.native_file_string();
#elif (BOOST_VERSION < 105000)
	// Dir is a basic_directory object
	return p.path().file_string();
#else
	return p.path().generic_string();
#endif
  }

  std::string get_fname_absolute(const std::string & fname) {

	fs::path p(fname);

	return fs::canonical(fs::absolute(fs::path(fname))).string();
  }

  vector<string> extract_input_files(const string & fullname,
									 const string & extension) {

	vector<string> input_files;

	fs::path abs_path( fs::initial_path() );

	// Creates an absolute path
	abs_path = fs::system_complete( fs::path( fullname ) );

	if ( !fs::exists(abs_path)) {
	  throw runtime_error("extract_input_files error:" + path_string(abs_path) + " not found.");
	}

	if ( fs::is_directory(abs_path) ) {

	  fs::directory_iterator end_iter;
	  for ( fs::directory_iterator dir_itr( abs_path );
			dir_itr != end_iter;
			++dir_itr ) {
		if (fs::is_directory(*dir_itr)) continue;
		if (extension.size()) {
		  #if (BOOST_VERSION < 105000)
			string dfile = dir_itr->leaf();
		  #else
			string dfile = dir_itr->path().generic_string();
		  #endif
		  size_t ext_i = dfile.find_last_of('.');
		  if (ext_i == string::npos) continue;
		  string dext(dfile.begin() + ext_i + 1, dfile.end());
		  if (dext != extension) continue;
		}
		input_files.push_back(dir_string(*dir_itr));
	  }
	} else {
	  input_files.push_back(path_string(abs_path));
	}
	return input_files;
  }

  /////////////////////////////////////////////////////////////

  bool exists_file(const string & fname) {

	namespace fs = boost::filesystem;

	fs::path full_path( fs::initial_path() );

	#if (BOOST_VERSION < 105000)
	  full_path = fs::system_complete( fs::path( fname, fs::native ) );
	#else
	  full_path = fs::system_complete( fs::path( fname, (void*)fs::native ) );
	#endif
	return exists(full_path);
  }


  /////////////////////////////////////////////////////////////

  string basename(const string & fname) {

	namespace fs = boost::filesystem;

	fs::path full_path( fs::initial_path() );

	#if (BOOST_VERSION < 105000)
	  full_path = fs::system_complete( fs::path( fname, fs::native ) );
	  return full_path.leaf();
	#else
	  full_path = fs::system_complete( fs::path( fname, (void*)fs::native ) );
	  return full_path.generic_string();
	#endif
  }



  /////////////////////////////////////////////////////////////
  // Filesystem stuff (paths, extensions, etc)

  string::const_iterator find_last(string::const_iterator sit,
								   string::const_iterator sit_end,
								   char delim) {
	string::const_iterator sit_found = sit_end;
	for(;sit != sit_end;++sit)
	  if (*sit == delim) sit_found = sit;
	return sit_found;
  }



  File_elem::File_elem(const string & fname) {
	fill(fname);
  }

  File_elem::File_elem(const string & fullname_in,
					   const string & out_dir,
					   const string & new_ext) {

	fill(fullname_in);

	set_path(out_dir);

	if (new_ext.size())
	  ext = new_ext;
  }

  void File_elem::fill(const string & str) {

	boost::filesystem::path p(str);
	p.normalize();

	path = p.branch_path().string();
	if (path == "") path = ".";

	#if (BOOST_VERSION < 105000)
	   string file_fname = p.leaf(); // name + extension
	#else
	   string file_fname = p.generic_string(); // name + extension
	#endif

	string::const_iterator beg = file_fname.begin();
	string::const_iterator end = file_fname.end();
	string::const_iterator it = find_last(file_fname.begin(), file_fname.end(), '.');

	ext.assign(it, end);
	fname.assign(beg, it); // without extension
  }

  void File_elem::set_path(const string & out_dir) {
	size_t m = out_dir.size();
	if (m) {
	  if(!boost::filesystem::exists(out_dir)) {
		// Better die
		//boost::filesystem::create_directory( out_dir_dir );
		cerr << "Error: " << out_dir << " doesn't exist.\n" << endl;
		exit(-1);
	  }
	  if (out_dir[m-1] == '/') --m;
	  path.assign(out_dir, 0, m);
	}
  }

  string File_elem::get_fname() const {
	string res(path);
	res.append("/");
	res.append(fname);
	res.append(ext);
	return res;
  }
}
