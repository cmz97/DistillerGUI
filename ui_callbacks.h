#ifndef UI_CALLBACKS_H
#define UI_CALLBACKS_H

#include <stdbool.h>

// Declare the variables as extern
extern bool ai_is_processing;
extern bool got_question;

// Function declarations
void handle_enter_key(void);
void handle_stream_end(void);
void ui_update_thinking_text(const char *text);
void ui_update_answer_text(const char *text);

#endif // UI_CALLBACKS_H 