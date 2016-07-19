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
#include "common.h"
#include "ui.h"

#define SLOT_SIZE 0xEAD6E

int read_slot(Handle system, int slot, char* slot_data) {
    int slot_offset;
    if (FSFILE_Read(system, NULL, 0x10 + slot * 4, &slot_offset, 4) != 0) {
        ui_pause("Error: Unable to read character data offset");
        return 0;
    }
    if (FSFILE_Read(system, NULL, slot_offset, slot_data, SLOT_SIZE) != 0) {
        ui_pause("Error: Unable to read character data");
        return 0;
    }
    return 1;
}

int write_slot(Handle system, int slot, char* slot_data) {
    int slot_offset;
    if (FSFILE_Read(system, NULL, 0x10 + slot * 4, &slot_offset, 4) != 0) {
        ui_pause("Error: Unable to read character data offset");
        return 0;
    }
    if (FSFILE_Write(system, NULL, slot_offset, slot_data, SLOT_SIZE, FS_WRITE_FLUSH) != 0) {
        ui_pause("Error: Unable to write character data");
        return 0;
    }
    char one = 1;
    if (FSFILE_Write(system, NULL, 4 + slot, &one, 1, FS_WRITE_FLUSH) != 0) {
        ui_pause("Error: Unable to write character slot info");
        return 0;
    }
    ui_info_add("Character data successfully writen.\n");
    return 1;
}

int delete_slot(Handle system, int slot) {
    char zero = 0;
    if (FSFILE_Write(system, NULL, 4 + slot, &zero, 1, FS_WRITE_FLUSH) != 0) {
        ui_pause("Error: Unable to write character slot info");
        return 0;
    }
    ui_info_add("Character data successfully deleted.");
    return 1;
}

int select_slot(const char* description, const char* info, Handle system, int enable_all) {
    int slot_info;
    if (FSFILE_Read(system, NULL, 4, &slot_info, 4) != 0) {
        ui_pause("Error: Unable to read character slot info");
        return -1;
    }
    ui_menu_entry slot_menu[] = {
        {"Character Slot 1", ((slot_info & 0x0000ff) | enable_all) ? 1 : 0},
        {"Character Slot 2", ((slot_info & 0x00ff00) | enable_all) ? 1 : 0},
        {"Character Slot 3", ((slot_info & 0xff0000) | enable_all) ? 1 : 0},
    };
    int slot = ui_menu(description, slot_menu, 3);
    if (slot == -1) {
        ui_pause("Process canceled");
    } else {
        ui_info_add(info);
        ui_info_add("\n");
        ui_info_add("  ");
        ui_info_add(slot_menu[slot].text);
        ui_info_add("\n");
    }
    return slot;
}

void copy_character() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to get your character from...", "Selected input game...", &extdata, &system, 0);
    if (game == -1)
        return;
    int slot = select_slot("Select the slot to get your character from...", "Selected input character slot...", system, 0);
    if (slot == -1) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        return;
    }
    char* slot_data = malloc(SLOT_SIZE); // error check
    if (read_slot(system, slot, slot_data)) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
    } else {
        free(slot_data);
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        return;
    }
    game = select_game("Select the game to put your character into...", "Selected output game...", &extdata, &system, 1);
    if (game == -1) {
        free(slot_data);
        return;
    }
    slot = select_slot("Select the slot to put your character into...", "Selected output character slot...", system, 1);
    if (slot == -1) {
        free(slot_data);
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        return;
    }
    write_slot(system, slot, slot_data);
    free(slot_data);
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
}

void delete_character() {
    FS_Archive extdata;
    Handle system;
    ui_info_clear();
    int game = select_game("Select the game to delete your character from...", "Selected game...", &extdata, &system, 1);
    if (game == -1)
        return;
    int slot = select_slot("Select the slot to delete your character from...", "Selected character slot...", system, 0);
    if (slot == -1) {
        FSFILE_Close(system);
        FSUSER_CloseArchive(extdata);
        return;
    }
    if (ui_confirm("Are you sure you want to delete this character?"))
        delete_slot(system, slot);
    FSFILE_Close(system);
    FSUSER_CloseArchive(extdata);
}

