#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <sys/stat.h>
#include <string.h>
#include <libgen.h>

#define HASH_MD5_SIZE 16

#ifdef __cplusplus
extern "C" {
#endif

    bool get_current_path(char* buf, const size_t buf_size);
    bool traverse_dir(char* path);
    bool calc_file_hash(char* path, off_t expected_file_size, unsigned char *hash_buf);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */

