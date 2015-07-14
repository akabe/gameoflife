#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include "main.h"
#include "pattern.h"

#define IMAGE_CHANNELS (3) /* RGB */

// Rendering
static GdkPixbuf    * offscreen_buffer       = NULL;
static gint           layout_cell_size       = DEFAULT_LAYOUT_CELL_SIZE;
static gint           layout_cell_spacing    = DEFAULT_LAYOUT_CELL_SPACING;
// Simulation
gint                  field_active_index     = 0;
guint8                cell_field [2][FIELD_WIDTH + 2][FIELD_HEIGHT + 2];
static guint          generation             = 0;
static volatile gint  simulation_interval    = DEFAULT_INTERVAL;
static gboolean       is_simulating          = FALSE; // 実時間シミュレーション中で TRUE
// Multi-Thread
static GThread      * thread; // シミュレーション用スレッド
static GStaticMutex   mutex_standby = G_STATIC_MUTEX_INIT; // シミュレーション同期
// Gesture
static gint           drawing_cell_state;
// GTK+ Widgets
static GtkWidget    * sw_canvas              = NULL;
static GtkWidget    * da_canvas              = NULL;
static GtkWidget    * lb_generation          = NULL;
static GtkWidget    * sb_interval            = NULL;
static GtkWidget    * sb_layout_cell_size    = NULL;
static GtkWidget    * sb_layout_cell_spacing = NULL;

static GtkWidget * _gtk_label_new       (const gchar * text);
static GtkWidget * _gtk_icon_button_new (const gchar * text,
                                         const gchar * stock_id);
static GtkWidget * _gtk_spin_button_new (gdouble min,
                                         gdouble max,
                                         gdouble step,
                                         gdouble value);

static void     btn_pattern_on_clicked              (GtkButton       * button,
                                                     const pattern_t * pattern);
static void     sb_interval_on_value_changed        (GtkSpinButton * spin_button,
                                                     gpointer        user_data);
static void     sb_layout_on_value_changed          (GtkSpinButton * spin_button,
                                                     gint          * param);
static gboolean canvas_on_button_press_event        (GtkWidget      * widget,
                                                     GdkEventButton * event,
                                                     gpointer         user_data);
static gboolean canvas_on_motion_notify_event       (GtkWidget      * widget,
                                                     GdkEventMotion * event,
                                                     gpointer         user_data);
static gboolean canvas_on_expose_event              (GtkWidget      * widget,
                                                     GdkEventExpose * event,
                                                     gpointer         user_data);
static gboolean main_on_idle                        (gpointer user_data);
static void     btn_step_on_clicked                 (GtkButton * button,
                                                     gpointer    user_data);
static void     btn_start_on_clicked                (GtkButton * button,
                                                     gpointer    user_data);
static void     btn_stop_on_clicked                 (GtkButton * button,
                                                     gpointer    user_data);
static void     btn_clear_on_clicked                (GtkButton * button,
                                                     gpointer    user_data);
static void     btn_info_on_clicked                 (GtkButton * button,
                                                     gpointer    user_data);
static gboolean put_cell_at_coordinate              (gint x,
                                                     gint y);
static void     update_onscreen_buffer              (gint x,
                                                     gint y,
                                                     gint width,
                                                     gint height);
static void     update_offscreen_buffer             (gint x1,
                                                     gint y1,
                                                     gint x2,
                                                     gint y2);
static void     resize_drawing_area                 (void);
static gint     find_min_y                          (void);
static gint     find_max_y                          (void);
static gint     find_min_x                          (void);
static gint     find_max_x                          (void);
static void     stop_simulation                     (void);
static gpointer simulation_thread                   (gpointer user_data);
static void     simulate                            (void);

int
main (int argc, char * * argv)
{
    GtkWidget *window, *sw_ptrn, *eb_canvas;
    GtkWidget *vbox_main, *vbox_side, *vbox_ptrn, *hbox_ctrl, *hbox_scrn, *tbl_side;
    GtkWidget *btn_step, *btn_start, *btn_stop, *btn_clear, *btn_info;
    GError *error;
    gsize i;

    gtk_init(&argc, &argv);

    memset(cell_field, 0, sizeof(cell_field)); // セルフィールドを初期化する

    // 同期用 Mutex をロックして，スレッドの動作を停止させておく．
    g_static_mutex_lock(&mutex_standby);

    // シミュレーション用のスレッドを起動する
    thread = g_thread_create(&simulation_thread, NULL, FALSE, &error);
    if (thread == NULL)
    {
        g_error("Cannot create GThread\n");
        return EXIT_FAILURE;
    }

    // 画面更新を登録する
    g_idle_add(&main_on_idle, NULL);

    // サイドエリア
    lb_generation          = _gtk_label_new("0");
    sb_interval            = _gtk_spin_button_new(0.0, 10000.0, 10.0, DEFAULT_INTERVAL);
    sb_layout_cell_size    = _gtk_spin_button_new(1.0, 10.0, 1.0, DEFAULT_LAYOUT_CELL_SIZE);
    sb_layout_cell_spacing = _gtk_spin_button_new(0.0, 3.0, 1.0, DEFAULT_LAYOUT_CELL_SPACING);

    g_signal_connect(sb_interval, "value-changed", G_CALLBACK(sb_interval_on_value_changed), NULL);
    g_signal_connect(sb_layout_cell_size, "value-changed", G_CALLBACK(sb_layout_on_value_changed), &layout_cell_size);
    g_signal_connect(sb_layout_cell_spacing, "value-changed", G_CALLBACK(sb_layout_on_value_changed), &layout_cell_spacing);

    tbl_side = gtk_table_new(7, 2, FALSE);
    gtk_table_attach(GTK_TABLE(tbl_side), _gtk_label_new("Generation"),
                     0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), _gtk_label_new("Interval"),
                     0, 1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), _gtk_label_new("Cell Size"),
                     0, 1, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), _gtk_label_new("Cell Spacing"),
                     0, 1, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), lb_generation,
                     1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), sb_interval,
                     1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), sb_layout_cell_size,
                     1, 2, 2, 3, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);
    gtk_table_attach(GTK_TABLE(tbl_side), sb_layout_cell_spacing,
                     1, 2, 3, 4, GTK_EXPAND | GTK_FILL, GTK_SHRINK, 5, 5);

    // 制御ボタン類
    btn_step  = _gtk_icon_button_new("Step", GTK_STOCK_REFRESH);
    btn_start = _gtk_icon_button_new("Start", GTK_STOCK_MEDIA_PLAY);
    btn_stop  = _gtk_icon_button_new("Stop", GTK_STOCK_MEDIA_PAUSE);
    btn_clear = _gtk_icon_button_new("Clear", GTK_STOCK_CLEAR);
    btn_info  = _gtk_icon_button_new("Info", GTK_STOCK_INFO);

    g_signal_connect(btn_step, "clicked", G_CALLBACK(btn_step_on_clicked), NULL);
    g_signal_connect(btn_start, "clicked", G_CALLBACK(btn_start_on_clicked), NULL);
    g_signal_connect(btn_stop, "clicked", G_CALLBACK(btn_stop_on_clicked), NULL);
    g_signal_connect(btn_clear, "clicked", G_CALLBACK(btn_clear_on_clicked), NULL);
    g_signal_connect(btn_info, "clicked", G_CALLBACK(btn_info_on_clicked), NULL);

    hbox_ctrl = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_step, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_start, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_stop, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_clear, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_ctrl), btn_info, FALSE, FALSE, 0);

    // 描画エリア
    da_canvas = gtk_drawing_area_new();
    g_signal_connect(da_canvas, "expose-event", G_CALLBACK(canvas_on_expose_event), NULL);

    eb_canvas = gtk_event_box_new();
    gtk_widget_add_events(eb_canvas, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_MOTION_MASK);
    gtk_container_add(GTK_CONTAINER(eb_canvas), da_canvas);
    g_signal_connect(eb_canvas, "button-press-event", G_CALLBACK(canvas_on_button_press_event), NULL);
    g_signal_connect(eb_canvas, "motion-notify-event", G_CALLBACK(canvas_on_motion_notify_event), NULL);

    sw_canvas = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw_canvas),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw_canvas), eb_canvas);

    resize_drawing_area();

    // パターン
    vbox_ptrn = gtk_vbox_new(FALSE, 0);
    for (i = 0; i < n_patterns; ++i)
    {
        GtkWidget *widget;
        if (patterns[i].func != NULL)
        {
            widget = gtk_button_new_with_label(patterns[i].name);
            gtk_button_set_relief(GTK_BUTTON(widget), GTK_RELIEF_NONE);
            gtk_widget_set_can_focus(widget, FALSE);
            g_signal_connect(widget, "clicked", G_CALLBACK(btn_pattern_on_clicked), (gpointer) &patterns[i]);
        }
        else
        {
            widget = gtk_label_new(patterns[i].name);
            gtk_label_set_use_markup(GTK_LABEL(widget), TRUE);
        }

        gtk_box_pack_start(GTK_BOX(vbox_ptrn), widget, FALSE, FALSE, 0);
    }

    sw_ptrn = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw_ptrn),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw_ptrn), vbox_ptrn);

    // サイドエリア
    vbox_side = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_side), tbl_side, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_side), sw_ptrn, TRUE, TRUE, 0);

    // メインレイアウト
    hbox_scrn = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_scrn), sw_canvas, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_scrn), vbox_side, FALSE, FALSE, 0);

    vbox_main = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_main), hbox_ctrl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox_main), hbox_scrn, TRUE, TRUE, 0);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Conway's Game of Life");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    gtk_container_add(GTK_CONTAINER(window), vbox_main);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}

static GtkWidget *
_gtk_label_new (const gchar * text)
{
    GtkWidget *label;

    label = gtk_label_new(text);
    gtk_misc_set_alignment(GTK_MISC(label), 0.0f, 0.5f); // Left Alignment

    return label;
}

static GtkWidget *
_gtk_icon_button_new (const gchar * text,
                      const gchar * stock_id)
{
    GtkWidget *button, *hbox, *image;

    image = gtk_image_new_from_stock(stock_id,
                                     GTK_ICON_SIZE_LARGE_TOOLBAR);

    hbox = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), image, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), gtk_label_new(text), TRUE, TRUE, 5);

    button = gtk_button_new();
    gtk_widget_set_can_focus(button, FALSE);
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_container_add(GTK_CONTAINER(button), hbox);

    return button;
}

static GtkWidget *
_gtk_spin_button_new (gdouble min,
                      gdouble max,
                      gdouble step,
                      gdouble value)
{
    GtkWidget *spin_button;

    spin_button = gtk_spin_button_new_with_range(min, max, step);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_button), value);

    return spin_button;
}

static void
btn_pattern_on_clicked (GtkButton       * button G_GNUC_UNUSED,
                        const pattern_t * pattern)
{
    const gint unit = layout_cell_size + layout_cell_spacing;
    GtkAdjustment *adjustment;
    gdouble pos;

    stop_simulation(); // 実時間シミュレーションを停止する．

    generation = 0; // 世代を 0 に戻す
    memset(cell_field[field_active_index], 0, sizeof(*cell_field)); // フィールドを初期化する

    (* pattern->func)(); // パターンをフィールド上に配置する

    gtk_widget_queue_draw(da_canvas); // 画面を再描画する

    // 画面の横位置を合わせる
    adjustment = gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(sw_canvas));
    pos = pattern->x * unit - (unit + gtk_adjustment_get_page_size(adjustment)) / 2.0;
    gtk_adjustment_set_value(adjustment, pos);

    // 画面の縦位置を合わせる
    adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(sw_canvas));
    pos = pattern->y * unit - (unit + gtk_adjustment_get_page_size(adjustment)) / 2.0;
    gtk_adjustment_set_value(adjustment, pos);

    return;
}

static void
sb_interval_on_value_changed (GtkSpinButton * spin_button,
                              gpointer        user_data G_GNUC_UNUSED)
{
    simulation_interval = gtk_spin_button_get_value(spin_button);

    return;
}

static void
sb_layout_on_value_changed (GtkSpinButton * spin_button,
                            gint          * param)
{
    *param = gtk_spin_button_get_value(spin_button);

    resize_drawing_area();

    return;
}

static gboolean
canvas_on_button_press_event (GtkWidget      * widget G_GNUC_UNUSED,
                              GdkEventButton * event,
                              gpointer         user_data G_GNUC_UNUSED)
{
    switch (event->button)
    {
        case 1: // Left Button
            drawing_cell_state = 1;
            return put_cell_at_coordinate(event->x, event->y);
        case 3: // Right Button
            drawing_cell_state = 0;
            return put_cell_at_coordinate(event->x, event->y);
        default:
            drawing_cell_state = -1;
            return FALSE;
    }
}

static gboolean
canvas_on_motion_notify_event (GtkWidget      * widget G_GNUC_UNUSED,
                               GdkEventMotion * event,
                               gpointer         user_data G_GNUC_UNUSED)
{
    if (drawing_cell_state != -1)
    {
        return put_cell_at_coordinate(event->x, event->y);
    }

    return FALSE;
}

/**
 * ウィンドウ上の座標を基にセルの状態を変更する．
 * ドラッグ時に呼び出される．
 */
static gboolean
put_cell_at_coordinate (gint x,
                        gint y)
{
    const gint unit = layout_cell_size + layout_cell_spacing;
    const gint p = x / unit,
               q = y / unit;

    if (p >= 0 && p < FIELD_WIDTH && q >= 0 && q < FIELD_HEIGHT)
    {
        GdkEvent e;

        cell_field[field_active_index][p+1][q+1] = drawing_cell_state;

        // 再描画する
        e.expose.type        = GDK_EXPOSE;
        e.expose.window      = da_canvas->window;
        e.expose.area.x      = MAX(0, x - unit);
        e.expose.area.y      = MAX(0, y - unit);
        e.expose.area.width  = 2 * unit;
        e.expose.area.height = 2 * unit;

        gtk_widget_send_expose(da_canvas, &e);

        return TRUE;
    }

    return FALSE;
}

static gboolean
canvas_on_expose_event (GtkWidget      * widget G_GNUC_UNUSED,
                        GdkEventExpose * event,
                        gpointer         user_data G_GNUC_UNUSED)
{
    gint x1, y1, x2, y2;

    x1 = event->area.x / (layout_cell_size + layout_cell_spacing);
    y1 = event->area.y / (layout_cell_size + layout_cell_spacing);
    x2 = (event->area.x + event->area.width + layout_cell_spacing) / (layout_cell_size + layout_cell_spacing) + 2;
    y2 = (event->area.y + event->area.height + layout_cell_spacing) / (layout_cell_size + layout_cell_spacing) + 2;

    if (x2 > FIELD_WIDTH) x2 = FIELD_WIDTH;
    if (y2 > FIELD_HEIGHT) y2 = FIELD_HEIGHT;

    update_offscreen_buffer(x1, y1, x2, y2);

    update_onscreen_buffer(event->area.x, event->area.y,
                           event->area.width, event->area.height);

    return TRUE;
}

/**
 * オフスクリーンバッファを画面に描画する．
 */
static void
update_onscreen_buffer (gint x,
                        gint y,
                        gint width,
                        gint height)
{
    const gint img_width = gdk_pixbuf_get_width(offscreen_buffer),
               img_height = gdk_pixbuf_get_height(offscreen_buffer);

    if (x + width > img_width) width = img_width - x;
    if (y + height > img_height) height = img_height - y;

    gdk_draw_pixbuf(da_canvas->window,
                    da_canvas->style->fg_gc[GTK_WIDGET_STATE(da_canvas)],
                    offscreen_buffer,
                    x, y,
                    x, y,
                    width, height,
                    GDK_RGB_DITHER_NONE,
                    0, 0);

    return;
}

/**
 * オフスクリーンバッファに「ユーザーから見えている部分」だけセルフィールドを描画する．
 */
static void
update_offscreen_buffer (gint x1,
                         gint y1,
                         gint x2,
                         gint y2)
{
    const gint stride = gdk_pixbuf_get_rowstride(offscreen_buffer),
               cstep = stride - layout_cell_size * IMAGE_CHANNELS,
               xstep = (layout_cell_size + layout_cell_spacing) * IMAGE_CHANNELS,
               ystep = (layout_cell_size + layout_cell_spacing) * stride;
    const const_field_t field = (const_field_t) cell_field[field_active_index];
    guint8 * restrict pixels = gdk_pixbuf_get_pixels(offscreen_buffer),
           * restrict xptr, * restrict cptr, color;
    gint i, j, k, l;

    pixels += y1 * ystep + x1 * xstep;

    for (i = y1; i < y2; ++i)
    {
        xptr = pixels;

        for (j = x1; j < x2; ++j)
        {
            color = (field[j+1][i+1] == 0 ? 0xff : (field[j+1][i+1] == 1 ? 0x00 : 0xcc));
            cptr = xptr;

            for (k = 0; k < layout_cell_size; ++k)
            {
                for (l = 0; l < layout_cell_size; ++l)
                {
                    cptr[0] = color;
                    cptr[1] = color;
                    cptr[2] = color;
                    cptr += IMAGE_CHANNELS;
                }

                cptr += cstep;
            }

            xptr += xstep;
        }

        pixels += ystep;
    }

    return;
}

/**
 * layout_cell_size, layout_cell_spacing の値に合わせて描画エリアの大きさを決定し，
 * GtkDrawingArea をリサイズする．
 * また，オフスクリーンバッファも必要に応じてリサイズする．
 */
static void
resize_drawing_area (void)
{
    gint width  = FIELD_WIDTH * layout_cell_size + (FIELD_WIDTH - 1) * layout_cell_spacing;
    gint height = FIELD_HEIGHT * layout_cell_size + (FIELD_HEIGHT - 1) * layout_cell_spacing;

    if (offscreen_buffer == NULL)
    {
        offscreen_buffer = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                          FALSE, 8,
                                          width,
                                          height);
    }
    else
    {
        gint width0  = gdk_pixbuf_get_width(offscreen_buffer);
        gint height0 = gdk_pixbuf_get_height(offscreen_buffer);

        if (width > width0 || height > height0)
        {
            g_object_unref(offscreen_buffer);

            offscreen_buffer = gdk_pixbuf_new(GDK_COLORSPACE_RGB,
                                              FALSE, 8,
                                              MAX(width, width0),
                                              MAX(height, height0));
        }
    }

    if (offscreen_buffer == NULL) // gdk_pixbuf_new はアロケーションエラーで落ちない！
    {
        fprintf(stderr, "** Allocation Error: GdkPixbuf is too large!\n");
        exit(EXIT_FAILURE);
    }

    // オフスクリーンバッファを初期化する
    memset(gdk_pixbuf_get_pixels(offscreen_buffer),
           0x99,
           gdk_pixbuf_get_height(offscreen_buffer) * gdk_pixbuf_get_rowstride(offscreen_buffer));

    // キャンバスの大きさを変更する
    gtk_widget_set_size_request(da_canvas, width, height);

    return;
}

/**
 * [btn_step] ステップ実行する．
 */
static void
btn_step_on_clicked (GtkButton * button    G_GNUC_UNUSED,
                     gpointer    user_data G_GNUC_UNUSED)
{
    if (is_simulating == FALSE)
    {
        simulate();
        gtk_widget_queue_draw(da_canvas);
    }
    else
    {
        stop_simulation();
    }

    return;
}

/**
 * [btn_start] 実時間シミュレーションを開始する．
 */
static void
btn_start_on_clicked (GtkButton * button    G_GNUC_UNUSED,
                      gpointer    user_data G_GNUC_UNUSED)
{
    if (is_simulating == FALSE)
    {
        // 実時間シミュレーション用スレッドの動作を開始する
        g_static_mutex_unlock(&mutex_standby);

        is_simulating = TRUE;
    }
    else
    {
        stop_simulation();
    }

    return;
}

/**
 * [btn_stop] 実時間シミュレーションを停止する．
 */
static void
btn_stop_on_clicked (GtkButton * button    G_GNUC_UNUSED,
                     gpointer    user_data G_GNUC_UNUSED)
{
    stop_simulation();

    return;
}

/**
 * [btn_clear] セルフィールドの全セルの状態を 0 に設定する．
 */
static void
btn_clear_on_clicked (GtkButton * button    G_GNUC_UNUSED,
                      gpointer    user_data G_GNUC_UNUSED)
{
    if (is_simulating) stop_simulation();

    memset(cell_field[field_active_index], 0, sizeof(*cell_field));

    gtk_widget_queue_draw(da_canvas);

    generation = 0;

    return;
}

/**
 * [btn_info] セル配置を表示する．
 */
static void
btn_info_on_clicked (GtkButton * button    G_GNUC_UNUSED,
                     gpointer    user_data G_GNUC_UNUSED)
{
    const gint min_x = find_min_x(), max_x = find_max_x(), min_y = find_min_y(), max_y = find_max_y();
    gint i, j;

    if (min_x != -1)
    {
        g_print("int x = %d, y = %d;\nstatic const int array [][2] = {{%d,%d}", min_x, min_y, max_x - min_x, max_y - min_y);

        for (i = 1; i < FIELD_HEIGHT + 1; ++i)
        {
            for (j = 1; j < FIELD_WIDTH + 1; ++j)
            {
                if (cell_field[field_active_index][j][i])
                {
                    g_print(",{%d,%d}", j - min_x, i - min_y);
                }
            }
        }
        g_print("};\n");
    }

    return;
}

/**
 * Y 座標の最小値を探す．
 */
static gint
find_min_y (void)
{
    gint i, j;
    for (i = 1; i < FIELD_HEIGHT + 1; ++i)
        for (j = 1; j < FIELD_WIDTH + 1; ++j)
            if (cell_field[field_active_index][j][i])
                return i;
    return -1;
}

/**
 * Y 座標の最大値を探す．
 */
static gint
find_max_y (void)
{
    gint i, j;
    for (i = FIELD_HEIGHT + 1; i >= 1; --i)
        for (j = 1; j < FIELD_WIDTH + 1; ++j)
            if (cell_field[field_active_index][j][i])
                return i;
    return -1;
}

/**
 * X 座標の最小値を探す．
 */
static gint
find_min_x (void)
{
    gint i, j;
    for (j = 1; j < FIELD_WIDTH + 1; ++j)
        for (i = 1; i < FIELD_HEIGHT + 1; ++i)
            if (cell_field[field_active_index][j][i])
                return j;
    return -1;
}

/**
 * X 座標の最大値を探す．
 */
static gint
find_max_x (void)
{
    gint i, j;
    for (j = FIELD_WIDTH + 1; j >= 1; --j)
        for (i = 1; i < FIELD_HEIGHT + 1; ++i)
            if (cell_field[field_active_index][j][i])
                return j;
    return -1;
}

static gboolean
main_on_idle (gpointer user_data G_GNUC_UNUSED)
{
    static guint prev_gen = 0;

    if (prev_gen != generation) // 世代が変化していれば，再描画する
    {
        gchar str[16];

        // Generation の表示を更新する
        sprintf(str, "%d", generation);
        gtk_label_set_label(GTK_LABEL(lb_generation), str);

        // 画面を更新する
        gtk_widget_queue_draw(da_canvas);

        prev_gen = generation;
    }

    return TRUE;
}

/**
 * シミュレーションを停止する．
 */
static void
stop_simulation (void)
{
    if (is_simulating)
    {
        // 実時間シミュレーション用スレッドの動作を停止する
        g_static_mutex_lock(&mutex_standby);

        is_simulating = FALSE;
    }

    return;
}

/**
 * シミュレーション用のバックグラウンドスレッド
 */
static gpointer
simulation_thread (gpointer user_data G_GNUC_UNUSED)
{
    while (1)
    {
        g_static_mutex_lock(&mutex_standby);
        simulate();
        g_static_mutex_unlock(&mutex_standby);
        g_usleep(simulation_interval * 1000);
    }

    return NULL;
}

/**
 * ライフゲームのシミュレーションを行う．
 */
static void
simulate (void)
{
    const_field_t prev_field;
    field_t curr_field;
    int i, j, s;

    if (field_active_index == 0)
    {
        prev_field = (const_field_t) cell_field[0];
        curr_field = (field_t) cell_field[1];
    }
    else
    {
        prev_field = (const_field_t) cell_field[1];
        curr_field = (field_t) cell_field[0];
    }

    for (i = 1; i < FIELD_HEIGHT + 1; ++i)
    {
        for (j = 1; j < FIELD_WIDTH + 1; ++j)
        {
            s = prev_field[j-1][i-1]
              + prev_field[j-1][i]
              + prev_field[j-1][i+1]
              + prev_field[j][i-1]
              + prev_field[j][i+1]
              + prev_field[j+1][i-1]
              + prev_field[j+1][i]
              + prev_field[j+1][i+1];

            if (prev_field[j][i] == 1 && (s == 2 || s == 3))
            {
                curr_field[j][i] = 1;
            }
            else if (prev_field[j][i] == 0 && s == 3)
            {
                curr_field[j][i] = 1;
            }
            else
            {
                curr_field[j][i] = 0;
            }
        }
    }

    // フィールドの更新が完了したら，アクティブフィールドを交換する
    field_active_index = (field_active_index == 1 ? 0 : 1);

    ++generation; // 世代を更新する

    return;
}