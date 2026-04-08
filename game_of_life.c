#include <stdio.h>
#include <ncurses.h>

#define WIDTH 80
#define HEIGHT 25

/*
 * SUMMARY:
 * play - основной игровой цикл: управление (a/z/space), вызов обновления и отрисовки.
 * render - вывод поля в ncurses: рисует '#' по краям и '*' для живых клеток внутри.
 * wrap - вспомогательная функция: перебрасывает координаты с рамки на противоположный игровой край.
 * count_neighbors - обсчет 8 соседей: использует wrap, чтобы клетки видели друг друга сквозь рамку.
 * get_next_state - логика одной клетки: проверяет на рамку (всегда 0) и применяет правила B3/S23.
 * update_gen - генерация нового поля: перебирает массив, вызывая get_next_state для каждой точки.
 * copy_field - техническая функция: переносит рассчитанное состояние из буфера в основной массив.
 * init_field - загрузка: считывает начальную расстановку из файлов map_N.txt или stdin.
 * is_alive - условие остановки: проверяет, осталось ли на поле хотя бы одно живое «существо».
 * show_final_stats - финал: выводит итоги в терминал и предлагает сохранить текущую карту в файл.
 */

int play();
void render(const int field[HEIGHT][WIDTH], int generation, int speed);
int count_neighbors(const int field[HEIGHT][WIDTH], int y, int x);
void update_gen(int current_field[HEIGHT][WIDTH], int next_field[HEIGHT][WIDTH]);
void copy_field(int source[HEIGHT][WIDTH], int destination[HEIGHT][WIDTH]);
int init_field(int field[HEIGHT][WIDTH]);
int is_alive(const int field[HEIGHT][WIDTH]);

void show_final_stats(int generation, const int field[HEIGHT][WIDTH]);

int main() { play(); }

int play() {
    int field[HEIGHT][WIDTH] = {0};
    int next_field[HEIGHT][WIDTH] = {0};
    int generation = 0;
    printf("--- GAME OF LIFE ---\n1-5: Стандартные карты\n6: Загрузить последнее сохранение\nВаш выбор: ");
    if (init_field(field) == 0) {
        return 0;
    }
    initscr();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    int current_speed = 100;
    int running = 1;
    while (running == 1) {
        timeout(current_speed);
        int key = getch();
        if (key == ' ') {
            running = 0;
        }
        if (key == 'a' && current_speed > 10) {
            current_speed = current_speed - 10;
        }
        if (key == 'z' && current_speed < 1000) {
            current_speed = current_speed + 10;
        }
        update_gen(field, next_field);
        copy_field(next_field, field);
        render(field, generation, current_speed);
        generation++;
        if (is_alive(field) == 0) {
            running = 0;
        }
    }
    endwin();
    show_final_stats(generation, field);
    return 0;
}

void render(const int field[HEIGHT][WIDTH], int generation, int speed) {
    clear();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (y == 0 || y == 24 || x == 0 || x == 79) {
                mvaddch(y, x, '#');
            } else if (field[y][x] == 1) {
                mvaddch(y, x, '*');
            } else {
                mvaddch(y, x, ' ');
            }
        }
    }
    mvprintw(26, 0, "Generation: %d  |  Speed Delay: %d ms  |  Space: Exit", generation, speed);
    refresh();
}

int wrap(int pos, int max) {
    int result = pos;
    if (pos == 0) {
        result = max - 2;
    } else if (pos == max - 1) {
        result = 1;
    }
    return result;
}

int count_neighbors(const int field[HEIGHT][WIDTH], int y, int x) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (i != 0 || j != 0) {
                int ny = wrap(y + i, HEIGHT);
                int nx = wrap(x + j, WIDTH);
                if (field[ny][nx] == 1) {
                    count++;
                }
            }
        }
    }
    return count;
}

int get_next_state(int current_field[HEIGHT][WIDTH], int y, int x) {
    int res = 0;
    int is_border = (y == 0 || y == HEIGHT - 1 || x == 0 || x == WIDTH - 1);

    if (is_border == 1) {
        res = 0;
    } else {
        int neighbors = count_neighbors(current_field, y, x);
        int alive = current_field[y][x];

        if ((alive == 1 && (neighbors == 2 || neighbors == 3)) || (alive == 0 && neighbors == 3)) {
            res = 1;
        } else {
            res = 0;
        }
    }
    return res;
}

void update_gen(int current_field[HEIGHT][WIDTH], int next_field[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            next_field[y][x] = get_next_state(current_field, y, x);
        }
    }
}

void copy_field(int source[HEIGHT][WIDTH], int destination[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            destination[y][x] = source[y][x];
        }
    }
}

int init_field(int field[HEIGHT][WIDTH]) {
    int n = -1;
    int success = 1;

    if (scanf("%d", &n) != 1) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
        n = -1;
    }

    FILE *f = NULL;
    if (n == 1) {
        f = fopen("map_1.txt", "r");
    } else if (n == 2) {
        f = fopen("map_2.txt", "r");
    } else if (n == 3) {
        f = fopen("map_3.txt", "r");
    } else if (n == 4) {
        f = fopen("map_4.txt", "r");
    } else if (n == 5) {
        f = fopen("map_5.txt", "r");
    } else if (n == 6) {
        f = fopen("saved_map.txt", "r");
    }

    if (n < 0 || n > 6 || (n > 0 && f == NULL)) {
        printf("Ошибка: Некорректный выбор или файл не найден!\n");
        success = 0;
    } else {
        FILE *input;
        if (n == 0) {
            input = stdin;
        } else {
            input = f;
        }

        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (fscanf(input, "%d", &field[i][j]) != 1) {
                    field[i][j] = 0;
                }
            }
        }
    }

    if (f != NULL) {
        fclose(f);
    }
    return success;
}

int is_alive(const int field[HEIGHT][WIDTH]) {
    int alive_flag = 0;
    for (int y = 0; y < HEIGHT && alive_flag == 0; y++) {
        for (int x = 0; x < WIDTH && alive_flag == 0; x++) {
            if (field[y][x] == 1) {
                alive_flag = 1;
            }
        }
    }
    return alive_flag;
}

void show_final_stats(int generation, const int field[HEIGHT][WIDTH]) {
    printf("\n--- ИГРА ОКОНЧЕНА ---\n");
    printf("Прожито поколений: %d\n", generation);
    printf("Хотите сохранить карту? (y/n): ");
    char choice;
    getchar();
    scanf("%c", &choice);
    if (choice == 'y' || choice == 'Y') {
        FILE *f = fopen("saved_map.txt", "w");
        if (f) {
            for (int i = 0; i < HEIGHT; i++) {
                for (int j = 0; j < WIDTH; j++) {
                    fprintf(f, "%d ", field[i][j]);
                }
                fprintf(f, "\n");
            }
            fclose(f);
            printf("Карта сохранена в saved_map.txt\n");
        }
    } else {
        printf("Выход без сохранения. До встречи!\n");
    }
}