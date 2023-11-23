#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define RIGHT 2
#define LEFT 1
#define HORIZONTAL 4
#define TOP 5
#define BOTTOM 6

typedef struct {
    int rows;
    int cols;
    unsigned char *cells;
} Map;

void freeMap(Map *map) {
    free(map->cells);
}

void helpPrint() {
    printf("The program is executed in the following ways:\n"
           "./maze --help\n"
           "or\n"
           "./maze --test file.txt\n"
           "or\n"
           "./maze --rpath R C file.txt\n"
           "or\n"
           "./maze --lpath R C file.txt\n"
           "or (maybe)\n"
           "./maze --shortest R C file.txt\n");
}

void invalidPrint() {
    printf("Invalid\n");
}

void validPrint() {
    printf("Valid\n");
}

int setPathArguments(int argc, char *argv[], int *r, int *c) {
    if (argc != 5) {
        return -1;
    }
    *r = atoi(argv[2]);
    *c = atoi(argv[3]);
    if (*r <= 0 || *c <= 0) {
        return -1;
    }
    return 1;
}

int parseArgument(int argc, char *argv[], int *r, int *c) {
    if (argc < 2) {
        return -1;
    }
    if (strcmp(argv[1], "--help") == 0) {
        helpPrint();
        return 0;
    } else if (strcmp(argv[1], "--test") == 0) {
        if (argc != 3) {
            return -1;
        }
        return 1;
    } else if (strcmp(argv[1], "--rpath") == 0) {
        int isValid = setPathArguments(argc, argv, r, c);
        if (isValid == -1) {
            return -1;
        }

        return 2;

    } else if (strcmp(argv[1], "--lpath") == 0) {
        int isValid = setPathArguments(argc, argv, r, c);
        if (isValid == -1) {
            return -1;
        }
        return 3;
    } else if (strcmp(argv[1], "--shortest") == 0) {
        // bonus
        return 99;
    }

    return -1;
}

bool isBorderNumberValid(int number) {
    return (number > -1 && number < 8) ? true : false;
}

int readFile(const char *filename, Map *map) {

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return -1;
    }

    // Read rows and columns
    if (fscanf(file, "%d %d", &map->rows, &map->cols) != 2) {
        fclose(file);
        return -1;
    }

    // Allocate memory for cells
    map->cells = (unsigned char *) malloc(map->rows * map->cols * sizeof(unsigned char));

    if (map->cells == NULL) {
        return -2;
    }

    // Read cell values
    for (int i = 0; i < map->rows * map->cols; i++) {

        if (fscanf(file, "%hhu", &map->cells[i]) != 1 || !isBorderNumberValid(map->cells[i])) {
            // Unable to read cell value or invalid value
            freeMap(map);
            fclose(file);
            return -1;
        }
    }
    int extraValue;
    if (fscanf(file, "%d", &extraValue) != EOF) {
        //Extra values in the file
        freeMap(map);
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

void printMap(const Map *map) {
    printf("Rows: %d\n", map->rows);
    printf("Columns: %d\n", map->cols);

    printf("Cell values:\n");
    for (int i = 0; i < map->rows; i++) {
        for (int j = 0; j < map->cols; j++) {
            printf("%hhu ", map->cells[i * map->cols + j]);
        }
        printf("\n");
    }
}

bool areBordersValid(Map *map) {
    for (int rows = 0; rows < map->rows; ++rows) {
        for (int cols = 0; cols < map->cols; ++cols) {
            int current = map->cells[rows * map->cols + cols];
//            printf("%d ",current);

            //checking right/left
            if (cols < map->cols - 1) {
//                printf("cols %d \n", cols + 1);
                if ((current & 0x02) == 0x02) {
                    int next = map->cells[rows * map->cols + cols + 1];
                    if ((next & 0x01) != 0x01) {
//                        printf("%d %d wrong (right/left)\n", current,next);
                        return false;
                    }

                }
            }
            //checking top/bottom
            if (rows > 0 && rows < map->rows - 1 && (cols + rows) % 2 == 0) {
                if ((current & 0x04) == 0x04) {
                    int next = map->cells[(rows - 1) * map->cols + cols];
                    if ((next & 0x04) != 0x04) {
//                        printf("index %d %d \n", rows,cols);
//                        printf("%d %d wrong (top/bottom)\n", current,next);
                        return false;
                    }

                }
            }
        }
//        printf("\n");
    }
    return true;
}

bool isborder(Map *map, int r, int c, int border) {
    r--;
    c--;
    int cell = map->cells[r * map->cols + c];
//    printf("cell %d\n", cell);

    if (border == LEFT && ((cell & 0x01) != 0x01)) {
        return false;
    }
    if (border == RIGHT && ((cell & 0x02) != 0x02)) {
        return false;
    }
    if (border == HORIZONTAL && ((cell & 0x04) != 0x04)) {
        return false;
    }
    return true;
}

int stepFromLeft(Map *map, int r, int c, bool rpath) {
    if ((r + c) % 2 == 0) {
        //odd
        if (!isborder(map, r, c, LEFT)) {

            if (!isborder(map, r, c, rpath ? RIGHT : HORIZONTAL)) {
                return rpath ? RIGHT : TOP;
            }
            if (!isborder(map, r, c, rpath ? HORIZONTAL : RIGHT)) {
                return rpath ? TOP : RIGHT;
            }
            return -1;
        }
    } else {
        //even
        if (!isborder(map, r, c, LEFT)) {
            if (!isborder(map, r, c, rpath ? HORIZONTAL : RIGHT)) {
                return rpath ? BOTTOM : RIGHT;
            }
            if (!isborder(map, r, c, rpath ? RIGHT : HORIZONTAL)) {
                return rpath ? RIGHT : BOTTOM;
            }
            return -1;
        }
    }
    return -2;
}

int stepFromRight(Map *map, int r, int c, bool rpath) {

    if ((r + c) % 2 == 1) {
        //odd
        if (!isborder(map, r, c, RIGHT)) {
            if (!isborder(map, r, c, rpath ? LEFT : HORIZONTAL)) {
                return rpath ? LEFT : BOTTOM;
            }
            if (!isborder(map, r, c, rpath ? HORIZONTAL : LEFT)) {
                return rpath ? BOTTOM : LEFT;
            }
            return -1;
        }
    } else {
        //even
        if (!isborder(map, r, c, RIGHT)) {
            if (!isborder(map, r, c, rpath ? HORIZONTAL : LEFT)) {
                return rpath ? TOP : LEFT;
            }
            if (!isborder(map, r, c, rpath ? LEFT : HORIZONTAL)) {
                return rpath ? LEFT : TOP;
            }
            return -1;
        }
    }
    return -2;
}

int stepFromTop(Map *map, int r, int c, bool rpath) {
    if (!isborder(map, r, c, HORIZONTAL)) {
        if (!isborder(map, r, c, rpath ? LEFT : RIGHT)) {
            return rpath ? LEFT : RIGHT;
        }
        if (!isborder(map, r, c, rpath ? RIGHT : LEFT)) {
            return rpath ? RIGHT : LEFT;
        }

        return -1;
    }
    return -2;
}

int stepFromBottom(Map *map, int r, int c, bool rpath) {
    if (!isborder(map, r, c, HORIZONTAL)) {
        if (!isborder(map, r, c, rpath ? RIGHT : LEFT)) {
            return rpath ? RIGHT : LEFT;
        }
        if (!isborder(map, r, c, rpath ? LEFT : RIGHT)) {
            return rpath ? LEFT : RIGHT;
        }

        return -2;
    }
    return -1;
}

int start_border(Map *map, int r, int c, int leftRight) {

    bool rpath = leftRight == RIGHT;
    //enter from left
    if (c == 1) {
        int result = stepFromLeft(map, r, c, rpath);
        if (result != -2) return result;
    }
    //enter from right
    if (c == map->cols) {
        int result = stepFromRight(map, r, c, rpath);
        if (result != -2) return result;
    }
    //enter from top
    if (r == 1 && c % 2 == 1) {
        int result = stepFromTop(map, r, c, rpath);
        if (result != -2) return result;
    }
    //enter from bottom
    if (r == map->cols && r + c % 2 == 1) {
        int result = stepFromBottom(map, r, c, rpath);
        if (result != -2) return result;
    }

    return -1;
}

int getInverseBorder(int border) {
    if (border == LEFT) {
        return RIGHT;
    }
    if (border == RIGHT) {
        return LEFT;
    }
    if (border == TOP) {
        return BOTTOM;
    }
    if (border == BOTTOM) {
        return TOP;
    }
    return -1;
}

int nextCellMove(Map *map, int r, int c, int leftRight, int fromBorder) {
    bool rpath = leftRight == RIGHT;

    if (fromBorder == LEFT) {
        return stepFromLeft(map, r, c, rpath);
    }
    if (fromBorder == RIGHT) {
        return stepFromRight(map, r, c, rpath);
    }
    if (fromBorder == TOP) {
        return stepFromTop(map, r, c, rpath);
    }
    if (fromBorder == BOTTOM) {
        return stepFromBottom(map, r, c, rpath);
    }
    return -1;
}

int nextStep(Map *map, int *r, int *c, int leftRight, int fromBorder) {
    int nextBorder = nextCellMove(map, *r, *c, leftRight, fromBorder);
    //try to get back out
    if (nextBorder == -1) {
//        printf("from border %d\n", fromBorder);
//        printf("next border %d\n", nextBorder);
        nextBorder = fromBorder;
    }
//    printf("next border %d\n", nextBorder);
    if (nextBorder == LEFT) {
        (*c)--;
    }
    if (nextBorder == RIGHT) {
        (*c)++;
    }
    if (nextBorder == BOTTOM) {
        (*r)++;
    }
    if (nextBorder == TOP) {
        (*r)--;
    }

    return nextBorder;

}

bool isCellOutOfMaze(Map *map, int r, int c) {
    if (r < 1 || c < 1) {
        return true;
    }
    if (r > map->rows || c > map->cols) {
        return true;
    }
    return false;
}

int main(int argc, char *argv[]) {
    Map map;
    char *path = NULL;
    int row = -1;
    int column = -1;


    int argumentsValue = parseArgument(argc, argv, &row, &column);
    int startCol=column;
    int startRow=row;
    if (argumentsValue == -1) {
        invalidPrint();
        return 0;
    } else if (argumentsValue == 0) {
        //--help
        return 0;
    }

    if (argumentsValue == 1) {
        //--test
        path = (char *) argv[2];
        int isFileValid = readFile(path, &map);
        if (isFileValid == -1) {
            invalidPrint();
            freeMap(&map);
            return 0;
        } else if (isFileValid == -2) {
            printf("Memory allocation failed\n");
            return -1;
        }
        if (!areBordersValid(&map)) {
            invalidPrint();
            freeMap(&map);
            return 0;
        }
        validPrint();
        freeMap(&map);
    } else if (argumentsValue == 2 || argumentsValue == 3) {
        int leftRight = argumentsValue == 2 ? RIGHT : LEFT;

        path = (char *) argv[4];
        int isFileValid = readFile(path, &map);
        if (isFileValid == -1) {
            invalidPrint();
            freeMap(&map);
            return 0;
        } else if (isFileValid == -2) {
            printf("Memory allocation failed\n");
        }
        if (!areBordersValid(&map)) {
            invalidPrint();
            return 0;
        }

//        printMap(&map);

        int startBorder = start_border(&map, row, column, leftRight);
        if (startBorder == -1) {
            invalidPrint();
            return 0;
        }
        int fromBorder = getInverseBorder(startBorder);
        int toBorder = -1;
        bool finished = false;


        int output[map.rows * map.cols * 3][2];
        for (int i = 0; i < map.rows * map.cols * 3; ++i) {
//            printf("from border %d\n", fromBorder);

            toBorder = nextStep(&map, &row, &column, leftRight, fromBorder);
            fromBorder = getInverseBorder(toBorder);

            if (isCellOutOfMaze(&map, row, column)) {
                output[i][0] = -1;
                output[i][1] = -1;
//                printf("hell yeah\n");
                finished = true;
                break;
            } else {
//                printf("%d %d\n", row, column);
                output[i][0] = row;
                output[i][1] = column;
            }

        }
        if (finished==true){
            printf("%d,%d\n", startRow, startCol);
            for (int i = 0; i <map.rows * map.cols * 3 ; ++i) {
                if (output[i][0]==-1 && output[i][1]==-1){
                    break;
                }
                printf("%d,%d\n", output[i][0], output[i][1]);
            }
        }else{
            invalidPrint();
        }

        freeMap(&map);
    }


//    printf("row:%d col:%d\n", row, column);
//    printf("filename:%s\n", argv[4]);
//    printf("filename:%s\n", path);
    return 0;
}
