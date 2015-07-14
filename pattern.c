#include <stdio.h>
#include <glib.h>
#include "main.h"
#include "pattern.h"

#define CENTER_X (FIELD_WIDTH / 2)
#define CENTER_Y (FIELD_HEIGHT / 2)

#define GLIDER_GUN_WIDTH     (35)
#define GLIDER_GUN_HEIGHT    (8)
#define EATER_WIDTH          (3)
#define EATER_HEIGHT         (3)

static void pattern_pentadecathlon         (void);
static void pattern_plusar                 (void);
static void pattern_clock2                 (void);
static void pattern_pinwheel               (void);
static void pattern_glider                 (void);
static void pattern_lightweight_spaceship  (void);
static void pattern_middleweight_spaceship (void);
static void pattern_heavyweight_spaceship  (void);
static void pattern_flotilla1              (void);
static void pattern_flotilla2              (void);
static void pattern_glider_gun             (void);
static void pattern_max                    (void);
static void pattern_thin_gun4              (void);
static void pattern_thin_gun8              (void);
static void pattern_thin_gun12             (void);
static void pattern_not_gate               (void);
static void pattern_and_gate               (void);
static void pattern_or_gate                (void);
static void pattern_duplicator             (void);
static void put_glider                     (int x,
                                            int y,
                                            int t);
static void put_array                      (int       x,
                                            int       y,
                                            const int array [][2],
                                            size_t    n_items);
static void put_array_lr                   (int       x,
                                            int       y,
                                            const int array [][2],
                                            size_t    n_items);
static void put_array_ub                   (int       x,
                                            int       y,
                                            const int array [][2],
                                            size_t    n_items);
static void put_array_lr_ub                (int       x,
                                            int       y,
                                            const int array [][2],
                                            size_t    n_items);
static void put                            (int x,
                                            int y);

const pattern_t patterns [] =
{
    {"<b>Oscillators</b>", NULL, 0, 0},
    {"Pentadecathlon", &pattern_pentadecathlon, 0, 0},
    {"Plusar", &pattern_plusar, 0, 0},
    {"Clock II", &pattern_clock2, 0, 0},
    {"Pinwheel", &pattern_pinwheel, 0, 0},
    {"<b>Spaceships</b>", NULL, 0, 0},
    {"Glider", &pattern_glider, CENTER_X, CENTER_Y},
    {"Lightweight Spaceship", &pattern_lightweight_spaceship, 0, 0},
    {"Middleweight Spaceship", &pattern_middleweight_spaceship, 0, 0},
    {"Heavyweight Spaceship", &pattern_heavyweight_spaceship, 0, 0},
    {"Flotilla (1)", &pattern_flotilla1, 0, 0},
    {"Flotilla (2)", &pattern_flotilla2, 0, 0},
    {"<b>Infinite Growth</b>", NULL, 0, 0},
    {"Glider Gun", &pattern_glider_gun, 0, 0},
    {"Max", &pattern_max, CENTER_X, CENTER_Y},
    {"<b>Computation</b>", NULL, 0, 0},
    {"Thin Gun (1/4)", &pattern_thin_gun4, 0, 0},
    {"Thin Gun (1/8)", &pattern_thin_gun8, 0, 0},
    {"Thin Gun (1/12)", &pattern_thin_gun12, 0, 0},
    {"NOT Gate", &pattern_not_gate, 0, 0},
    {"AND Gate", &pattern_and_gate, 0, 0},
    {"OR Gate", &pattern_or_gate, 0, 0},
    {"Duplicator", &pattern_duplicator, 0, 0}
};

const gsize n_patterns = G_N_ELEMENTS(patterns);

// イーター
static const int array_eater [][2] = {{3,3},{0,0},{1,0},{0,1},{2,1},{2,2},{2,3},{3,3}};
// グライダー銃
static const int array_glider_gun [][2] = {{35,8},{24,0},{22,1},{24,1},{12,2},{13,2},{20,2},{21,2},{34,2},{35,2},{11,3},{15,3},{20,3},{21,3},{34,3},{35,3},{0,4},{1,4},{10,4},{16,4},{20,4},{21,4},{0,5},{1,5},{10,5},{14,5},{16,5},{17,5},{22,5},{24,5},{10,6},{16,6},{24,6},{11,7},{15,7},{12,8},{13,8}};
// 1/4 希薄銃
static const int array_thin_gun4 [][2] = {{122,37},{23,0},{24,1},{22,2},{23,2},{24,2},{63,3},{63,4},{65,4},{52,5},{53,5},{66,5},{67,5},{74,5},{75,5},{52,6},{53,6},{66,6},{67,6},{72,6},{76,6},{66,7},{67,7},{71,7},{77,7},{86,7},{87,7},{63,8},{65,8},{70,8},{71,8},{73,8},{77,8},{86,8},{87,8},{63,9},{71,9},{77,9},{72,10},{76,10},{74,11},{75,11},{109,13},{110,13},{107,14},{111,14},{98,15},{106,15},{112,15},{98,16},{100,16},{105,16},{106,16},{108,16},{112,16},{121,16},{122,16},{101,17},{102,17},{106,17},{112,17},{121,17},{122,17},{87,18},{88,18},{101,18},{102,18},{107,18},{111,18},{87,19},{88,19},{101,19},{102,19},{109,19},{110,19},{11,20},{12,20},{98,20},{100,20},{10,21},{12,21},{98,21},{9,22},{16,22},{17,22},{22,22},{0,23},{1,23},{9,23},{12,23},{15,23},{18,23},{21,23},{23,23},{0,24},{1,24},{9,24},{16,24},{17,24},{18,24},{20,24},{22,24},{23,24},{10,25},{12,25},{19,25},{20,25},{22,25},{23,25},{34,25},{35,25},{11,26},{12,26},{20,26},{22,26},{23,26},{34,26},{35,26},{21,27},{23,27},{22,28},{41,34},{42,34},{40,35},{42,35},{40,36},{39,37},{40,37}};
// 1/8 希薄銃
static const int array_thin_gun8 [][2] = {{142,36},{38,0},{39,1},{37,2},{38,2},{39,2},{78,3},{78,4},{80,4},{130,4},{131,4},{11,5},{12,5},{67,5},{68,5},{81,5},{82,5},{89,5},{90,5},{130,5},{132,5},{10,6},{12,6},{67,6},{68,6},{81,6},{82,6},{87,6},{91,6},{120,6},{125,6},{126,6},{133,6},{9,7},{16,7},{17,7},{22,7},{81,7},{82,7},{86,7},{92,7},{101,7},{102,7},{119,7},{121,7},{124,7},{127,7},{130,7},{133,7},{141,7},{142,7},{0,8},{1,8},{9,8},{12,8},{15,8},{18,8},{21,8},{23,8},{78,8},{80,8},{85,8},{86,8},{88,8},{92,8},{101,8},{102,8},{119,8},{120,8},{122,8},{124,8},{125,8},{126,8},{133,8},{141,8},{142,8},{0,9},{1,9},{9,9},{16,9},{17,9},{18,9},{20,9},{22,9},{23,9},{78,9},{86,9},{92,9},{107,9},{108,9},{119,9},{120,9},{122,9},{123,9},{130,9},{132,9},{10,10},{12,10},{19,10},{20,10},{22,10},{23,10},{34,10},{35,10},{87,10},{91,10},{107,10},{108,10},{119,10},{120,10},{122,10},{130,10},{131,10},{11,11},{12,11},{20,11},{22,11},{23,11},{34,11},{35,11},{89,11},{90,11},{119,11},{121,11},{21,12},{23,12},{120,12},{22,13},{57,33},{58,33},{56,34},{58,34},{56,35},{55,36},{56,36}};
// 1/12 希薄銃
static const int array_thin_gun12 [][2] = {{117,47},{104,0},{105,0},{11,1},{12,1},{104,1},{106,1},{10,2},{12,2},{94,2},{99,2},{100,2},{107,2},{9,3},{16,3},{17,3},{22,3},{93,3},{95,3},{98,3},{101,3},{104,3},{107,3},{115,3},{116,3},{0,4},{1,4},{9,4},{12,4},{15,4},{18,4},{21,4},{23,4},{93,4},{94,4},{96,4},{98,4},{99,4},{100,4},{107,4},{115,4},{116,4},{0,5},{1,5},{9,5},{16,5},{17,5},{18,5},{20,5},{22,5},{23,5},{81,5},{82,5},{93,5},{94,5},{96,5},{97,5},{104,5},{106,5},{10,6},{12,6},{19,6},{20,6},{22,6},{23,6},{34,6},{35,6},{81,6},{82,6},{93,6},{94,6},{96,6},{104,6},{105,6},{11,7},{12,7},{20,7},{22,7},{23,7},{34,7},{35,7},{93,7},{95,7},{21,8},{23,8},{94,8},{22,9},{53,11},{54,12},{52,13},{53,13},{54,13},{93,14},{93,15},{95,15},{82,16},{83,16},{96,16},{97,16},{104,16},{105,16},{82,17},{83,17},{96,17},{97,17},{102,17},{106,17},{96,18},{97,18},{101,18},{107,18},{116,18},{117,18},{93,19},{95,19},{100,19},{101,19},{103,19},{107,19},{116,19},{117,19},{93,20},{101,20},{107,20},{102,21},{106,21},{104,22},{105,22},{72,44},{73,44},{71,45},{73,45},{71,46},{70,47},{71,47}};

/******************************************************************************
 *
 *  Oscillators
 *
 *****************************************************************************/

static void
pattern_pentadecathlon (void)
{
    static const int array [][2] = {{15,2}, {2,0},{5,0},{10,0},{13,0},{0,1},{1,1},{2,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},{13,1},{14,1},{15,1},{2,2},{5,2},{10,2},{13,2}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_plusar (void)
{
    static const int array [][2] = {{12,12},{2,0},{3,0},{4,0},{8,0},{9,0},{10,0},{0,2},{5,2},{7,2},{12,2},{0,3},{5,3},{7,3},{12,3},{0,4},{5,4},{7,4},{12,4},{2,5},{3,5},{4,5},{8,5},{9,5},{10,5},{2,7},{3,7},{4,7},{8,7},{9,7},{10,7},{0,8},{5,8},{7,8},{12,8},{0,9},{5,9},{7,9},{12,9},{0,10},{5,10},{7,10},{12,10},{2,12},{3,12},{4,12},{8,12},{9,12},{10,12}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_clock2 (void)
{
    static const int array [][2] = {{11,11},{6,0},{7,0},{6,1},{7,1},{4,3},{5,3},{6,3},{7,3},{0,4},{1,4},{3,4},{8,4},{0,5},{1,5},{3,5},{4,5},{8,5},{3,6},{5,6},{6,6},{8,6},{10,6},{11,6},{3,7},{8,7},{10,7},{11,7},{4,8},{5,8},{6,8},{7,8},{4,10},{5,10},{4,11},{5,11}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_pinwheel (void)
{
    static const int array [][2] = {{11,11},{6,0},{7,0},{6,1},{7,1},{4,3},{5,3},{6,3},{7,3},{0,4},{1,4},{3,4},{8,4},{0,5},{1,5},{3,5},{4,5},{8,5},{3,6},{6,6},{8,6},{10,6},{11,6},{3,7},{5,7},{8,7},{10,7},{11,7},{4,8},{5,8},{6,8},{7,8},{4,10},{5,10},{4,11},{5,11}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

/******************************************************************************
 *
 *  Spaceships
 *
 *****************************************************************************/

static void
pattern_glider (void)
{
    static const int array [][2] = {{20,8},{0,0},{1,0},{6,0},{7,0},{8,0},{12,0},{13,0},{14,0},{19,0},{20,0},{0,1},{2,1},{6,1},{14,1},{18,1},{20,1},{0,2},{7,2},{13,2},{20,2},{0,6},{7,6},{13,6},{20,6},{0,7},{2,7},{6,7},{14,7},{18,7},{20,7},{0,8},{1,8},{6,8},{7,8},{8,8},{12,8},{13,8},{14,8},{19,8},{20,8}};

    put_array(CENTER_X - 11, CENTER_Y - 5, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_lightweight_spaceship (void)
{
    static const int array [][2] = {{4,3},{0,0},{3,0},{4,1},{0,2},{4,2},{1,3},{2,3},{3,3},{4,3}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_middleweight_spaceship (void)
{
    static const int array [][2] = {{5,4},{2,0},{0,1},{4,1},{5,2},{0,3},{5,3},{1,4},{2,4},{3,4},{4,4},{5,4}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_heavyweight_spaceship (void)
{
    static const int array [][2] = {{6,4},{2,0},{3,0},{0,1},{5,1},{6,2},{0,3},{6,3},{1,4},{2,4},{3,4},{4,4},{5,4},{6,4}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_flotilla1 (void)
{
    static const int array [][2] = {{7,15},{3,0},{4,0},{1,1},{2,1},{4,1},{5,1},{1,2},{2,2},{3,2},{4,2},{2,3},{3,3},{1,6},{2,6},{3,6},{4,6},{5,6},{6,6},{7,6},{0,7},{7,7},{7,8},{0,9},{6,9},{1,10},{3,12},{4,12},{1,13},{2,13},{4,13},{5,13},{1,14},{2,14},{3,14},{4,14},{2,15},{3,15}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

static void
pattern_flotilla2 (void)
{
    static const int array [][2] = {{18,28},{9,0},{10,0},{11,0},{12,0},{13,0},{14,0},{8,1},{14,1},{14,2},{8,3},{13,3},{6,4},{7,4},{4,5},{15,5},{16,6},{4,7},{16,7},{5,8},{6,8},{7,8},{8,8},{9,8},{10,8},{11,8},{12,8},{13,8},{14,8},{15,8},{16,8},{1,12},{2,12},{3,12},{4,12},{5,12},{6,12},{7,12},{8,12},{9,12},{10,12},{11,12},{12,12},{13,12},{14,12},{15,12},{16,12},{17,12},{18,12},{0,13},{18,13},{18,14},{0,15},{17,15},{2,16},{3,16},{4,17},{15,17},{16,18},{4,19},{16,19},{5,20},{6,20},{7,20},{8,20},{9,20},{10,20},{11,20},{12,20},{13,20},{14,20},{15,20},{16,20},{9,24},{10,24},{11,24},{12,24},{13,24},{14,24},{8,25},{14,25},{14,26},{8,27},{13,27},{10,28},{11,28}};

    put_array(6, 6, array, G_N_ELEMENTS(array));

    return;
}

/******************************************************************************
 *
 *  Infinite Growth
 *
 *****************************************************************************/

static void
pattern_glider_gun (void)
{
    put_array(3, 3, array_glider_gun, G_N_ELEMENTS(array_glider_gun));

    return;
}


static void
pattern_max (void)
{
    static const int array [][2] = {{26,26},{18,0},{17,1},{18,1},{19,1},{12,2},{13,2},{14,2},{19,2},{20,2},{11,3},{14,3},{15,3},{16,3},{19,3},{21,3},{22,3},{10,4},{14,4},{16,4},{19,4},{21,4},{10,5},{15,5},{17,5},{19,5},{21,5},{23,5},{24,5},{12,6},{17,6},{19,6},{23,6},{24,6},{0,7},{1,7},{2,7},{3,7},{9,7},{11,7},{16,7},{20,7},{22,7},{23,7},{24,7},{0,8},{4,8},{5,8},{7,8},{9,8},{10,8},{11,8},{13,8},{14,8},{24,8},{25,8},{0,9},{6,9},{7,9},{13,9},{1,10},{4,10},{5,10},{7,10},{10,10},{13,10},{15,10},{16,10},{7,11},{9,11},{11,11},{13,11},{15,11},{17,11},{23,11},{24,11},{25,11},{26,11},{1,12},{4,12},{5,12},{7,12},{10,12},{13,12},{16,12},{17,12},{19,12},{21,12},{22,12},{26,12},{0,13},{6,13},{7,13},{11,13},{13,13},{15,13},{19,13},{20,13},{26,13},{0,14},{4,14},{5,14},{7,14},{9,14},{10,14},{13,14},{16,14},{19,14},{21,14},{22,14},{25,14},{0,15},{1,15},{2,15},{3,15},{9,15},{11,15},{13,15},{15,15},{17,15},{19,15},{10,16},{11,16},{13,16},{16,16},{19,16},{21,16},{22,16},{25,16},{13,17},{19,17},{20,17},{26,17},{1,18},{2,18},{12,18},{13,18},{15,18},{16,18},{17,18},{19,18},{21,18},{22,18},{26,18},{2,19},{3,19},{4,19},{6,19},{10,19},{15,19},{17,19},{23,19},{24,19},{25,19},{26,19},{2,20},{3,20},{7,20},{9,20},{14,20},{2,21},{3,21},{5,21},{7,21},{9,21},{11,21},{16,21},{5,22},{7,22},{10,22},{12,22},{16,22},{4,23},{5,23},{7,23},{10,23},{11,23},{12,23},{15,23},{6,24},{7,24},{12,24},{13,24},{14,24},{7,25},{8,25},{9,25},{8,26}};

    put_array(CENTER_X - array[0][0] / 2, CENTER_Y - array[0][1] / 2, array, G_N_ELEMENTS(array));

    return;
}

/******************************************************************************
 *
 *  Logic Gates
 *
 *****************************************************************************/

static void
pattern_thin_gun4 (void)
{
    put_array_lr_ub(3, 3, array_thin_gun4, G_N_ELEMENTS(array_thin_gun4));

    return;
}

static void
pattern_thin_gun8 (void)
{
    put_array_lr_ub(3, 3, array_thin_gun8, G_N_ELEMENTS(array_thin_gun8));
/*
    static const int array [][2] = {{35,8},{22,0},{21,1},{23,1},{11,2},{12,2},{20,2},{22,2},{23,2},{34,2},{35,2},{10,3},{12,3},{19,3},{20,3},{22,3},{23,3},{34,3},{35,3},{0,4},{1,4},{9,4},{16,4},{17,4},{18,4},{20,4},{22,4},{23,4},{0,5},{1,5},{9,5},{12,5},{15,5},{18,5},{21,5},{23,5},{9,6},{16,6},{17,6},{22,6},{10,7},{12,7},{11,8},{12,8}};

    // 1/8 希薄銃
    put_array_lr(200, 150, array_glider_gun, G_N_ELEMENTS(array_glider_gun));
    put_array_ub(133, 152, array, G_N_ELEMENTS(array));
    put_glider(170, 147, 0);

    // イーター
    put_array_lr(188, 180, array_eater, G_N_ELEMENTS(array_eater));

    put_array_lr_ub(240, 151, array, G_N_ELEMENTS(array));
*/
    return;
}

static void
pattern_thin_gun12 (void)
{
    put_array_lr_ub(3, 3, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
/*
    static const int array [][2] = {{35,8},{22,0},{21,1},{23,1},{11,2},{12,2},{20,2},{22,2},{23,2},{34,2},{35,2},{10,3},{12,3},{19,3},{20,3},{22,3},{23,3},{34,3},{35,3},{0,4},{1,4},{9,4},{16,4},{17,4},{18,4},{20,4},{22,4},{23,4},{0,5},{1,5},{9,5},{12,5},{15,5},{18,5},{21,5},{23,5},{9,6},{16,6},{17,6},{22,6},{10,7},{12,7},{11,8},{12,8}};

    // 1/12 希薄銃
    put_array_lr(200, 150, array_glider_gun, G_N_ELEMENTS(array_glider_gun));
    put_array_ub(118, 137, array, G_N_ELEMENTS(array));
    put_glider(170, 147, 0);

    // イーター
    put_array_lr(188, 180, array_eater, G_N_ELEMENTS(array_eater));

    put_array_lr_ub(199, 136, array, G_N_ELEMENTS(array));
*/
    return;
}

static void
pattern_not_gate (void)
{
    // 右上の信号系列
    put_glider(34, 40, 1); // 第 1 番グライダーを消去
    put_glider(27, 32, 3); // 第 2 番グライダーを消去
    put_glider(19, 25, 1); // 第 3 番グライダーを消去
    put_glider(12, 17, 3); // 第 4 番グライダーを消去
    put_glider( 4, 10, 1); // 第 5 番グライダーを消去

    // グライダー銃
    put_array_ub(20, 80, array_glider_gun, G_N_ELEMENTS(array_glider_gun));

    return;
}

static void
pattern_and_gate (void)
{
    // 右上の信号系列
    put_glider(44, 60, 1); // 第 1 番グライダーを消去
    put_glider(37, 52, 3); // 第 2 番グライダーを消去
    put_glider(29, 45, 1); // 第 3 番グライダーを消去
    put_glider(22, 37, 3); // 第 4 番グライダーを消去
    put_glider(14, 30, 1); // 第 5 番グライダーを消去

    // 右上の信号系列
    put_glider(44, 44, 1); // 第 1 番グライダーを消去
    put_glider(37, 36, 3); // 第 2 番グライダーを消去
    put_glider(29, 29, 1); // 第 3 番グライダーを消去
    put_glider(22, 21, 3); // 第 4 番グライダーを消去
    put_glider(14, 14, 1); // 第 5 番グライダーを消去

    // グライダー銃
    put_array_ub(30, 100, array_glider_gun, G_N_ELEMENTS(array_glider_gun));

    // イーター
    put_array_ub(100, 49, array_eater, G_N_ELEMENTS(array_eater));

    return;
}

static void
pattern_or_gate (void)
{
    // 左下の信号系列
    put_glider(44, 60, 1); // 第 1 番グライダーを消去
    put_glider(37, 52, 3); // 第 2 番グライダーを消去
    put_glider(29, 45, 1); // 第 3 番グライダーを消去
    put_glider(22, 37, 3); // 第 4 番グライダーを消去
    put_glider(14, 30, 1); // 第 5 番グライダーを消去

    // 右上の信号系列
    put_glider(44, 44, 1); // 第 1 番グライダーを消去
    put_glider(37, 36, 3); // 第 2 番グライダーを消去
    put_glider(29, 29, 1); // 第 3 番グライダーを消去
    put_glider(22, 21, 3); // 第 4 番グライダーを消去
    put_glider(14, 14, 1); // 第 5 番グライダーを消去

    // グライダー銃 (遮断パルス)
    put_array_ub(30, 100, array_glider_gun, G_N_ELEMENTS(array_glider_gun));

    // グライダー銃 (出力パルス)
    put_array_lr_ub(99, 99, array_glider_gun, G_N_ELEMENTS(array_glider_gun));

    // イーター (グライダー銃)
    //put_eater(100, 49);
    //flip_ub(100, 49, EATER_WIDTH, EATER_HEIGHT);

    // イーター (右上信号)
    put_array(91, 90, array_eater, G_N_ELEMENTS(array_eater));

    return;
}

static void
pattern_duplicator (void)
{
    static const int array_input [][2] = {{182,182},{2,0},{0,1},{2,1},{1,2},{2,2},
                                                    {92,90},{90,91},{92,91},{91,92},{92,92},
                                                    {182,180},{180,181},{182,181},{181,182},{182,182}};
    static const int array_block [][2] = {{2,2},{0,0},{0,1},{1,0},{1,1}};

    // 入力信号
    put_array(106, 182, array_input, G_N_ELEMENTS(array_input));

    // ベースクロック信号
    put_array_lr(267, 521, array_thin_gun4, G_N_ELEMENTS(array_thin_gun4));

    // 第2信号のマスククロック信号
    put_array(519, 423, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
    // 第1信号のマスククロック信号
    put_array(459, 483, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
    // 第3信号のマスククロック信号
    put_array(399, 543, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
    // 第1信号マスククロックの第1グライダーは邪魔なので除去する
    put_array(565, 375, array_block, G_N_ELEMENTS(array_block));
/*
    // 入力信号
    put_array(6, 62, array_input, G_N_ELEMENTS(array_input));

    // ベースクロック信号
    put_array_lr(167, 401, array_thin_gun4, G_N_ELEMENTS(array_thin_gun4));

    // 第2信号のマスククロック信号
    put_array(419, 303, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
    // 第1信号のマスククロック信号
    put_array(359, 363, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
    // 第3信号のマスククロック信号
    put_array(299, 423, array_thin_gun12, G_N_ELEMENTS(array_thin_gun12));
    // 第1信号マスククロックの第1グライダーは邪魔なので除去する
    put_array(465, 255, array_block, G_N_ELEMENTS(array_block));
*/
    return;
}

/******************************************************************************
 *
 *  Utility
 *
 *****************************************************************************/

static void
put_glider (int x,
            int y,
            int t)
{
    switch (t)
    {
        case 0:
            put(x+1, y);
            put(x+2, y+1);
            put(x  , y+2);
            put(x+1, y+2);
            put(x+2, y+2);
            break;
        case 1:
            put(x  , y);
            put(x+2, y);
            put(x+1, y+1);
            put(x+2, y+1);
            put(x+1, y+2);
            break;
        case 2:
            put(x+2, y);
            put(x  , y+1);
            put(x+2, y+1);
            put(x+1, y+2);
            put(x+2, y+2);
            break;
        case 3:
            put(x  , y);
            put(x+1, y+1);
            put(x+2, y+1);
            put(x  , y+2);
            put(x+1, y+2);
            break;
    }

    return;
}

static void
put_array (int       x,
           int       y,
           const int array [][2],
           size_t    n_items)
{
    size_t i;

    for (i = 1; i < n_items; ++i)
    {
        put(x + array[i][0], y + array[i][1]);
    }

    return;
}

static void
put_array_lr (int       x,
              int       y,
              const int array [][2],
              size_t    n_items)
{
    size_t i;

    x += array[0][0];

    for (i = 1; i < n_items; ++i)
    {
        put(x - array[i][0], y + array[i][1]);
    }

    return;
}

static void
put_array_ub (int       x,
              int       y,
              const int array [][2],
              size_t    n_items)
{
    size_t i;

    y += array[0][1];

    for (i = 1; i < n_items; ++i)
    {
        put(x + array[i][0], y - array[i][1]);
    }

    return;
}

static void
put_array_lr_ub (int       x,
                 int       y,
                 const int array [][2],
                 size_t    n_items)
{
    size_t i;

    x += array[0][0];
    y += array[0][1];

    for (i = 1; i < n_items; ++i)
    {
        put(x - array[i][0], y - array[i][1]);
    }

    return;
}

static void
put (int x,
     int y)
{
    if (x >= 0 && x < FIELD_WIDTH && y >= 0 && y < FIELD_HEIGHT)
    {
        cell_field[field_active_index][x+1][y+1] = 1;
    }

    return;
}