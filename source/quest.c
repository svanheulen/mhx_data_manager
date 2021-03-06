/*
Copyright 2016 Seth VanHeulen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "common.h"
#include "ui.h"

#define QUEST_SIZE_SMALL 0x1400
#define QUEST_SIZE_LARGE 0x1C00
#define INSTALLED_OFFSET 0x34
#define QUEST_OFFSET 0x45D4

const int challenge_quest_ids[] = {
    1020001, 1020002, 1020003, 1020004, 1020005, 1020006, 1020007, 1020008, 1020009, 1020010,
    1020011, 1020012, 1020013, 1020014, 1120001, 1120002, 1120003, 1120004, 1120005, 1120006
};

const int event_quest_ids[] = {
    1010001, 1010002, 1010003, 1010004, 1010101, 1010102, 1010103, 1010104, 1010105, 1010106,
    1010107, 1010108, 1010109, 1010110, 1010111, 1010112, 1010113, 1010114, 1010115, 1010116,
    1010117, 1010118, 1010119, 1010120, 1010121, 1010122, 1010123, 1010124, 1010125, 1010126,
    1010127, 1010128, 1010129, 1010130, 1010131, 1010132, 1010133, 1010134, 1010135, 1010136,
    1010137, 1010138, 1010139, 1010140, 1010141, 1010142, 1010143, 1010144, 1010145, 1010146,
    1010147, 1010148, 1010149, 1010150, 1010151, 1010152, 1010153, 1010154, 1010155, 1010156,
    1010157, 1010158, 1010159, 1010160, 1010161, 1010162, 1010163, 1010164, 1010165, 1010166,
    1010167, 1110001, 1110002, 1110003, 1110004, 1110005, 1110006, 1110101, 1110102, 1110103,
    1110104, 1110105, 1110106, 1110107, 1110108, 1110109, 1110110, 1110111, 1010005, 1010006,
    1010007, 1010008, 1010168, 1010169, 1010170, 1010171, 1010172, 1010173, 1010174, 1010175,
    1010176, 1010177, 1010178, 1010179, 1010180, 1010181, 1010182, 1010183, 1010184, 1010185,
    1010186, 1010187, 1110007, 1110008, 1110009, 1110112, 1110113, 1110114, 1110115, 1110116
};

void find_file_ids(int* ids, const char* import_path) {
    char file_name[50];
    for (int i = 0; i < 20; i++) {
        sprintf(file_name, "%s/q%07d.arc", import_path, challenge_quest_ids[i]);
        if (access(file_name, R_OK) == -1)
            ids[i] = 0;
        else
            ids[i] = challenge_quest_ids[i];
    }
    for (int i = 0; i < 120; i++) {
        sprintf(file_name, "%s/q%07d.arc", import_path, event_quest_ids[i]);
        if (access(file_name, R_OK) == -1)
            ids[20 + i] = 0;
        else
            ids[20 + i] = event_quest_ids[i];
    }
}

void get_ids(int* bits, int* ids) {
    for (int i = 0; i < 140; i++) {
        if (((bits[0] >> i) & 1) == 0)
            ids[i] = 0;
        else
            ids[i] = challenge_quest_ids[i];
    }
    for (int i = 0; i < 120; i++) {
        if (((bits[(i/32)+1] >> (i%32)) & 1) == 0)
            ids[20 + i] = 0;
        else
            ids[20 + i] = event_quest_ids[i];
    }
}

void get_bits(int* bits, int* ids) {
    for (int i = 0; i < 5; i++)
        bits[i] = 0;
    for (int i = 0; i < 20; i++) {
        if (ids[i] != 0)
            bits[0] |= 1 << i;
    }
    for (int i = 0; i < 120; i++) {
        if (ids[20 + i] != 0)
            bits[(i/32)+1] |= 1 << (i%32);
    }
}

void extract(Handle system, const char* export_path, int large) {
    ui_info_add("Exporting quests ...\n");
    int section_offset;
    if (FSFILE_Read(system, NULL, 0xc, &section_offset, 4) != 0) {
        ui_info_add("Failed.\n");
        ui_pause("Error: Unable to read DLC section offset");
        return;
    }
    int installed_bits[5];
    if (FSFILE_Read(system, NULL, section_offset + INSTALLED_OFFSET, installed_bits, 20) != 0) {
        ui_info_add("Failed.\n");
        ui_pause("Error: Unable to read installed quests info");
        return;
    }
    int installed_ids[140];
    get_ids(installed_bits, installed_ids);
    int quest_offset = section_offset + QUEST_OFFSET;
    int quest_info[2];
    char quest_data[QUEST_SIZE_LARGE];
    char file_name[0x100];
    FILE* export;
    for (int i = 0; i < 140; i++) {
        // silent error
        if (FSFILE_Read(system, NULL, quest_offset, quest_info, 8) != 0) {
            quest_offset += large ? QUEST_SIZE_LARGE : QUEST_SIZE_SMALL;
            continue;
        }
        int skip;
        for (skip = 0; skip < 140; skip++) {
            if (quest_info[0] == installed_ids[skip])
                break;
        }
        // silent error
        if ((quest_info[0] != 0) && (skip != 140) && (FSFILE_Read(system, NULL, quest_offset + 8, quest_data, quest_info[1]) == 0)) {
            sprintf(file_name, "%s/q%07d.arc", export_path, quest_info[0]); // snprintf
            ui_info_add("  ");
            ui_info_add(strrchr(file_name, '/') - 3); // ... hacky
            ui_info_add(" ... ");
            export = fopen(file_name, "wb");
            if (export != NULL) {
                if (fwrite(quest_data, 1, quest_info[1], export) == quest_info[1])
                    ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
                else
                    ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
                fclose(export);
            } else {
                ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
            }
        }
        quest_offset += large ? QUEST_SIZE_LARGE : QUEST_SIZE_SMALL;
    }
    ui_info_add("Complete.\n");
}

void inject(Handle system, const char* import_path, int large) {
    ui_info_add("Importing quests ...\n");
    int section_offset;
    if (FSFILE_Read(system, NULL, 0xc, &section_offset, 4) != 0) {
        ui_info_add("Failed.\n");
        ui_pause("Error: Unable to read DLC section offset");
        return;
    }
    int installed_bits[5];
    if (FSFILE_Read(system, NULL, section_offset + INSTALLED_OFFSET, installed_bits, 20) != 0) {
        ui_info_add("Failed.\n");
        ui_pause("Error: Unable to read installed quests info");
        return;
    }
    int installed_ids[140];
    get_ids(installed_bits, installed_ids);
    int file_ids[140];
    find_file_ids(file_ids, import_path);
    int quest_offset = section_offset + QUEST_OFFSET;
    int quest_info[2];
    char quest_data[QUEST_SIZE_LARGE];
    char file_name[0x100];
    FILE* import;
    for (int i = 0; i < 140; i++) {
        // silent error
        if (FSFILE_Read(system, NULL, quest_offset, quest_info, 4) != 0) {
            quest_offset += large ? QUEST_SIZE_LARGE : QUEST_SIZE_SMALL;
            continue;
        }
        for (int j = 0; j < 140; j++) {
            int check = quest_info[0] != 0 && file_ids[j] == quest_info[0];
            check = check || (quest_info[0] == 0 && file_ids[j] != 0 && i < 120 && ((file_ids[j] > 1010000 && file_ids[j] < 1020000) || (file_ids[j] > 1110000 && file_ids[j] < 1120000)));
            check = check || (quest_info[0] == 0 && file_ids[j] != 0 && i >= 120 && ((file_ids[j] > 1020000 && file_ids[j] < 1110000) || file_ids[j] > 1120000));
            if (check) {
                sprintf(file_name, "%s/q%07d.arc", import_path, file_ids[j]); // snprintf
                ui_info_add("  ");
                ui_info_add(strrchr(file_name, '/') - 3); // ... hacky
                ui_info_add(" ... ");
                import = fopen(file_name, "rb");
                if (import != NULL) {
                    quest_info[1] = fread(quest_data, 1, QUEST_SIZE_LARGE, import);
                    if (ferror(import) || (quest_info[1] + 8) > (large ? QUEST_SIZE_LARGE : QUEST_SIZE_SMALL)) {
                        fclose(import);
                        file_ids[j] = 0;
                        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
                        break;
                    }
                    fclose(import);
                    quest_info[0] = file_ids[j];
                    if (FSFILE_Write(system, NULL, quest_offset, quest_info, 8, FS_WRITE_FLUSH) != 0) {
                        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
                    } else if (FSFILE_Write(system, NULL, quest_offset + 8, quest_data, quest_info[1], FS_WRITE_FLUSH) != 0) {
                        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
                    } else {
                        installed_ids[j] = file_ids[j];
                        ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
                    }
                } else {
                    ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
                }
                file_ids[j] = 0;
                break;
            }
        }
        quest_offset += large ? QUEST_SIZE_LARGE : QUEST_SIZE_SMALL;
    }
    get_bits(installed_bits, installed_ids);
    if (FSFILE_Write(system, NULL, section_offset + INSTALLED_OFFSET, installed_bits, 20, FS_WRITE_FLUSH) != 0) {
        ui_info_add("Failed.\n");
        ui_pause("Error: Unable to write installed quests info");
        return;
    }
    ui_info_add("Complete.\n");
}

int clear(Handle system) {
    ui_info_add("Deleting quests ... ");
    int section_offset;
    if (FSFILE_Read(system, NULL, 0xc, &section_offset, 4) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        return 0;
    }
    int installed_bits[] = {0, 0, 0, 0, 0};
    if (FSFILE_Write(system, NULL, section_offset + INSTALLED_OFFSET, installed_bits, 20, FS_WRITE_FLUSH) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        return 0;
    }
    ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
    return 1;
}

void export_quests() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to export quests from...", "Selected game...", &extdata, &system, 0);
    if (game == -1)
        return;
    if (game == 0 && create_path("/3ds/mhx_data_manager/quest/jpn"))
        extract(system, "/3ds/mhx_data_manager/quest/jpn", 0);
    else if (game == 1 && create_path("/3ds/mhx_data_manager/quest/eur"))
        extract(system, "/3ds/mhx_data_manager/quest/eur", 1);
    else if (game == 2 && create_path("/3ds/mhx_data_manager/quest/usa"))
        extract(system, "/3ds/mhx_data_manager/quest/usa", 1);
    else
        ui_pause("Error: Unable to create output path");
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
}

void import_quests() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to import quests into...", "Selected game...", &extdata, &system, 1);
    if ((game == -1) || !ui_confirm("Any quest in your save file with the same ID as\nan imported quest will be overwritten. Are you\nsure you want to import quests into this game's\nsave file?\n\n(See bottom screen for selected game)"))
        return;
    if (game == 0)
        inject(system, "/3ds/mhx_data_manager/quest/jpn", 0);
    else if (game == 1)
        inject(system, "/3ds/mhx_data_manager/quest/eur", 1);
    else if (game == 2)
        inject(system, "/3ds/mhx_data_manager/quest/usa", 1);
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
}

void delete_quests() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to delete quests from...", "Selected game...", &extdata, &system, 1);
    if (game == -1)
        return;
    if (ui_confirm("Are you sure you want to delete all quests from\nthis game's save file?\n\n(See bottom screen for selected game)"))
        clear(system);
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
}

