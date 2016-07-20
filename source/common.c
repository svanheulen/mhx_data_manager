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
#include "ui.h"

int select_game(const char* description, const char* info, FS_Archive* extdata, Handle* system, int allow_write) {
    ui_menu_entry game_menu[] = {
        {"Monster Hunter X (JPN)", 1},
        {"Monster Hunter Generations (EUR)", 1},
        {"Monster Hunter Generations (USA)", 1},
    };
    int game = ui_menu(description, game_menu, 3);
    if (game == -1) {
        ui_pause("Process canceled");
        return -1;
    }
    int path_data[3] = {MEDIATYPE_SD, 0x1554, 0};
    FS_Path path = {PATH_BINARY, 0xC, (char*) path_data};
    if (game == 1)
        path_data[1] = 0x185b;
    else if (game == 2)
        path_data[1] = 0x1870;
    if (FSUSER_OpenArchive(extdata, ARCHIVE_EXTDATA, path) != 0) {
        ui_pause("Error: Unable to open extdata");
        return -1;
    }
    if (FSUSER_OpenFile(system, *extdata, fsMakePath(PATH_ASCII, "/system"), allow_write ? FS_OPEN_WRITE : FS_OPEN_READ, 0) != 0) {
        FSUSER_CloseArchive(*extdata);
        ui_pause("Error: Unable to open extdata:/system");
        return -1;
    }
    ui_info_add(info);
    ui_info_add("\n");
    ui_info_add("  ");
    ui_info_add(game_menu[game].text);
    ui_info_add("\n");
    return game;
}

