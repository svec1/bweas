#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HOOK_DETECTED 0xDEC
#define HOOK_DETECTED_NULL NULL
#define HOOK_DETECTED_STR                                                                                              \
    "WARNING!!!A system call was detected, most likely the dynamically loaded dll library is hostile."

#define BEG_RED_TEXT "\033[1;31m"
#define END_RED_TEXT "\033[0m\n"

#define HOOK_DETECTED_INF(add_inf) printf(BEG_RED_TEXT HOOK_DETECTED_STR END_RED_TEXT "Detail: " add_inf "\n");

typedef int socklen_t;

int
system(const char *string) {
    HOOK_DETECTED_INF("system() call")
    return HOOK_DETECTED;
}

int
chmod(const char *filename, int mode) {
    HOOK_DETECTED_INF("chmod() call")
    return HOOK_DETECTED;
}
int
fchmod(int fildes, mode_t mode) {
    HOOK_DETECTED_INF("fchmod() call")
    return HOOK_DETECTED;
}
int
chown(const char *pathname, uid_t owner, gid_t group) {
    HOOK_DETECTED_INF("chown() call")
    return HOOK_DETECTED;
}
int
fchown(int fd, uid_t owner, gid_t group) {
    HOOK_DETECTED_INF("fchown() call")
    return HOOK_DETECTED;
}
int
lchown(const char *pathname, uid_t owner, gid_t group) {
    HOOK_DETECTED_INF("lchown() call")
    return HOOK_DETECTED;
}
void *
mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset) {
    HOOK_DETECTED_INF("mmap() call")
    return HOOK_DETECTED;
}
int
munmap(void *start, size_t length) {
    HOOK_DETECTED_INF("munmap() call")
    return HOOK_DETECTED;
}
int
setuid(uid_t uid) {
    HOOK_DETECTED_INF("setuid() call")
    return HOOK_DETECTED;
}
int
setgid(gid_t gid) {
    HOOK_DETECTED_INF("setgid() call")
    return HOOK_DETECTED;
}
int
seteuid(uid_t euid) {
    HOOK_DETECTED_INF("seteuid() call")
    return HOOK_DETECTED;
}
int
setegid(gid_t egid) {
    HOOK_DETECTED_INF("setegid() call")
    return HOOK_DETECTED;
}
int
listen(int sockfd, int backlog) {
    HOOK_DETECTED_INF("listen() call")
    return HOOK_DETECTED;
}
int
socket(int domain, int type, int protocol) {
    HOOK_DETECTED_INF("socket() call")
    return HOOK_DETECTED;
}
int
bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    HOOK_DETECTED_INF("bind() call")
    return HOOK_DETECTED;
}
int
recv(int s, void *buf, size_t len, int flags) {
    HOOK_DETECTED_INF("recv() call")
    return HOOK_DETECTED;
}
int
recvfrom(int s, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) {
    HOOK_DETECTED_INF("recvfrom() call")
    return HOOK_DETECTED;
}
int
recvmsg(int s, struct msghdr *msg, int flags) {
    HOOK_DETECTED_INF("recvmsg() call")
    return HOOK_DETECTED;
}
ssize_t
send(int s, const void *msg, size_t len, int flags) {
    HOOK_DETECTED_INF("send() call")
    return HOOK_DETECTED;
}
ssize_t
sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    HOOK_DETECTED_INF("sendto() call")
    return HOOK_DETECTED;
}
ssize_t
sendmsg(int s, const struct msghdr *msg, int flags) {
    HOOK_DETECTED_INF("sendmsg() call")
    return HOOK_DETECTED;
}