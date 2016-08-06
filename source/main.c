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
#include "character.h"
#include "quest.h"
#include "save.h"
#include "dlc.h"
#include "ui.h"

int main(int argc, char* argv[]) {
    gfxInitDefault();
    ui_init();
    ui_menu_entry main_menu[] = {
        {"Copy Character", 1},
        {"Delete Character", 1},
        {"Export Quests", 1},
        {"Import Quests", 1},
        {"Delete Quests", 1},
        {"Backup Save", 1},
        {"Restore Save", 1},
        {"Get Encryption Keys", 1},
    };
    while (aptMainLoop()) {
        int task = ui_menu("\x1b[32;1mMonster Hunter X Data Manager (v2.1.1)\n\n\x1b[31;1mAlways keep a good backup of your save file!\x1b[0m\n\nSelect a task...", main_menu, 8);
        if (task == 0)
            copy_character();
        else if (task == 1)
            delete_character();
        else if (task == 2)
            export_quests();
        else if (task == 3)
            import_quests();
        else if (task == 4)
            delete_quests();
        else if (task == 5)
            backup_save();
        else if (task == 6)
            restore_save();
        else if (task == 7)
            get_encryption_keys();
        else
            break;
    }
    gfxExit();
    return 0;
}

