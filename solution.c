#include <dirent.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define ROOT_DIR "./test"
#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 64
#define MAX_LINE_LENGTH 256
#define MAX_CHAIN_LENGTH 25600
#define KEY_STR "Minotaur"
#define INCLUDE_STR "@include"

void print_error(char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

bool is_dots_dir(struct dirent *entry) {
    return strcmp(entry->d_name, ".") == 0|| strcmp(entry->d_name, "..") == 0;
}

char *get_next_path(char *source_dir, char *next) {
    char *next_path = (char *) malloc(MAX_PATH_LENGTH);
    snprintf(next_path, MAX_PATH_LENGTH, "%s/%s", source_dir, next);
    return next_path;
}

struct {
    char *filename;
    char *path;
} typedef FileType;

struct {
    FileType **data;
    size_t len;
    size_t size;
} typedef FileList;

FileList *create_file_list() {
    FileList *new_list = (FileList *) malloc(sizeof(FileList));
    new_list->data = NULL;
    new_list->len = 0;
    new_list->size = 1;
    return new_list;
}

FileType *create_file(char *filename, char *path) {
    FileType *new_file = (FileType *) malloc(sizeof(FileType));
    new_file->filename = filename;
    new_file->path = path;
    return new_file;
}

FileType *find_file_in_list(FileList *list, char *filename) {
    for (int i = 0; i < list->len; i++) {
        FileType *current_file = list->data[i];
        if (strcmp(current_file->filename, filename) == 0) {
            return current_file;
        }
    }
    return NULL;
}

void add_to_file_list(FileList *list, FileType *file) {
    if (list->len + 1 >= list->size) {
        list->size *= 2;
        list->data = (FileType **) realloc(list->data, list->size*sizeof(FileType *));
        if (!list->data) print_error("Realloc list->data is failed");
    }
    list->data[list->len++] = file;
}

void make_file_list(char *dir_path, FileList *list) {
    DIR *dir = opendir(dir_path);
    if (!dir) print_error("Dir is not opened");

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (is_dots_dir(entry)) continue;
            //printf("DIR: %s\n", entry->d_name);
            char *new_path = get_next_path(dir_path, entry->d_name);
            make_file_list(new_path, list);
        } else if (entry->d_type == DT_REG) {
            char *file_path = get_next_path(dir_path, entry->d_name);
            //printf("%s: %s\n", entry->d_name, file_path);
            FileType *file_item = find_file_in_list(list, entry->d_name);
            if (file_item == NULL) {
                FileType *new_file_item = create_file(entry->d_name, file_path);
                add_to_file_list(list, new_file_item);
            }
        }
    }
    closedir(dir);
}

void print_file_list(FileList *list) {
    FILE *fp = fopen("result.txt", "w");
    for (int i = 0; i < list->len; i++) {
        FileType* current_file = list->data[i];
        fprintf(fp, "%s: %s\n", current_file->filename, current_file->path);
    }
    fclose(fp);
}

bool strstarts(char *source, char *starts) {
    return strncmp(source, starts, strlen(starts)) == 0;
}

char result_chain[MAX_CHAIN_LENGTH];
bool solve_labyrinth(FileList *list, char *filename) {
    FileType *file_item = find_file_in_list(list, filename);
    FILE *fp = fopen(file_item->path, "r");
    if (!fp) print_error("File is not opened");
    char line[MAX_LINE_LENGTH];
    while(fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
        if (strstarts(line, KEY_STR)) {
            //printf("%s\n", file_item->path);
            snprintf(result_chain, MAX_CHAIN_LENGTH, "%s%s\n", result_chain, file_item->path);
            return true;
        } else if (strstarts(line, INCLUDE_STR)) {
            char new_filename[MAX_FILENAME_LENGTH];
            sscanf(line, "@include %s", new_filename);
            if (solve_labyrinth(list, new_filename)) {
                //printf("%s\n", file_item->path);
                snprintf(result_chain, MAX_CHAIN_LENGTH, "%s%s\n", result_chain, file_item->path);
                return true;
            }
        }
    }
    fclose(fp);
    return false;
}

int main() {
    FileList *list = create_file_list();
    make_file_list(ROOT_DIR, list);
    print_file_list(list);
    //solve_labyrinth(list, "file.txt");
    //FILE *fp = fopen("result.txt", "w");
    //fputs(result_chain, fp);
    //fclose(fp);
    return 0;
}
