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

