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

typedef struct {
    char text[50];
    int enabled;
} ui_menu_entry;

void ui_init();
int ui_menu(const char* description, ui_menu_entry * menu, int menu_length);
int ui_confirm(const char* question);
void ui_pause(const char* message);
void ui_info_add(const char* info);
void ui_info_clear();

