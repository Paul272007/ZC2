#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>

// clang-format off

// Reset
#define COLOR_RESET "\033[0m"

// Regular Colors
#define BLACK     "\033[0;30m"
#define RED       "\033[0;31m"
#define GREEN     "\033[0;32m"
#define YELLOW    "\033[0;33m"
#define BLUE      "\033[0;34m"
#define PURPLE    "\033[0;35m"
#define CYAN      "\033[0;36m"
#define WHITE     "\033[0;37m"

// Bold Colors
#define B_BLACK   "\033[1;30m"
#define B_RED     "\033[1;31m"
#define B_GREEN   "\033[1;32m"
#define B_YELLOW  "\033[1;33m"
#define B_BLUE    "\033[1;34m"
#define B_PURPLE  "\033[1;35m"
#define B_CYAN    "\033[1;36m"
#define B_WHITE   "\033[1;37m"

// Underline Colors
#define U_BLACK   "\033[4;30m"
#define U_RED     "\033[4;31m"
#define U_GREEN   "\033[4;32m"
#define U_YELLOW  "\033[4;33m"
#define U_BLUE    "\033[4;34m"
#define U_PURPLE  "\033[4;35m"
#define U_CYAN    "\033[4;36m"
#define U_WHITE   "\033[4;37m"

// Background Colors
#define BG_BLACK  "\033[40m"
#define BG_RED    "\033[41m"
#define BG_GREEN  "\033[42m"
#define BG_YELLOW "\033[43m"
#define BG_BLUE   "\033[44m"
#define BG_PURPLE "\033[45m"
#define BG_CYAN   "\033[46m"
#define BG_WHITE  "\033[47m"

// Special characters (e.g. borders / lines)

#define LIGHT_VERTICAL_LEFT                 "\u2524"
#define VERTICAL_SINGLE_LEFT_DOUBLE         "\u2561"
#define VERTICAL_DOUBLE_LEFT_SINGLE         "\u2562"
#define DOWN_DOUBLE_LEFT_SINGLE             "\u2556"
#define DOWN_SINGLE_LEFT_DOUBLE             "\u2555"
#define DOUBLE_VERTICAL_LEFT                "\u2563"
#define DOUBLE_VERTICAL                     "\u2551"
#define DOUBLE_DOWN_LEFT                    "\u2557"
#define DOUBLE_UP_LEFT                      "\u255D"
#define UP_DOUBLE_LEFT_SINGLE               "\u255C"
#define UP_SINGLE_LEFT_DOUBLE               "\u255B"
#define LIGHT_DOWN_LEFT                     "\u2510"
#define LIGHT_UP_RIGHT                      "\u2514"
#define LIGHT_UP_HORIZONTAL                 "\u2534"
#define LIGHT_DOWN_HORIZONTAL               "\u252C"
#define LIGHT_VERTICAL_RIGHT                "\u251C"
#define LIGHT_HORIZONTAL                    "\u2500"
#define LIGHT_VERTICAL                      "\u2502"
#define LIGHT_VERTICAL_HORIZONTAL           "\u253C"
#define VERTICAL_SINGLE_RIGHT_DOUBLE        "\u255E"
#define VERTICAL_DOUBLE_RIGHT_SINGLE        "\u255F"
#define DOUBLE_UP_RIGHT                     "\u255A"
#define DOUBLE_DOWN_RIGHT                   "\u2554"
#define DOUBLE_UP_HORIZONTAL                "\u2569"
#define DOUBLE_DOWN_HORIZONTAL              "\u2566"
#define DOUBLE_VERTICAL_RIGHT               "\u2560"
#define DOUBLE_HORIZONTAL                   "\u2550"
#define DOUBLE_VERTICAL_HORIZONTAL          "\u256C"
#define UP_SINGLE_HORIZONTAL_DOUBLE         "\u2567"
#define UP_DOUBLE_HORIZONTAL_SINGLE         "\u2568"
#define DOWN_SINGLE_HORIZONTAL_DOUBLE       "\u2564"
#define DOWN_DOUBLE_HORIZONTAL_SINGLE       "\u2565"
#define UP_DOUBLE_RIGHT_SINGLE              "\u2559"
#define UP_SINGLE_RIGHT_DOUBLE              "\u2558"
#define DOWN_SINGLE_RIGHT_DOUBLE            "\u2552"
#define DOWN_DOUBLE_RIGHT_SINGLE            "\u2553"
#define VERTICAL_DOUBLE_HORIZONTAL_SINGLE   "\u256B"
#define VERTICAL_SINGLE_HORIZONTAL_DOUBLE   "\u256A"
#define LIGHT_UP_LEFT                       "\u2518"
#define LIGHT_DOWN_RIGHT                    "\u250C"

// Macros for the special font
#define FONT_HEIGHT 5
#define FONT_WIDTH 6

#ifdef __cplusplus
extern "C"
{
#endif

// The ascii art font
extern const char UPPER[26][FONT_HEIGHT][FONT_WIDTH];

typedef struct {
  char *cross;
  char *sepCross;
  char *row;
  char *col;
  char *sepRow;
  char *sepCol;
  char *borderCol;
  char *borderRow;
  char *topT;
  char *topSepT;
  char *bottomSepT;
  char *leftT;
  char *rightT;
  char *leftSepT;
  char *rightSepT;
  char *bottomT;
  char *topLeftCorner;
  char *bottomLeftCorner;
  char *topRightCorner;
  char *bottomRightCorner;
} TableChars ;

typedef struct {
  // Number of columns and lines (including headers)
  int n_cols;
  int n_rows;
  int *max_widths;
  // Indicate if the first values for columns and rows are headers
  // The body of the table
  char ***content;
  int **lengths;
  int8_t flags;
  TableChars *chars;
  int current_line;
} Table;

// Utility functions for displaying

/**
 * @brief Store the size of the terminal into height and width
 */
void get_term_size(int *height, int *width);

/**
 * @brief Print out an ascii art version of the string.
 * Doesn't handle lowercase and special characters yet.
 */
void title(const char *title);

/**
 * @brief Print out the given text centered in a frame the size of the window.
 */
void frame(const char *title);

/**
 * @brief Print out a centered title in a frame.
 * Same limitations as the title() function
 */
void header(const char *title);

/**
 * @brief Draw a horizontal line of length size.
 */
void draw_h_line(int length);

/**
 * @brief Draw a vertical line of length size.
 */
void draw_v_line(int length);

/**
 * @brief Print out length spaces
 */
void padding(int length);
void table_top_line(int nb_col, const int *lengths);
void table_bottom_line(int nb_col, const int *lengths);
void table_middle_line(int nb_values, const char **values, const int *lengths);
void separator(int nb_col, const int *lengths);

Table *newTable(int n_rows, int n_cols, int8_t hasRowHeaders, int8_t hasColHeaders);
void setTableThickness(Table *ptr, int8_t rowThickness, int8_t colThickness, int8_t rowSeparatorThickness, int8_t colSeparatorThickness, int8_t rowBorderThickness, int8_t colBorderThickness);
void setTableContent(Table *ptr, char ***content);
void deleteTable(Table *ptr);
void drawTable(Table *ptr);

#ifdef __cplusplus
}
#endif

#endif // _DISPLAY_H_
