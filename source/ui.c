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
#include "ui.h"

PrintConsole top_screen, bottom_screen;

void ui_init() {
    consoleInit(GFX_TOP, &top_screen);
    consoleInit(GFX_BOTTOM, &bottom_screen);
}

int ui_menu(const char* description, ui_menu_entry * menu, int menu_length) {
    int selected;
    for (selected = 0; selected < menu_length; selected++) {
        if (menu[selected].enabled)
            break;
    }
    int refresh = 1;
    while (aptMainLoop()) {
        hidScanInput();
        unsigned int keys = hidKeysDown();
        if (keys & (KEY_DUP|KEY_DDOWN)) {
            if (selected == menu_length)
                continue;
            int next;
            if (keys & KEY_DDOWN) {
                for (next = 1; next < menu_length + 1; next++) {
                    if (menu[(selected + next) % menu_length].enabled)
                        break;
                }
            } else {
                for (next = menu_length - 1; next >= 0; next--) {
                    if (menu[(selected + next) % menu_length].enabled)
                        break;
                }
            }
            selected = (selected + next) % menu_length;
            refresh = 1;
        } else if (keys & KEY_A) {
            consoleClear();
            svcSleepThread(100000000);
            if (selected == menu_length)
                return -1;
            return selected;
        } else if (keys & KEY_B) {
            consoleClear();
            svcSleepThread(100000000);
            return -1;
        }
        if (refresh) {
            consoleSelect(&top_screen);
            consoleClear();
            printf("%s\n\n", description);
            for (int i = 0; i < menu_length; i++) {
                if (menu[i].enabled == 0)
                    printf("\x1b[30;1m%s\x1b[0m\n", menu[i].text);
                else if (selected == i)
                    printf("\x1b[34;1m%s\x1b[0m\n", menu[i].text);
                else
                    printf("%s\n", menu[i].text);
            }
            printf("\n\n\n\nA = Select, B = Cancel");
            gfxFlushBuffers();
            gfxSwapBuffers();
            gspWaitForVBlank();
            refresh = 0;
            svcSleepThread(100000000);
        }
    }
    consoleClear();
    svcSleepThread(100000000);
    return -1;
}

int ui_confirm(const char* question) {
    consoleSelect(&top_screen);
    consoleClear();
    printf("%s\n\n\n\n\nA = Yes, B = No", question);
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
    while (aptMainLoop()) {
        hidScanInput();
        unsigned int keys = hidKeysDown();
        if (keys & KEY_A) {
            consoleClear();
            svcSleepThread(100000000);
            return 1;
        } else if (keys & KEY_B) {
            consoleClear();
            svcSleepThread(100000000);
            return 0;
        }
    }
    return 0;
}

void ui_pause(const char* message) {
    consoleSelect(&top_screen);
    consoleClear();
    printf("%s\n\n\n\n\nA = Continue", message);
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_A) {
            consoleClear();
            svcSleepThread(100000000);
            break;
        }
    }
}

void ui_info_add(const char* info) {
    consoleSelect(&bottom_screen);
    printf(info);
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
}

void ui_info_clear() {
    consoleSelect(&bottom_screen);
    consoleClear();
}

