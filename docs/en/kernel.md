# KERNEL
The core  is  a  structure  consisting  of  two  components,  and they are very  dependent on each other:

 - a low-level  core
 - a high-level core(assistant)

[The kernel directory](https://github.com/svec1/bweas/tree/main/src/kernel)

### LOW-LEVEL

The low-level kernel provides a system for tracking files, interacting with them, and error handling.
#### File Tracking
File tracking is described in the [sys_file.h](https://github.com/svec1/bweas/blob/main/src/kernel/low_level/sys_file.h) file. The file description is sys_file.h:

 -
        typedef  struct  inf_file{
    	    str  name;
    	    str  descr;
    	    struct  tm  *strack_file;
        } inf_file;
           

	This  structure  describes the information  about the file: 

	 - str  name - it is logical  that the file  name. 
	 - str  descr - is the description of this file, basically it is expected why this file
	   was opened.  
	 - struct  tm  *strack_file  -  file  opening  time.

	**The last  two  are  mostly  not  that  important.**
	
	***[sys_file.c](https://github.com/svec1/bweas/blob/main/src/kernel/low_level/sys_file.c)***

-
       typedef  struct  file{
        	FILE**  files;
        	inf_file*  desc;
        	size32t  count;
       } _kl_file;

	This is a structure  that  combines an array of files  that  will  be  monitored  further,  their  information,  it  also  			has  information  about the number of elements  in the file  array  (the files  array  and  desc  must  always  have the same  number of elements).

- 
	    void  init_track_file(u32t  size_buf_inf_f);
        void  free_track_f_list();
	
	Initialization functions of the global variable `_kl_file krnlFiles`(they are the opposite).

-     void  kl_track_file(FILE*  file, str  name, str  open_inf);
      void  kl_strack_file(u32t  ind);

	Functions that start and stop tracking files.

**These  functions  and  structures  are  more  important  for  understanding  this  system.**

#### Error System

The error system is described  in the [err.h](https://github.com/svec1/bweas/blob/main/src/kernel/low_level/err.h) file:
	

 - 
       typedef  struct  err{
    	  str  name_e;
          str  desc_e;
          u32t  ind;
       } _err;
	
	Everything  seems to be clear here:
	 - str name_e - error  name
	 - str desc_e - error  information
	 - u32t ind - error index

 - 
       _err  make_err(cstr  name, cstr  desc, u32t  ind);
       _err*  make_err_p(cstr  name, cstr  desc, u32t  ind);

	These functions create either a variable of type *_err* or a pointer to *_err*.


-     void  add_err(_err  obj, _err  **src, size32t  *size_src);

	Adds an error to a  dynamic  array of errors,  with  already  existing  elements.

**The functions  that  were  not  specified  here  are  auxiliary.**

And  finally, a small  abstract  API  for the correct  interaction of all  these  "systems", an assistant.  It  provides  important  tools  for  working  with  errors  and  files,  as  well as error  handling,  and  kernel  error  initialization.
This small API is presented in the [assist_f.h](https://github.com/svec1/bweas/blob/main/src/kernel/low_level/assist_f.h) file:
	
 1.     void  init_assistf(u32t  output_warning);
	  void  dump_assistf();

	These functions initialize/reset the settings of this API.

 2.     const  pfile  KERNELF  open_file_inp(str  path, str  inf_open);
       const  pfile  KERNELF  open_file_out(str  path, str  inf_open);

	These  functions  provide the ability to interact  with  files  by  opening  them  in  different  modes  (read-"inp"/write-"out").

 3.     str KERNELF read_file_inp(const pfile file);
      void KERNELF write_file_out(const pfile file, str src);

	These  functions  provide  reading  and  writing to a  file,  they  require a constant  pointer to the  file  that the previous  functions  return

 4. `void close_file_assist(str path);`
	
	A function that closes a file that has already been opened by the functions of this API.

 5.     void KERNELF err_fatal_obj (_err obj_err);
      void KERNELF err_fatal_ref (const _err *obj_err);

	Functions that handle the error passed to the parameters.

 6. `const _err*  get_kernel_err(u32t ind);`

	A function  that  returns a reference  to a kernel  error  by the index  passed  in the parameter.
	There are 3  kernel  errors in total:
		
		 1. KNL001:Undefined behavior - The kernel can no longer serve the program.
		 2. KNL002:Out of range - The kernel noticed that the value was out of range.
		 3. KNL003:Danger Found - The kernel has noticed the danger. Security is not guaranteed.

### HIGH-LEVEL

The high  level of the kernel  provides a class  for  easy  access  to the low-level kernel,  logging,  as  well as the ability to load  dynamic  libraries  with  interception of WinAPI  functions(Windows) and interception of system call(functions from standarts libraries for linux).
To  achieve  logging,  this  level is used  for  any  output of information(must).

##### WINAPI

It's not  worth  disassembling  default  functions, it's a waste of time,  but  downloading  dll  libraries  and  interceptions is worth it.

 -     HMODULE  load_dll(std::string  dll_name);
       void  dump_dll(std::string  dll_name);

	Loads/free the dll. Moreover, when  loading, a certain  handle is returned  (the library  index  in the vector).

 - `DWORD get_error_win32();`

	Returns the WinAPI  error  code.

 - `void*  get_ptr_func(std::string  dll_name, std::string  name_func);`
 
	 Returns a pointer  to a function  from a dll,  and  name_func  should  match the name of the function  after  the  linker  for the dll has worked(use  extern  "C"  for  ease).

 -     void  safe_call_dll_func_begin();
	   void  safe_call_dll_func_end();

	A function  with the prefix  begin  produces a hook, a function  with the prefix  end,  returns  all  pointers  back  (ends the interception).

##### UNIX
For Unix-like systems, exactly the same functions are presented (relevant) as for WinApi. The only difference is that in the case of WinAPI, interception occurs in real time, and in UNIX-like systems, interception occurs during the assembly of this system, when, after generating a special dynamic library where the intercepted functions are declared, the linker links this library with the LD_PRELOAD preset.
