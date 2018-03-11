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
#include <glib.h>

#define HASH_MD5_SIZE 16

#ifdef __cplusplus
extern "C" {
#endif

    bool get_current_path(char* buf, const size_t buf_size);
    bool traverse_dir_depth(char* path, GTree *tree);
    bool traverse_dir_width(char* path, GTree *tree);
    bool process_path(char *path, GTree *tree, GQueue *width_queue);
    bool calc_file_hash(char* path, off_t expected_file_size, unsigned char *hash_buf);
    gint compareKeys (gconstpointer name1, gconstpointer name2);
    gint printTreeNode(gpointer key, gpointer value, gpointer data);
    gint deleteTreeNode(gpointer key, gpointer value, gpointer data);
    void printQueueElem(gpointer value);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H */