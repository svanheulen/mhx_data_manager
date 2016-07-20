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
#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "ui.h"

#define SYSTEM_SIZE_SMALL 0x389B2B
#define SYSTEM_SIZE_LARGE 0x3D0C2F

void backup_save() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to backup your save file from...", "Selected game...", &extdata, &system, 0);
    if (game == -1)
        return;
    char* system_data = malloc(SYSTEM_SIZE_LARGE);
    ui_info_add("Reading save file ... ");
    if (system_data == NULL) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to allocate memory for save file");
        return;
    }
    u32 system_data_size = 0;
    if (FSFILE_Read(system, &system_data_size, 0, system_data, SYSTEM_SIZE_LARGE) != 0) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to read save file");
        return;
    }
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
    ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
    ui_info_add("Writing backup save file ... ");
    FILE* backup = NULL;
    if (game == 0)
        backup = fopen("save/jpn/system", "wb");
    else if (game == 1)
        backup = fopen("save/eur/system", "wb");
    else if (game == 2)
        backup = fopen("save/usa/system", "wb");
    if (backup == NULL) {
        free(system_data);
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to open backup save file");
        return;
    }
    if (fwrite(system_data, 1, system_data_size, backup) != system_data_size) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to write entire backup save file");
    } else {
        ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
    }
    fclose(backup);
    free(system_data);
}

void restore_save() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to restore your backup save file\nto...", "Selected game...", &extdata, &system, 1);
    if ((game == -1) || !ui_confirm("Are you sure you want to overwrite this game's\nsave file?\n\n(See bottom screen for selected game)"))
        return;
    ui_info_add("Reading backup save file ... ");
    FILE* backup = NULL;
    if (game == 0)
        backup = fopen("save/jpn/system", "rb");
    else if (game == 1)
        backup = fopen("save/eur/system", "rb");
    else if (game == 2)
        backup = fopen("save/usa/system", "rb");
    if (backup == NULL) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to open backup save file");
        return;
    }
    char* system_data = malloc(SYSTEM_SIZE_LARGE);
    if (system_data == NULL) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to allocate memory for backup save\nfile");
        return;
    }
    int system_data_size = fread(system_data, 1, SYSTEM_SIZE_LARGE, backup);
    if (ferror(backup)) {
        free(system_data);
        fclose(backup);
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to read backup save file");
    }
    fclose(backup);
    ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
    ui_info_add("Writing save file ... ");
    if (FSFILE_Write(system, NULL, 0, system_data, system_data_size, FS_WRITE_FLUSH) != 0) {
        ui_info_add("\x1b[31;1mfailure.\x1b[0m\n");
        ui_pause("Error: Unable to write save file");
    } else {
        ui_info_add("\x1b[32;1msuccess.\x1b[0m\n");
    }
    free(system_data);
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
}

