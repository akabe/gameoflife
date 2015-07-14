#ifndef INCLUDE_GUARD_9D9A65F0_B528_11E2_A877_8C736E05BBBD
#define INCLUDE_GUARD_9D9A65F0_B528_11E2_A877_8C736E05BBBD

#ifdef __cplusplus
extern "C" {
#endif

#define FIELD_WIDTH                 (650)
#define FIELD_HEIGHT                (600)
#define DEFAULT_INTERVAL            (10)
#define DEFAULT_LAYOUT_CELL_SIZE    (1)
#define DEFAULT_LAYOUT_CELL_SPACING (0)

typedef const guint8 (* restrict const_field_t) [FIELD_HEIGHT + 2];
typedef guint8 (* restrict field_t) [FIELD_HEIGHT + 2];

extern gint   field_active_index;
extern guint8 cell_field [2][FIELD_WIDTH + 2][FIELD_HEIGHT + 2];

#ifdef __cplusplus
}
#endif

#endif /* !INCLUDE_GUARD_9D9A65F0_B528_11E2_A877_8C736E05BBBD */
