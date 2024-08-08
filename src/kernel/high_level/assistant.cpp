#include "assistant.hpp"

#include <stdlib.h>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include "hook_winapi.hpp"
#endif

assistant::assistant(){
    init_assistf(KL_DEBUG_OUTPUT);
}
assistant::assistant(bool _output, bool _log) : output(_output), log(_log){
    init_assistf(KL_DEBUG_OUTPUT);
}

assistant::~assistant(){
    #ifdef _WIN32
    for(const auto& it : hDLL_s)
        FreeLibrary(it.second);
    #endif
    log_file.close();
    dump_assistf();

    for(u32t i = 0; i < err_assistant.size(); ++i){
        free(err_assistant[i]->name_e);
        free(err_assistant[i]->desc_e);
        free(err_assistant[i]);
    }
}

void assistant::file_log_name(std::string name_file){
    if(name_file.empty()) return;
    log_file.open(name_file, std::ios::app);
}
void assistant::switch_log(bool val){
    log = val;
}
void assistant::switch_otp(bool val){
    output = val;
}
void assistant::add_err(std::string name_err, std::string desc){
    err_assistant.push_back(make_err_p(name_err.c_str(), desc.c_str(), err_assistant.size()));
}
void assistant::call_err(std::string name_err){
    i32t ind = find_err_ind_p(name_err.c_str(), err_assistant.data(), err_assistant.size());
    if(ind == -1){
        this->operator<<("An error with the name \"" + name_err + "\" was not found");
        err_fatal_ref(get_kernel_err(0));
    }

    this->operator<<(std::string(err_assistant[ind]->name_e) + "("+ std::to_string(ind) +"): " + err_assistant[ind]->desc_e);
    exit(EXIT_FAILURE);
}
void assistant::call_err(std::string name_err, std::string addit){
    i32t ind = find_err_ind_p(name_err.c_str(), err_assistant.data(), err_assistant.size());
    if(ind == -1){
        this->operator<<("An error with the name \"" + name_err + "\" was not found");
        err_fatal_ref(get_kernel_err(0));
    }

    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
    this->operator<<(std::string(err_assistant[ind]->name_e) + "("+ std::to_string(ind) +"): " + err_assistant[ind]->desc_e + "\nDetail: [" + addit + "]");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    exit(EXIT_FAILURE);
}

HND assistant::open_file(std::string name_file, i32t mode){
    if(name_file.find("C:\\") == name_file.npos)
        name_file = std::filesystem::current_path().string() + "\\" + name_file;
    name_all_files.push_back(name_file);
    return open_file(mode, name_all_files[name_all_files.size()-1]);
}
void assistant::close_file(HND handle){
    close_file_assist(get_path_file(get_pfile_ind(handle)));
}
HND assistant::get_handle_file(std::string name_file){
    if(name_file.find("C:\\") == name_file.npos)
        name_file = std::filesystem::current_path().string() + "\\" + name_file;
    return get_index_file(get_pfile(const_cast<char*>(name_file.c_str())));
}
std::string assistant::read_file(HND handle){
    char* ptr_buf = read_file_inp(get_pfile_ind(handle));
    std::string text(ptr_buf);
    free(ptr_buf);
    return text;
}
void assistant::write_file(HND handle, std::string buf){
    write_file_out(get_pfile_ind(handle), const_cast<char*>(buf.c_str()));
}
#ifdef _WIN32
DWORD assistant::get_error_win32(){
    return GetLastError();
}
HMODULE assistant::load_dll(std::string name_file){
    HMODULE hDLL = LoadLibraryExA(name_file.c_str(), NULL, NULL);
    if(!hDLL)
        return NULL;
    hDLL_s[name_file] = hDLL;
    return hDLL;
}
void assistant::dump_dll(std::string dll_name){
    if(hDLL_s.find(dll_name) == hDLL_s.end())
        return;
    FreeLibrary(hDLL_s[dll_name]);
    hDLL_s.erase(dll_name);
}
void assistant::safe_call_dll_func_begin(){
    _winapi_call_hook();
}
void assistant::check_safe_call_dll_func(){
    if(_winapi_call_knw_was_hook()){
        printf("\n");
        if(log)
            log_file << HOOK_TEXT_DETECTED << "\n";
        err_fatal_ref(get_kernel_err(2));
    }
}
void assistant::safe_call_dll_func_end(){
    _winapi_call_remove();
}
void* assistant::get_ptr_func(std::string dll_name, std::string name_func){
    if(hDLL_s.find(dll_name) == hDLL_s.end())
        return nullptr;
    return GetProcAddress(hDLL_s[dll_name], name_func.c_str());
}
HMODULE assistant::get_handle_module_dll(std::string dll_name){
    if(hDLL_s.find(dll_name) == hDLL_s.end())
        return 0;
    return hDLL_s[dll_name];
}
#endif
void assistant::operator<<(std::string text){
    if(output)
        printf("%s\n", text.c_str());
    if(log)
        log_file << text << "\n";
}


HND assistant::open_file(i32t mode, std::string &path){
    pfile pf;
    if(mode == MODE_READ_FILE)
        pf = open_file_inp(const_cast<char*>(path.c_str()), "read of file");
    else
        pf = open_file_out(const_cast<char*>(path.c_str()), "write to file");

    return get_index_file(pf);
}