/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Compatibility bits for kernel 5.15 that the rtw88 fork expects:
 *  - guard() from linux/cleanup.h (kernel 6.5), only the mutex and
 *    spinlock_irqsave classes used by the SDIO code,
 *  - FIELD_PREP_CONST() from linux/bitfield.h,
 *  - the Realtek ids from linux/mmc/sdio_ids.h.
 */
#ifndef __RTW_COMPAT_GUARD_H
#define __RTW_COMPAT_GUARD_H

#include <linux/bitfield.h>
#include <linux/mmc/sdio_ids.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>

#ifndef FIELD_PREP_CONST
#define FIELD_PREP_CONST(_mask, _val)					\
	(((typeof(_mask))(_val) << __bf_shf(_mask)) & (_mask))
#endif

#ifndef SDIO_VENDOR_ID_REALTEK
#define SDIO_VENDOR_ID_REALTEK			0x024c
#endif
#ifndef SDIO_DEVICE_ID_REALTEK_RTW8723BS
#define SDIO_DEVICE_ID_REALTEK_RTW8723BS	0xb723
#endif

struct rtw_guard_mutex {
	struct mutex *lock;
};

static inline struct rtw_guard_mutex rtw_guard_mutex_ctor(struct mutex *lock)
{
	struct rtw_guard_mutex g = { .lock = lock };

	mutex_lock(lock);
	return g;
}

static inline void rtw_guard_mutex_dtor(struct rtw_guard_mutex *g)
{
	mutex_unlock(g->lock);
}

struct rtw_guard_spinlock_irqsave {
	spinlock_t *lock;
	unsigned long flags;
};

static inline struct rtw_guard_spinlock_irqsave
rtw_guard_spinlock_irqsave_ctor(spinlock_t *lock)
{
	struct rtw_guard_spinlock_irqsave g = { .lock = lock };

	spin_lock_irqsave(lock, g.flags);
	return g;
}

static inline void
rtw_guard_spinlock_irqsave_dtor(struct rtw_guard_spinlock_irqsave *g)
{
	spin_unlock_irqrestore(g->lock, g->flags);
}

#define __RTW_GUARD_CAT(a, b)	a##b
#define __RTW_GUARD_NAME(l)	__RTW_GUARD_CAT(__rtw_guard_, l)

#ifndef guard
#define guard(_name)							\
	struct rtw_guard_##_name __RTW_GUARD_NAME(__COUNTER__)		\
	__attribute__((cleanup(rtw_guard_##_name##_dtor))) =		\
		rtw_guard_##_name##_ctor
#endif

#endif /* __RTW_COMPAT_GUARD_H */
