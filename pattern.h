#ifndef INCLUDE_GUARD_B0CB549E_B60F_11E2_A91F_8C736E05BBBD
#define INCLUDE_GUARD_B0CB549E_B60F_11E2_A91F_8C736E05BBBD

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const gchar * name;
    void (* func) (void);
    gint x;
    gint y;
} pattern_t;

extern const gsize     n_patterns;
extern const pattern_t patterns [];

#ifdef __cplusplus
}
#endif

#endif /* !INCLUDE_GUARD_B0CB549E_B60F_11E2_A91F_8C736E05BBBD */
