#ifndef _WIN32
#define _POSIX_C_SOURCE 200112L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#endif

int cpath(const char *input, char *output, size_t output_size, int to_absolute) {
    if (!input || !output || output_size == 0) return -1;
    char temp[262144];
    size_t len = strlen(input);
    if (len >= sizeof(temp)) return -1;
    strcpy(temp, input);
#ifdef _WIN32
    for (size_t i=0;i<len;i++) if (temp[i]=='/') temp[i]='\\';
#else
    for (size_t i=0;i<len;i++) if (temp[i]=='\\') temp[i]='/';
#endif
    size_t j = 0;
    for (size_t i=0;i<len;i++) {
        if (i>0 && temp[i]==temp[i-1] && (temp[i]=='/'||temp[i]=='\\')) continue;
        temp[j++] = temp[i];
    }
    temp[j]='\0';
    if (to_absolute) {
#ifdef _WIN32
        char full[262144];
        if (_fullpath(full,temp,sizeof(full))) strncpy(output,full,output_size-1);
        else return -1;
#else
        char full[262144];
        if (realpath(temp,full)) strncpy(output,full,output_size-1);
        else strncpy(output,temp,output_size-1);
#endif
        output[output_size-1]='\0';
    } else {
        strncpy(output,temp,output_size-1);
        output[output_size-1]='\0';
    }
    return 0;
}

char* read_out(const char *filename) {
    static char *buf = NULL;
    static size_t buf_size = 0;
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if ((size_t)size + 1 > buf_size) {
        char *new_buf = realloc(buf, size + 1);
        if (!new_buf) { fclose(f); return NULL; }
        buf = new_buf;
        buf_size = size + 1;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

int write_to(const char *filename, const char *data) {
    char path[262144]; 
    strncpy(path, filename, sizeof(path)-1); 
    path[sizeof(path)-1]='\0';
#ifdef _WIN32
    char *p = strrchr(path,'\\');
#else
    char *p = strrchr(path,'/');
#endif
    if (p) { 
        *p = '\0'; 
#ifdef _WIN32
        _mkdir(path);
#else
        mkdir(path, 0777);
#endif
    }
    FILE *f = fopen(filename,"w");
    if (!f) return -1;
    fputs(data, f);
    fclose(f);
    return 0;
}

int make_dir(const char *dirname) {
#ifdef _WIN32
    return _mkdir(dirname);
#else
    return mkdir(dirname,0777);
#endif
}

int copy_to(const char *target, const char *location) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(target);
    if (attr == INVALID_FILE_ATTRIBUTES) return -1;
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        _mkdir(location);
        WIN32_FIND_DATAA d; char search[262144];
        snprintf(search, sizeof(search), "%s\\*.*", target);
        HANDLE h = FindFirstFileA(search, &d);
        if (h == INVALID_HANDLE_VALUE) return -1;
        do {
            if (strcmp(d.cFileName, ".") == 0 || strcmp(d.cFileName, "..") == 0) continue;
            char child_target[262144], child_location[262144];
            snprintf(child_target, sizeof(child_target), "%s\\%s", target, d.cFileName);
            snprintf(child_location, sizeof(child_location), "%s\\%s", location, d.cFileName);
            copy_to(child_target, child_location);
        } while (FindNextFileA(h, &d));
        FindClose(h);
        return 0;
    } else {
        FILE *in = fopen(target, "rb");
        if (!in) return -1;
        char path[262144]; strncpy(path, location, sizeof(path)-1); path[sizeof(path)-1] = '\0';
        char *p = strrchr(path, '\\');
        if (p) { *p = '\0'; _mkdir(path); }
        FILE *out = fopen(location, "wb");
        if (!out) { fclose(in); return -1; }
        char buf[262144]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
        fclose(in); fclose(out);
        return 0;
    }
#else
    struct stat s;
    if (lstat(target, &s) != 0) return -1;
    if (S_ISDIR(s.st_mode)) {
        mkdir(location, 0777);
        DIR *dir = opendir(target);
        if (!dir) return -1;
        struct dirent *e;
        while ((e = readdir(dir)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char child_target[262144], child_location[262144];
            snprintf(child_target, sizeof(child_target), "%s/%s", target, e->d_name);
            snprintf(child_location, sizeof(child_location), "%s/%s", location, e->d_name);
            copy_to(child_target, child_location);
        }
        closedir(dir);
        return 0;
    } else {
        FILE *in = fopen(target, "rb");
        if (!in) return -1;
        char path[262144]; strncpy(path, location, sizeof(path)-1); path[sizeof(path)-1] = '\0';
        char *p = strrchr(path, '/');
        if (p) { *p = '\0'; mkdir(path, 0777); }
        FILE *out = fopen(location, "wb");
        if (!out) { fclose(in); return -1; }
        char buf[262144]; size_t n;
        while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
        fclose(in); fclose(out);
        return 0;
    }
#endif
}

int move_to(const char *target, const char *location) {
    return rename(target,location);
}

int delete_from(const char *target) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(target);
    if (attr == INVALID_FILE_ATTRIBUTES) return -1;
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        WIN32_FIND_DATAA d; char search[262144];
        snprintf(search, sizeof(search), "%s\\*.*", target);
        HANDLE h = FindFirstFileA(search, &d);
        if (h == INVALID_HANDLE_VALUE) return -1;
        do {
            if (strcmp(d.cFileName, ".") == 0 || strcmp(d.cFileName, "..") == 0) continue;
            char child[262144]; snprintf(child, sizeof(child), "%s\\%s", target, d.cFileName);
            delete_from(child);
        } while (FindNextFileA(h, &d));
        FindClose(h);
        RemoveDirectoryA(target);
        return 0;
    } else return DeleteFileA(target);
#else
    struct stat s;
    if (lstat(target, &s) != 0) return -1;
    if (S_ISDIR(s.st_mode)) {
        DIR *dir = opendir(target);
        if (!dir) return -1;
        struct dirent *e;
        while ((e = readdir(dir)) != NULL) {
            if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
            char child[262144]; snprintf(child, sizeof(child), "%s/%s", target, e->d_name);
            delete_from(child);
        }
        closedir(dir);
        rmdir(target);
        return 0;
    } else return remove(target);
#endif
}

int link_to(const char *target, const char *location) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(target);
    if(attr==INVALID_FILE_ATTRIBUTES) return -1;
    int dir_flag = (attr & FILE_ATTRIBUTE_DIRECTORY)?1:0;
    return CreateSymbolicLinkA(location,target,dir_flag)?0:-1;
#else
    return symlink(target,location);
#endif
}

int switch_to(const char *location) {
#ifdef _WIN32
    return _chdir(location);
#else
    return chdir(location);
#endif
}

char* list_out(const char *directory) {
    static char *buf = NULL;
    static size_t buf_size = 0;
    size_t offset = 0;
#ifdef _WIN32
    WIN32_FIND_DATAA d;
    char search[262144];
    snprintf(search, sizeof(search), "%s\\*.*", directory);
    HANDLE h = FindFirstFileA(search, &d);
    if (h == INVALID_HANDLE_VALUE) return NULL;
    do {
        if (strcmp(d.cFileName, ".") == 0 || strcmp(d.cFileName, "..") == 0) continue;
        char line[1024];
        LARGE_INTEGER sz; sz.HighPart = d.nFileSizeHigh; sz.LowPart = d.nFileSizeLow;
        snprintf(line, sizeof(line), "%llu %s\n", sz.QuadPart, d.cFileName);
        size_t len = strlen(line);
        if (offset + len >= buf_size) {
            size_t new_size = (offset + len + 1) * 2;
            char *new_buf = realloc(buf, new_size);
            if (!new_buf) break;
            buf = new_buf;
            buf_size = new_size;
        }
        strcpy(buf + offset, line);
        offset += len;
    } while (FindNextFileA(h, &d));
    FindClose(h);
#else
    DIR *dir = opendir(directory);
    if (!dir) return NULL;
    struct dirent *e;
    while ((e = readdir(dir)) != NULL) {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        char fpath[1024]; snprintf(fpath, sizeof(fpath), "%s/%s", directory, e->d_name);
        struct stat s; if (lstat(fpath, &s) != 0) continue;
        char tb[64]; struct tm *tm = localtime(&s.st_mtime); strftime(tb, sizeof(tb), "%b %d %H:%M", tm);
        char line[1024];
        if (S_ISLNK(s.st_mode)) {
            char target[1024]; ssize_t r = readlink(fpath, target, sizeof(target) - 1);
            if (r >= 0) { target[r] = '\0'; snprintf(line, sizeof(line), "%lu %lld %s %s -> %s\n",
                        (unsigned long)s.st_nlink, (long long)s.st_size, tb, e->d_name, target); }
            else snprintf(line, sizeof(line), "%lu %lld %s %s\n",
                        (unsigned long)s.st_nlink, (long long)s.st_size, tb, e->d_name);
        } else snprintf(line, sizeof(line), "%lu %lld %s %s\n",
                        (unsigned long)s.st_nlink, (long long)s.st_size, tb, e->d_name);
        size_t len = strlen(line);
        if (offset + len >= buf_size) {
            size_t new_size = (offset + len + 1) * 2;
            char *new_buf = realloc(buf, new_size);
            if (!new_buf) break;
            buf = new_buf;
            buf_size = new_size;
        }
        strcpy(buf + offset, line);
        offset += len;
    }
    closedir(dir);
#endif
    if (buf) buf[offset] = '\0';
    return buf;
}

int cmdexec(const char *file, char *const argv[]) {
#ifdef _WIN32
    if(!file||!argv) return -1;
    char cmdline[262144]="", buf[8192];
    for(int i=0;argv[i];i++){
        if(i) strncat(cmdline," ",sizeof(cmdline)-strlen(cmdline)-1);
        const char *arg=argv[i];
        char *p=buf; size_t r=sizeof(buf);
        int needs_quotes=(strchr(arg,' ')||strchr(arg,'\t')||strchr(arg,'"'));
        if(needs_quotes){ if(r-- >0)*p++='"'; }
        for(size_t j=0;j<strlen(arg);j++){
            size_t bs=0;
            while(j<strlen(arg)&&arg[j]=='\\'){ bs++; j++; }
            if(j==strlen(arg)){ for(size_t k=0;k<bs*(needs_quotes?2:1);k++){ if(r-->0)*p++='\\'; } break; }
            if(arg[j]=='"'){ for(size_t k=0;k<bs*2+1;k++){ if(r-->0)*p++='\\'; } if(r-->0)*p++='"'; }
            else{ for(size_t k=0;k<bs;k++){ if(r-->0)*p++='\\'; } if(r-->0)*p++=arg[j]; }
        }
        if(needs_quotes){ if(r-->0)*p++='"'; } *p=0;
        strncat(cmdline,buf,sizeof(cmdline)-strlen(cmdline)-1);
    }
    STARTUPINFOA si; PROCESS_INFORMATION pi;
    memset(&si,0,sizeof(si)); memset(&pi,0,sizeof(pi)); si.cb=sizeof(si);
    if(!CreateProcessA(NULL,cmdline,NULL,NULL,0,0,NULL,NULL,&si,&pi)) return -1;
    WaitForSingleObject(pi.hProcess,INFINITE);
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    return 0;
#else
    if(!file||!argv) return -1;
    pid_t pid=fork();
    if(pid==0){ execvp(file,argv); _exit(127); }
    if(pid<0) return -1;
    int status; waitpid(pid,&status,0); return status;
#endif
}
