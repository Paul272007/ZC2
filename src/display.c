#include <ctype.h>
#include <display.h>
#include <stdio.h>
#include <string.h>

// Too long to write printf("\n") entirely
#define ENDL() printf("\n")

Table *newTable(int n_rows, int n_cols, int8_t hasRowHeaders,
                int8_t hasColHeaders)
{
  if (n_cols < 1 || n_rows < 1 || (n_cols < 2 && hasColHeaders) ||
      (n_rows < 2 && hasRowHeaders))
    return NULL;
  Table *ptr = malloc(sizeof(Table));
  if (ptr == NULL)
    return NULL;
  // clang-format off
  /*
   * 1st bit : 1 if the first row is header
   * 2nd bit : 1 if the first column is header
   * 3rd bit : 1 if the rows are double lines, single lines otherwise
   * 4th bit : 1 if the columns are double lines, single lines otherwise
   * 5th bit : 1 if the row header/body separator is a double line, single line otherwise
   * 6th bit : 1 if the column header/body separator is a double line, single line otherwise
   * 7th bit : 1 if the row table borders are double lines, single lines otherwise
   * 8th bit : 1 if the column table borders are double lines, single lines otherwise
   */
  // clang-format on
  ptr->flags = 0b00000000;
  ptr->n_cols = n_cols;
  ptr->n_rows = n_rows;
  // Allocate array the width of each column
  ptr->max_widths = malloc(ptr->n_cols * sizeof(int));
  // Allocate lines of content
  ptr->content = malloc(ptr->n_rows * sizeof(char **));
  ptr->chars = malloc(sizeof(TableChars));
  ptr->lengths = malloc(ptr->n_rows * sizeof(int *));
  // Allocate columns
  for (int i = 0; i < n_rows; i++)
  {
    ptr->content[i] = malloc(ptr->n_cols * sizeof(char *));
    ptr->lengths[i] = malloc(ptr->n_cols * sizeof(int));
    for (int j = 0; j < n_cols; j++)
    {
      // We don't know the size of the content yet so we just wait for it to
      // arrive
      ptr->content[i][j] = NULL;
    }
  }
  ptr->current_line = 0;
  switch (hasRowHeaders)
  {
  case 0:
    ptr->flags &= ~(1 << 0);
    break;
  case 1:
    ptr->flags |= (1 << 0);
    break;
  default:
    break;
  }
  switch (hasColHeaders)
  {
  case 0:
    ptr->flags &= ~(1 << 1);
    break;
  case 1:
    ptr->flags |= (1 << 1);
  default:
    break;
  }
  return ptr;
}

void setTableThickness(Table *ptr, int8_t rowThickness, int8_t colThickness,
                       int8_t rowSeparatorThickness,
                       int8_t colSeparatorThickness, int8_t rowBorderThickness,
                       int8_t colBorderThickness)
{
  if (rowThickness == 1)
    ptr->flags &= ~(1 << 2);
  else if (rowThickness == 2)
    ptr->flags |= (1 << 2);

  if (colThickness == 1)
    ptr->flags &= ~(1 << 3);
  else if (colThickness == 2)
    ptr->flags |= (1 << 3);

  if (rowSeparatorThickness == 1)
    ptr->flags &= ~(1 << 4);
  else if (rowSeparatorThickness == 2)
    ptr->flags |= (1 << 4);

  if (colSeparatorThickness == 1)
    ptr->flags &= ~(1 << 5);
  else if (colSeparatorThickness == 2)
    ptr->flags |= (1 << 5);

  if (rowBorderThickness == 1)
    ptr->flags &= ~(1 << 6);
  else if (rowBorderThickness == 2)
    ptr->flags |= (1 << 6);

  if (colBorderThickness == 1)
    ptr->flags &= ~(1 << 7);
  else if (colBorderThickness == 2)
    ptr->flags |= (1 << 7);
}

void setTableContent(Table *ptr, char ***content)
{
  for (size_t i = 0; i < ptr->n_rows; i++)
  {
    for (size_t j = 0; j < ptr->n_cols; j++)
    {
      if (ptr->content[i][j])
        free(ptr->content[i][j]);
      if (content[i][j])
        ptr->content[i][j] = strdup(content[i][j]);
    }
  }
}

void freeTableChars(TableChars *ptr)
{
  free(ptr->borderCol);
  free(ptr->borderRow);
  free(ptr->bottomLeftCorner);
  free(ptr->bottomRightCorner);
  free(ptr->topLeftCorner);
  free(ptr->topRightCorner);
  free(ptr->leftT);
  free(ptr->leftSepT);
  free(ptr->rightT);
  free(ptr->rightSepT);
  free(ptr->topT);
  free(ptr->topSepT);
  free(ptr->bottomT);
  free(ptr->bottomSepT);
  free(ptr->col);
  free(ptr->cross);
  free(ptr->row);
  free(ptr);
}

void deleteTable(Table *ptr)
{
  if (!ptr)
    return;
  free(ptr->max_widths);
  if (ptr->chars)
    freeTableChars(ptr->chars);
  for (int i = 0; i < ptr->n_rows; i++)
  {
    for (int j = 0; j < ptr->n_cols; j++)
    {
      free(ptr->content[i][j]);
    }
    free(ptr->lengths[i]);
    free(ptr->content[i]);
  }
  free(ptr->content);
  free(ptr);
}

/**
 * @brief For each column, get the size of the widest/longest element
 */
static void getMaxWidths(Table *ptr)
{
  for (int j = 0; j < ptr->n_cols; j++)
  {
    int max_col = 0;
    for (int i = 0; i < ptr->n_rows; i++)
    {
      int length = strlen(ptr->content[i][j]);
      ptr->lengths[i][j] = length;
      if (length > max_col)
      {
        max_col = length;
      }
    }
    ptr->max_widths[j] = max_col + 2;
  }
}

static void getTableChars(Table *ptr)
{
  int8_t hasRowHeaders = (ptr->flags << 0) & 1;
  int8_t hasColHeaders = (ptr->flags << 1) & 1;
  int8_t rowThickness = (ptr->flags << 2) & 1;
  int8_t colThickness = (ptr->flags << 3) & 1;
  int8_t rowSeparatorThickness = (ptr->flags << 4) & 1;
  int8_t colSeparatorThickness = (ptr->flags << 5) & 1;
  int8_t rowBorderThickness = (ptr->flags << 6) & 1;
  int8_t colBorderThickness = (ptr->flags << 7) & 1;
  char *topLeftCorner, *topRightCorner, *topT, *bottomT, *topSepT, *bottomSepT,
      *row, *col, *rowBorder, *colBorder, *bottomLeftCorner, *bottomRightCorner,
      *leftT, *leftSepT, *rightT, *rightSepT, *cross, *sepCross;
  if (rowBorderThickness)
  {
    rowBorder = DOUBLE_HORIZONTAL;
    if (colBorderThickness)
    {
      topLeftCorner = DOUBLE_DOWN_RIGHT;
      topRightCorner = DOUBLE_DOWN_LEFT;
      bottomLeftCorner = DOUBLE_UP_RIGHT;
      bottomRightCorner = DOUBLE_UP_LEFT;
    }
    else
    {
      topLeftCorner = DOWN_SINGLE_RIGHT_DOUBLE;
      topRightCorner = DOWN_SINGLE_LEFT_DOUBLE;
      bottomLeftCorner = UP_SINGLE_RIGHT_DOUBLE;
      bottomRightCorner = UP_SINGLE_LEFT_DOUBLE;
    }
    if (colThickness)
    {
      topT = DOUBLE_DOWN_HORIZONTAL;
      bottomT = DOUBLE_UP_HORIZONTAL;
    }
    else
    {
      topT = DOWN_SINGLE_HORIZONTAL_DOUBLE;
      bottomT = UP_SINGLE_HORIZONTAL_DOUBLE;
    }
    if (colSeparatorThickness)
    {
      topSepT = DOUBLE_DOWN_HORIZONTAL;
      bottomSepT = DOUBLE_UP_HORIZONTAL;
    }
    else
    {
      topSepT = DOWN_SINGLE_HORIZONTAL_DOUBLE;
      bottomSepT = UP_SINGLE_HORIZONTAL_DOUBLE;
    }
  }
  else // !rowThickness
  {
    rowBorder = LIGHT_HORIZONTAL;
    if (colBorderThickness)
    {
      topRightCorner = DOWN_DOUBLE_LEFT_SINGLE;
      topLeftCorner = DOWN_DOUBLE_RIGHT_SINGLE;
      bottomLeftCorner = UP_DOUBLE_RIGHT_SINGLE;
      bottomRightCorner = UP_DOUBLE_LEFT_SINGLE;
    }
    else
    {
      topLeftCorner = LIGHT_DOWN_RIGHT;
      topRightCorner = LIGHT_DOWN_LEFT;
      bottomLeftCorner = LIGHT_UP_RIGHT;
      bottomRightCorner = LIGHT_UP_LEFT;
    }
    if (colThickness)
    {
      topT = DOWN_DOUBLE_HORIZONTAL_SINGLE;
      bottomT = UP_DOUBLE_HORIZONTAL_SINGLE;
    }
    else
    {
      topT = LIGHT_DOWN_HORIZONTAL;
      topT = LIGHT_UP_HORIZONTAL;
    }
    if (colSeparatorThickness)
    {
      topSepT = DOWN_DOUBLE_HORIZONTAL_SINGLE;
      bottomSepT = UP_DOUBLE_HORIZONTAL_SINGLE;
    }
    else
    {
      topSepT = LIGHT_DOWN_HORIZONTAL;
      bottomSepT = LIGHT_UP_HORIZONTAL;
    }
  }
  // Values depending on the colum
  if (colBorderThickness)
  {
    colBorder = DOUBLE_VERTICAL;
    if (rowSeparatorThickness)
    {
      leftT = DOUBLE_VERTICAL_RIGHT;
      rightT = DOUBLE_VERTICAL_LEFT;
    }
    else
    {
      leftT = VERTICAL_DOUBLE_RIGHT_SINGLE;
      rightT = VERTICAL_DOUBLE_LEFT_SINGLE;
    }
  }
  else // !colBorderThickness
  {
    colBorder = LIGHT_VERTICAL;
    if (rowSeparatorThickness)
    {
      leftT = VERTICAL_SINGLE_RIGHT_DOUBLE;
      rightT = VERTICAL_SINGLE_LEFT_DOUBLE;
    }
    else
    {
      leftT = LIGHT_VERTICAL_RIGHT;
      rightT = LIGHT_VERTICAL_LEFT;
    }
  }
  // Values depending on the row separator thickness
  cross = (rowSeparatorThickness)
              ? ((colThickness) ? DOUBLE_VERTICAL_HORIZONTAL
                                : VERTICAL_SINGLE_HORIZONTAL_DOUBLE)
              : ((colThickness) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE
                                : LIGHT_VERTICAL_HORIZONTAL);
  sepCross = (rowSeparatorThickness)
                 ? ((colSeparatorThickness) ? DOUBLE_VERTICAL_HORIZONTAL
                                            : VERTICAL_SINGLE_HORIZONTAL_DOUBLE)
                 : ((colSeparatorThickness) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE
                                            : LIGHT_VERTICAL_HORIZONTAL);
  ptr->chars->topLeftCorner = strdup(topLeftCorner);
  ptr->chars->topT = strdup(topT);
  ptr->chars->topSepT = strdup(topSepT);
  ptr->chars->topRightCorner = strdup(topRightCorner);
}

void drawLine(int length, char *line)
{
  for (int i = 0; i < length; i++)
  {
    printf("%s", line);
  }
}

static void tableTopLine(Table *ptr)
{
  // Top left corner
  int8_t hasRowHeaders = (ptr->flags << 0) & 1;
  int8_t hasColHeaders = (ptr->flags << 1) & 1;
  // Top left corner
  printf("%s", ptr->chars->topLeftCorner);
  drawLine(ptr->max_widths[0], ptr->chars->borderRow);
  if (hasColHeaders)
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      printf("%s", ptr->chars->topSepT);
    }
  }
  else if (hasRowHeaders)
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      if (i == 1)
        printf("%s", ptr->chars->topSepT);
      else
        printf("%s", ptr->chars->topT);
    }
  }
  else
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      printf("%s", ptr->chars->topT);
    }
  }
  // Top right corner
  printf("%s", ptr->chars->topRightCorner);
}

void tableBottomLine(Table *ptr)
{
  int8_t hasRowHeaders = (ptr->flags << 0) & 1;
  // Bottom left corner
  printf("%s", ptr->chars->bottomLeftCorner);
  drawLine(ptr->max_widths[0], ptr->chars->borderRow);
  if (hasRowHeaders)
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      if (i == 1)
        printf("%s", ptr->chars->bottomSepT);
      else
        printf("%s", ptr->chars->bottomT);
    }
  }
  else
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      printf("%s", ptr->chars->bottomT);
    }
  }
  // Bottom right corner
  printf("%s", ptr->chars->bottomRightCorner);
}

void tableColumnHeaderSeparator(Table *ptr)
{
  int8_t hasRowHeaders = (ptr->flags << 0) & 1;
  // Left T
  printf("%s", ptr->chars->leftSepT);
  drawLine(ptr->max_widths[0], ptr->chars->sepRow);
  if (hasRowHeaders)
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      if (i == 1)
        printf("%s", ptr->chars->sepCross);
      else
        printf("%s", ptr->chars->cross);
    }
  }
  else
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      printf("%s", ptr->chars->cross);
    }
  }
  // Right T
  printf("%s", ptr->chars->rightSepT);
}

void tableSeparator(Table *ptr)
{
  int8_t hasRowHeaders = (ptr->flags << 0) & 1;
  // Left T
  printf("%s", ptr->chars->leftT);
  drawLine(ptr->max_widths[0], ptr->chars->row);
  if (hasRowHeaders)
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      if (i == 1)
        printf("%s", ptr->chars->sepCross);
      else
        printf("%s", ptr->chars->cross);
    }
  }
  else
  {
    for (int i = 1; i < ptr->n_cols; i++)
    {
      printf("%s", ptr->chars->cross);
    }
  }
  // Right T
  printf("%s", ptr->chars->rightT);
}

void tableMiddleLine(Table *ptr)
{
  // Left border
  printf("%s", ptr->chars->borderCol);

  padding(ptr->max_widths[0] - ptr->lengths[ptr->current_line][0] - 1);
  printf("%s ", ptr->content[ptr->current_line][0]);

  for (int i = 1; i < ptr->n_cols; i++)
  {
    // - 1 for the space after the word
    printf("%s", ptr->chars->col);
    padding(ptr->max_widths[i] - ptr->lengths[ptr->current_line][i] - 1);
    printf("%s ", ptr->content[ptr->current_line][i]);
  }
  // Right border and new line
  ptr->current_line++;
  printf("%s\n", ptr->chars->borderCol);
}

void drawTable(Table *ptr)
{
  int8_t hasColHeaders = (ptr->flags << 1) & 1;
  getMaxWidths(ptr);
  getTableChars(ptr);
  table_top_line(ptr->n_cols, ptr->max_widths);
  if (hasColHeaders)
  {
    tableMiddleLine(ptr);
    tableColumnHeaderSeparator(ptr);
  }
  tableMiddleLine(ptr);
  while (ptr->current_line < ptr->n_rows)
  {
    tableSeparator(ptr);
    tableMiddleLine(ptr);
  }
  table_bottom_line(ptr->n_cols, ptr->max_widths);
}

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
void table_top_line(int nb_col, const int *lengths)
{
  // Top left corner
  printf("%s", LIGHT_DOWN_RIGHT);
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
*/

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
