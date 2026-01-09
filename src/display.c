#include <ctype.h>
#include <display.h>
#include <stdio.h>
#include <string.h>

// Too long to write printf("\n") entirely
#define ENDL() printf("\n")

const char UPPER[26][FONT_HEIGHT][FONT_WIDTH] = {
    {" $$$ ", "$   $", "$$$$$", "$   $", "$   $"},
    {"$$$$ ", "$   $", "$$$$ ", "$   $", "$$$$ "},
    {" $$$ ", "$   $", "$    ", "$   $", " $$$ "},
    {"$$$$ ", "$   $", "$   $", "$   $", "$$$$ "},
    {"$$$$$", "$    ", "$$$$ ", "$    ", "$$$$$"},
    {"$$$$$", "$    ", "$$$$ ", "$    ", "$    "},
    {" $$$ ", "$    ", "$  $$", "$   $", " $$$ "},
    {"$   $", "$   $", "$$$$$", "$   $", "$   $"},
    {" $$$ ", "  $  ", "  $  ", "  $  ", " $$$ "},
    {" $$$$", "    $", "    $", "$   $", " $$$ "},
    {"$   $", "$  $ ", "$$$  ", "$  $ ", "$   $"},
    {"$    ", "$    ", "$    ", "$    ", "$$$$$"},
    {"$   $", "$$ $$", "$ $ $", "$   $", "$   $"},
    {"$   $", "$$  $", "$ $ $", "$  $$", "$   $"},
    {" $$$ ", "$   $", "$   $", "$   $", " $$$ "},
    {"$$$$ ", "$   $", "$$$$ ", "$    ", "$    "},
    {" $$$ ", "$   $", "$   $", "$  $ ", " $$ $"},
    {"$$$$ ", "$   $", "$$$$ ", "$  $ ", "$   $"},
    {" $$$$", "$    ", " $$$ ", "    $", "$$$$ "},
    {"$$$$$", "  $  ", "  $  ", "  $  ", "  $  "},
    {"$   $", "$   $", "$   $", "$   $", " $$$ "},
    {"$   $", "$   $", "$   $", " $ $ ", "  $  "},
    {"$ $ $", "$ $ $", "$ $ $", "$ $ $", " $$$ "},
    {"$   $", " $ $ ", "  $  ", " $ $ ", "$   $"},
    {"$   $", " $ $ ", "  $  ", "  $  ", "  $  "},
    {"$$$$$", "   $ ", "  $  ", " $   ", "$$$$$"}};

static void top_line(int length)
{
  printf("\n%s", LIGHT_DOWN_RIGHT);
  draw_h_line(length - 2);
  printf("%s\n", LIGHT_DOWN_LEFT);
}

static void bottom_line(int length)
{
  printf("%s", LIGHT_HORIZONTAL);
  draw_h_line(length - 2);
  printf("%s\n", LIGHT_UP_LEFT);
}

/*
static int sum(int *arr, int len)
{
  int total = 0;
  for (int i = 0; i < len; i++)
  {
    total += arr[i];
  }
  return total;
}
*/

void table_top_line(int nb_col, const int *lengths)
{
  // Top left corner
  printf("%s", LIGHT_DOWN_RIGHT);
  // int width = sum(lengths, nb_col) + nb_col + 1;
  int last_col = nb_col - 1;
  for (int i = 0; i < nb_col; i++)
  {
    draw_h_line(lengths[i]);
    if (!(i == last_col))
    {
      printf("%s", LIGHT_DOWN_HORIZONTAL);
    }
  }
  // Top right corner
  printf("%s\n", LIGHT_DOWN_LEFT);
}

void table_bottom_line(int nb_col, const int *lengths)
{
  // Bottom left corner
  printf("%s", LIGHT_UP_RIGHT);
  int last_col = nb_col - 1;
  for (int i = 0; i < nb_col; i++)
  {
    draw_h_line(lengths[i]);
    if (!(i == last_col))
    {
      printf("%s", LIGHT_UP_HORIZONTAL);
    }
  }
  // Bottom right corner
  printf("%s\n", LIGHT_UP_LEFT);
}

void separator(int nb_col, const int *lengths)
{
  // Left T
  printf("%s", LIGHT_VERTICAL_RIGHT);
  int last_col = nb_col - 1;
  for (int i = 0; i < nb_col; i++)
  {
    draw_h_line(lengths[i]);
    if (!(i == last_col))
    {
      printf("%s", LIGHT_VERTICAL_HORIZONTAL);
    }
  }
  // Right T
  printf("%s\n", LIGHT_VERTICAL_LEFT);
}

void table_middle_line(int nb_values, const char **values, const int *lengths)
{
  // Left border
  printf("%s", LIGHT_VERTICAL);
  int last_col = nb_values - 1;
  for (int i = 0; i < nb_values; i++)
  {
    padding(lengths[i] - strlen(values[i]) - 1);
    printf("%s ", values[i]);
    if (i != last_col)
    {
      printf("%s", LIGHT_VERTICAL);
    }
  }
  // Right border and new line
  printf("%s\n", LIGHT_VERTICAL);
}

void get_term_size(int *height, int *width)
{
  struct winsize w;
  /* Ask the size of stdout (the terminal) to the system */
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  *height = w.ws_row;
  *width = w.ws_col;
}

void title(const char *title)
{
  ENDL();
  // For each line of the font height
  for (int i = 0; i < FONT_HEIGHT; i++)
  {
    for (int j = 0; title[j] != '\0'; j++)
    {
      if (isalpha(title[j]))
      {
        char c = toupper(title[j]);
        printf("%s  ", UPPER[c - 'A'][i]);
      }
      else
      {
        printf("       ");
      }
    }
    ENDL();
  }
}

void draw_h_line(int length)
{
  for (int i = 0; i < length; i++)
  {
    printf("%s", LIGHT_HORIZONTAL);
  }
}

void draw_v_line(int length)
{
  for (int i = 0; i < length; i++)
  {
    printf("%s\n", LIGHT_VERTICAL);
  }
}

void padding(int length)
{
  for (int i = 0; i < length; i++)
  {
    printf(" ");
  }
}

void frame(const char *title)
{
  int height, width;
  get_term_size(&height, &width);
  int title_len = strlen(title);
  int padding_len = (width - 2 - title_len) / 2;
  ENDL();
  // Top line
  top_line(width);
  // Middle
  printf("%s", LIGHT_VERTICAL);
  padding(padding_len);
  printf("%s", title);
  padding(padding_len);
  printf("%s", LIGHT_VERTICAL);
  // Bottom line
  bottom_line(width);
  ENDL();
}

void header(const char *title)
{
  int height, width;
  get_term_size(&height, &width);
  int title_len = 7 * strlen(title) - 1;
  int padding_len = (width - 2 - title_len) / 2;
  // if padding_len is negative, it's gonna spread on multiple lines which won't
  // be nice
  ENDL();
  // Top line
  top_line(width);
  // Middle
  printf("%s", LIGHT_VERTICAL);
  padding(width - 2);
  printf("%s", LIGHT_VERTICAL);
  for (int i = 0; i < FONT_HEIGHT; i++)
  {
    printf("%s", LIGHT_VERTICAL);
    padding(padding_len);
    for (int j = 0; title[j] != '\0'; j++)
    {
      if (isalpha(title[j]))
      {
        printf("%s  ", UPPER[toupper(title[j]) - 'A'][i]);
      }
      else
      {
        padding(6);
      }
    }
    padding(width - 2);
    printf("%s\n", LIGHT_VERTICAL);
  }
  printf("%s", LIGHT_VERTICAL);
  padding(width);
  printf("%s", LIGHT_VERTICAL);
  // Bottom line
  bottom_line(width);
  ENDL();
}
/*
int main()
{
  title("Hello");
  ENDL();
  frame("Hello");
  header("Hello");
  return 0;
}
*/
