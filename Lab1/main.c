#include "main.h"

int main(int argc, char** argv) {
    char path[FILENAME_MAX];
    
    printf("%d\n", argc);
    for (int i=0; i<argc; i++)
        printf("%s\n", argv[i]);
    
    if (argc>1){
        strcpy(path, argv[1]);
        printf("Using explicit path '%s'\n", path);
    } else{
        if (!get_current_path(path, sizeof(path))){
            printf("Couldn't get current path; exiting");
            return EXIT_FAILURE;
        }
        printf("Using current path '%s'\n", path);
    }
    
    run(path, true);
    
    run(path, false);
    
    return EXIT_SUCCESS;
}

void run(char *path, bool depth){
    GTree *tree = g_tree_new(compareKeys);
    
    clock_t begin = clock();
    if (depth)
        traverse_dir_depth(path, tree);
    else
        traverse_dir_width(path, tree);
    clock_t end = clock();
    
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("\nExecution time with %s search: %f\n", (depth ? "depth" : "width"), time_spent);
    
    g_tree_foreach(tree, printTreeNode, NULL);
    
    g_tree_foreach(tree, deleteTreeNode, NULL);
    g_tree_destroy(tree);
}

bool get_current_path(char* buf, const size_t buf_size){
    char file_link_name[50];
    char file_link[FILENAME_MAX];
    size_t len;

    sprintf(file_link_name, "/proc/%d/exe", getpid());
    
    if ((len = readlink(file_link_name, file_link, sizeof(file_link))) != -1){
        file_link[len] = 0;
        strncpy(buf, dirname(file_link), buf_size-1);
        buf[buf_size-1] = 0;
    } else {
        perror("Error reading file link");
        return false;
    }
    return true;
}

bool traverse_dir_depth(char* path, GTree *tree){ 
    return process_path(path, tree, NULL);    
}

bool traverse_dir_width(char* path, GTree *tree){
    GQueue *queue = g_queue_new();
    g_queue_push_head(queue, path);
    
    gpointer node;
    bool result = true;
    while (result && (node = g_queue_pop_tail(queue)) != NULL){
        result = process_path((char*)node, tree, queue);
    }
    
    g_queue_free(queue);
    
    return result;
}

bool process_path(char *path, GTree *tree, GQueue *width_queue){
    DIR *dir;
    if ((dir = opendir(path)) == NULL){
        perror("Directory opening failed");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0)
            continue;

        char *entry_name = malloc(FILENAME_MAX * sizeof(char));
        sprintf(entry_name, "%s/%s", path, entry->d_name);

        struct stat entry_stat;
        if (stat(entry_name, &entry_stat) == -1){
            perror("Entry statistics getting failed");
            continue;
        }
        if (S_ISDIR(entry_stat.st_mode)){
            if (width_queue != NULL)
                g_queue_push_head(width_queue, entry_name);
            else
                traverse_dir_depth(entry_name, tree);
        } else if (S_ISREG(entry_stat.st_mode)){
            char *full_name = malloc(FILENAME_MAX * sizeof(char));
            if (realpath(entry_name, full_name) == NULL){
                perror("Full path getting failed");
                continue;
            }

            unsigned char *hash_md5 = malloc(HASH_MD5_SIZE * sizeof(unsigned char));
            if (!calc_file_hash(entry_name, entry_stat.st_size, hash_md5)){
                perror("Hash calculating failed");
                continue;
            }

            GQueue *queue = g_tree_lookup(tree, hash_md5);
            if (queue==NULL){
                queue = g_queue_new();
            }
            g_queue_push_head(queue, full_name);

            g_tree_insert(tree, hash_md5, queue);
        }
    }

    if (closedir(dir)==-1){
        perror("Directory closing failed");
    }
    
    return true;
}

bool calc_file_hash(char* path, off_t expected_file_size, unsigned char *hash_buf){
    int fd;
    if ((fd = open(path, O_RDONLY)) == -1){
        perror("File opening failed");
        return false;
    }

    unsigned char *content = malloc(expected_file_size * sizeof(unsigned char));
    if (content == NULL){
        printf("File content buffer allocation failed\n");
        return false;
    }
    
    off_t real_content_size = read(fd, content, expected_file_size);

    if (close(fd) == -1)
        perror("File closing failed");
    
    bool result = (MD5(content, real_content_size, hash_buf) != NULL);

    free(content);
    
    return result;
}

gint compareKeys(gconstpointer name1, gconstpointer name2){
    const unsigned char *sKey1 = name1;
    const unsigned char *sKey2 = name2;

    return memcmp(name1, name2, HASH_MD5_SIZE);
}

gint printTreeNode(gpointer key, gpointer value, gpointer data){
    unsigned char *sKey = key;
    GQueue *queue = value;
    
    if (g_queue_get_length(queue) > 1){
        printf("Hash: ");
        for (int i = 0; i < HASH_MD5_SIZE; i++)
            printf("%hhX", sKey[i]);
        printf(", Values (%u):\n", g_queue_get_length(queue));
        g_queue_foreach(queue, printQueueElem, NULL);
        printf("\n");
    }
    
    return FALSE;
}

gint deleteTreeNode(gpointer key, gpointer value, gpointer data){
    GQueue *queue = value;
    
    g_queue_free(queue);
    free(key);
    
    return FALSE;
}

void printQueueElem(gpointer value){
    char *sValue = value;
    printf("%s\t", sValue);
}