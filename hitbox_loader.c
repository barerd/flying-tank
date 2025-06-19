#define _XOPEN_SOURCE 700
#include <ftw.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <cjson/cJSON.h>
#include "entity.h"
#include "hitbox_loader.h"
#include "collision.h"

#define MAX_JSON_PATHS 256
static char* json_file_paths[MAX_JSON_PATHS];
static int json_file_count = 0;

// Called by nftw for each file
static int collect_json_files(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf) {
    (void)sb;
    (void)ftwbuf;

    if (typeflag == FTW_F && strstr(fpath, ".json")) {
        if (json_file_count < MAX_JSON_PATHS) {
            json_file_paths[json_file_count++] = strdup(fpath);
        }
    }
    return 0;
}

void load_all_hitboxes(const char* hitbox_root, Entity** entities, int entity_count) {
    json_file_count = 0;
    nftw(hitbox_root, collect_json_files, 10, FTW_PHYS);

    for (int i = 0; i < entity_count; i++) {
        Entity* e = entities[i];
        if (!e || !e->id) continue;

        // Look for a matching JSON file
        for (int j = 0; j < json_file_count; j++) {
            const char* json_path = json_file_paths[j];
            const char* filename = strrchr(json_path, '/');
            if (!filename) filename = json_path; // No slash: file is in root
            else filename++; // Skip the slash

            // Match e->id to filename without ".json"
            size_t id_len = strlen(e->id);
            size_t fname_len = strlen(filename);
            if (fname_len < 6 || strcmp(filename + fname_len - 5, ".json") != 0) continue;
            if (strncmp(filename, e->id, id_len) != 0) continue;
            if (filename[id_len] != '.' || strcmp(filename + id_len, ".json") != 0) continue;

            // Match found, load the file
            load_entity_hitbox(e, json_path);
            printf("Loaded hitbox for entity ID: %s from %s\n", e->id, json_path);
            break;  // stop after first match
        }
    }

    // Clean up collected file list
    for (int i = 0; i < json_file_count; i++) {
        free(json_file_paths[i]);
    }
    json_file_count = 0;

    printf("Finished loading hitboxes for %d entities\n", entity_count);
}
