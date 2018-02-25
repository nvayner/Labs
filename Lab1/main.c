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
    
    traverse_dir(path);
    
    return EXIT_SUCCESS;
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

bool traverse_dir(char* path){
    printf("traversing '%s'\n", path);
    
    DIR *dir;
    if ((dir = opendir(path)) == NULL){
        perror("Directory opening failed");
        return false;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL){
        if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0)
            continue;
        
        char entry_name[FILENAME_MAX];
        sprintf(entry_name, "%s/%s", path, entry->d_name);
        printf("directory entry '%s'\n", entry_name);
        
        struct stat entry_stat;
        if (stat(entry_name, &entry_stat) == -1){
            perror("Entry statistics getting failed");
            continue;
        }
        if (S_ISDIR(entry_stat.st_mode)){
            traverse_dir(entry_name);
        } else if (S_ISREG(entry_stat.st_mode)){
            char full_name[FILENAME_MAX];
            if (realpath(entry_name, full_name) == NULL){
                perror("Full path getting failed");
                continue;
            }
            
            unsigned char hash_md5[HASH_MD5_SIZE];
            if (!calc_file_hash(entry_name, entry_stat.st_size, hash_md5)){
                perror("Hash calculating failed");
                continue;
            }
            
            printf("'%s' -> ", full_name);
            for (int i = 0; i < HASH_MD5_SIZE; i++)
                printf("%02X", hash_md5[i]);
            printf("\n");
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
