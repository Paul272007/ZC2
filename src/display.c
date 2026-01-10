#include <ctype.h>
#include <display.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Too long to write printf("\n") entirely
#define ENDL() printf("\n")

static int getBit(uint8_t flags, int pos) { return (flags >> pos) & 1; }

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
  // ptr->flags = 0b00000000;
  ptr->flags = 0;
  ptr->n_cols = n_cols;
  ptr->n_rows = n_rows;
  ptr->current_line = 0;
  // Allocate array the width of each column
  ptr->max_widths = malloc(ptr->n_cols * sizeof(int));
  // Allocate lines of content
  ptr->content = malloc(ptr->n_rows * sizeof(char **));
  ptr->lengths = malloc(ptr->n_rows * sizeof(int *));
  ptr->chars = malloc(sizeof(TableChars));
  if (!ptr->content || !ptr->lengths || !ptr->chars)
    return NULL;
  memset(ptr->chars, 0, sizeof(TableChars));
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
      else
        ptr->content[i][j] = strdup("");
    }
  }
}

static void safeFree(char *p)
{
  if (p)
    free(p);
}

static void freeTableChars(TableChars *ptr)
{
  if (!ptr)
    return;
  safeFree(ptr->borderCol);
  safeFree(ptr->borderRow);
  safeFree(ptr->bottomLeftCorner);
  safeFree(ptr->bottomRightCorner);
  safeFree(ptr->topLeftCorner);
  safeFree(ptr->topRightCorner);
  safeFree(ptr->leftT);
  safeFree(ptr->leftSepT);
  safeFree(ptr->rightT);
  safeFree(ptr->rightSepT);
  safeFree(ptr->topT);
  safeFree(ptr->topSepT);
  safeFree(ptr->bottomT);
  safeFree(ptr->bottomSepT);
  safeFree(ptr->col);
  safeFree(ptr->cross);
  safeFree(ptr->sepCross);
  safeFree(ptr->sepRow);
  safeFree(ptr->sepCol);
  safeFree(ptr->row);
  free(ptr);
}

void deleteTable(Table *ptr)
{
  if (!ptr)
    return;
  free(ptr->max_widths);
  freeTableChars(ptr->chars);
  for (int i = 0; i < ptr->n_rows; i++)
  {
    for (int j = 0; j < ptr->n_cols; j++)
    {
      if (ptr->content[i][j])
        free(ptr->content[i][j]);
    }
    free(ptr->lengths[i]);
    free(ptr->content[i]);
  }
  free(ptr->content);
  free(ptr->lengths);
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
      int length = (ptr->content[i][j]) ? strlen(ptr->content[i][j]) : 0;
      ptr->lengths[i][j] = length;
      if (length > max_col)
        max_col = length;
    }
    ptr->max_widths[j] = max_col + 2;
  }
}

static void getTableChars(Table *ptr)
{
  int8_t hasRowHeaders = getBit(ptr->flags, 0);
  int8_t hasColHeaders = getBit(ptr->flags, 1);
  int8_t rowThickness = getBit(ptr->flags, 2);
  int8_t colThickness = getBit(ptr->flags, 3);
  int8_t rowSeparatorThickness = getBit(ptr->flags, 4);
  int8_t colSeparatorThickness = getBit(ptr->flags, 5);
  int8_t rowBorderThickness = getBit(ptr->flags, 6);
  int8_t colBorderThickness = getBit(ptr->flags, 7);
  // Declare variables;
  char *topLeftCorner = " ", *topRightCorner = " ", *topT = " ", *bottomT = " ";
  char *topSepT = " ", *bottomSepT = " ", *row = "-", *col = "|", *sepRow = "-",
       *sepCol = "-";
  char *rowBorder = "-", *colBorder = "|", *bottomLeftCorner = " ",
       *bottomRightCorner = " ";
  char *leftT = " ", *leftSepT = " ", *rightT = " ", *rightSepT = " ";
  char *cross = "+", *sepCross = "+";
  // Initialise characters
  row = (rowThickness) ? DOUBLE_HORIZONTAL : LIGHT_HORIZONTAL;
  col = (colThickness) ? DOUBLE_VERTICAL : LIGHT_VERTICAL;
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
    // ... (Logique T haut/bas conservée mais simplifiée pour la lisibilité)
    topT =
        (colThickness) ? DOUBLE_DOWN_HORIZONTAL : DOWN_SINGLE_HORIZONTAL_DOUBLE;
    bottomT =
        (colThickness) ? DOUBLE_UP_HORIZONTAL : UP_SINGLE_HORIZONTAL_DOUBLE;
    topSepT = (colSeparatorThickness) ? DOUBLE_DOWN_HORIZONTAL
                                      : DOWN_SINGLE_HORIZONTAL_DOUBLE;
    bottomSepT = (colSeparatorThickness) ? DOUBLE_UP_HORIZONTAL
                                         : UP_SINGLE_HORIZONTAL_DOUBLE;
  }
  else
  { // !rowBorderThickness
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
    topT =
        (colThickness) ? DOWN_DOUBLE_HORIZONTAL_SINGLE : LIGHT_DOWN_HORIZONTAL;
    bottomT =
        (colThickness) ? UP_DOUBLE_HORIZONTAL_SINGLE : LIGHT_UP_HORIZONTAL;
    topSepT = (colSeparatorThickness) ? DOWN_DOUBLE_HORIZONTAL_SINGLE
                                      : LIGHT_DOWN_HORIZONTAL;
    bottomSepT = (colSeparatorThickness) ? UP_DOUBLE_HORIZONTAL_SINGLE
                                         : LIGHT_UP_HORIZONTAL;
  }

  if (colBorderThickness)
  {
    colBorder = DOUBLE_VERTICAL;
    leftT =
        (rowThickness) ? DOUBLE_VERTICAL_RIGHT : VERTICAL_DOUBLE_RIGHT_SINGLE;
    leftSepT = (rowSeparatorThickness) ? DOUBLE_VERTICAL_RIGHT
                                       : VERTICAL_DOUBLE_RIGHT_SINGLE;
    rightT =
        (rowThickness) ? DOUBLE_VERTICAL_LEFT : VERTICAL_DOUBLE_LEFT_SINGLE;
    rightSepT = (rowSeparatorThickness) ? DOUBLE_VERTICAL_LEFT
                                        : VERTICAL_DOUBLE_LEFT_SINGLE;
  }
  else
  {
    colBorder = LIGHT_VERTICAL;
    leftT =
        (rowThickness) ? VERTICAL_SINGLE_RIGHT_DOUBLE : LIGHT_VERTICAL_RIGHT;
    leftSepT = (rowSeparatorThickness) ? VERTICAL_SINGLE_RIGHT_DOUBLE
                                       : LIGHT_VERTICAL_RIGHT;
    rightT = (rowThickness) ? VERTICAL_SINGLE_LEFT_DOUBLE : LIGHT_VERTICAL_LEFT;
    rightSepT = (rowSeparatorThickness) ? VERTICAL_SINGLE_LEFT_DOUBLE
                                        : LIGHT_VERTICAL_LEFT;
  }

  // Handle crosses
  if (rowSeparatorThickness)
  {
    sepRow = DOUBLE_HORIZONTAL;
    cross = (colThickness) ? DOUBLE_VERTICAL_HORIZONTAL
                           : VERTICAL_SINGLE_HORIZONTAL_DOUBLE;
    sepCross = (colSeparatorThickness) ? DOUBLE_VERTICAL_HORIZONTAL
                                       : VERTICAL_SINGLE_HORIZONTAL_DOUBLE;
  }
  else
  {
    sepRow = LIGHT_HORIZONTAL;
    cross = (colThickness) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE
                           : LIGHT_VERTICAL_HORIZONTAL;
    sepCross = (colSeparatorThickness) ? VERTICAL_DOUBLE_HORIZONTAL_SINGLE
                                       : LIGHT_VERTICAL_HORIZONTAL;
  }
  sepCol = (colSeparatorThickness) ? DOUBLE_VERTICAL : LIGHT_VERTICAL;
  // Fill char struct
  if (ptr->chars->topLeftCorner)
    freeTableChars(ptr->chars); // Nettoyage si réappelé
  ptr->chars->topLeftCorner = strdup(topLeftCorner);
  ptr->chars->topRightCorner = strdup(topRightCorner);
  ptr->chars->bottomLeftCorner = strdup(bottomLeftCorner);
  ptr->chars->bottomRightCorner = strdup(bottomRightCorner);
  ptr->chars->leftT = strdup(leftT);
  ptr->chars->rightT = strdup(rightT);
  ptr->chars->leftSepT = strdup(leftSepT);
  ptr->chars->rightSepT = strdup(rightSepT);
  ptr->chars->topT = strdup(topT);
  ptr->chars->bottomT = strdup(bottomT);
  ptr->chars->topSepT = strdup(topSepT);
  ptr->chars->bottomSepT = strdup(bottomSepT);
  ptr->chars->row = strdup(row);
  ptr->chars->col = strdup(col);
  ptr->chars->sepRow = strdup(sepRow);
  ptr->chars->sepCol = strdup(sepCol);
  ptr->chars->borderRow = strdup(rowBorder);
  ptr->chars->borderCol = strdup(colBorder);
  ptr->chars->cross = strdup(cross);
  ptr->chars->sepCross = strdup(sepCross);
}

static void drawLine(int length, char *line)
{
  for (int i = 0; i < length; i++)
    printf("%s", line);
}

static void tableTopLine(Table *ptr)
{
  int8_t hasRowHeaders = getBit(ptr->flags, 0);

  printf("%s", ptr->chars->topLeftCorner);
  drawLine(ptr->max_widths[0], ptr->chars->borderRow);

  for (int i = 1; i < ptr->n_cols; i++)
  {
    // 1. D'abord le séparateur
    if (hasRowHeaders && i == 1)
      printf("%s", ptr->chars->topSepT);
    else
      printf("%s", ptr->chars->topT);

    // 2. Ensuite la ligne horizontale pour cette colonne
    drawLine(ptr->max_widths[i], ptr->chars->borderRow);
  }
  printf("%s\n", ptr->chars->topRightCorner);
}

static void tableBottomLine(Table *ptr)
{
  int8_t hasRowHeaders = getBit(ptr->flags, 0);

  printf("%s", ptr->chars->bottomLeftCorner);
  drawLine(ptr->max_widths[0], ptr->chars->borderRow);

  for (int i = 1; i < ptr->n_cols; i++)
  {
    if (hasRowHeaders && i == 1)
      printf("%s", ptr->chars->bottomSepT);
    else
      printf("%s", ptr->chars->bottomT);

    drawLine(ptr->max_widths[i], ptr->chars->borderRow);
  }
  printf("%s\n", ptr->chars->bottomRightCorner);
}

static void tableColumnHeaderSeparator(Table *ptr)
{
  int8_t hasRowHeaders = getBit(ptr->flags, 0);

  printf("%s", ptr->chars->leftSepT);
  drawLine(ptr->max_widths[0], ptr->chars->sepRow);

  for (int i = 1; i < ptr->n_cols; i++)
  {
    if (hasRowHeaders && i == 1)
      printf("%s", ptr->chars->sepCross);
    else
      printf("%s", ptr->chars->cross);

    drawLine(ptr->max_widths[i], ptr->chars->sepRow);
  }
  printf("%s\n", ptr->chars->rightSepT);
}

static void tableSeparator(Table *ptr)
{
  int8_t hasRowHeaders = getBit(ptr->flags, 0);

  printf("%s", ptr->chars->leftT);
  drawLine(ptr->max_widths[0], ptr->chars->row);

  for (int i = 1; i < ptr->n_cols; i++)
  {
    if (hasRowHeaders && i == 1)
      printf("%s", ptr->chars->sepCross);
    else
      printf("%s", ptr->chars->cross);

    drawLine(ptr->max_widths[i], ptr->chars->row);
  }
  printf("%s\n", ptr->chars->rightT);
}

static void tableMiddleLine(Table *ptr)
{
  // Bordure gauche
  printf("%s", ptr->chars->borderCol);

  // Colonne 0
  padding(ptr->max_widths[0] - ptr->lengths[ptr->current_line][0] - 1);
  printf("%s ", ptr->content[ptr->current_line][0]
                    ? ptr->content[ptr->current_line][0]
                    : "");

  // Autres colonnes
  for (int i = 1; i < ptr->n_cols; i++)
  {
    printf("%s", ptr->chars->col); // Séparateur vertical interne
    padding(ptr->max_widths[i] - ptr->lengths[ptr->current_line][i] - 1);
    printf("%s ", ptr->content[ptr->current_line][i]
                      ? ptr->content[ptr->current_line][i]
                      : "");
  }

  ptr->current_line++;
  // Bordure droite
  printf("%s\n", ptr->chars->borderCol);
}

void drawTable(Table *ptr)
{
  if (!ptr)
    return;
  int8_t hasColHeaders = getBit(ptr->flags, 1);

  getMaxWidths(ptr);
  getTableChars(ptr);

  // Correction des noms de fonctions (snake_case -> camelCase)
  tableTopLine(ptr);

  if (hasColHeaders)
  {
    tableMiddleLine(ptr);
    tableColumnHeaderSeparator(ptr);
  }

  // S'il reste des lignes à dessiner
  if (ptr->current_line < ptr->n_rows)
  {
    tableMiddleLine(ptr);
    while (ptr->current_line < ptr->n_rows)
    {
      tableSeparator(ptr);
      tableMiddleLine(ptr);
    }
  }

  tableBottomLine(ptr);
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
