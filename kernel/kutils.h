// SPDX-License-Identifier: GPL-2.0
// vim: set ts=8 sw=8 noet tw=80 cc=80 fo+=t :

#ifndef KUTILS_H_
#define KUTILS_H_

#ifndef prfx_fmt
#define prfx_fmt ""
#endif

#ifdef DEBUG
/* Debug builds: prefix + function name + line number, then the caller's format.
 */
#define pr_fmt(fmt) prfx_fmt "%s:%d: " fmt, __func__, __LINE__
#else
/* Release builds: prefix only. */
#define pr_fmt(fmt) prfx_fmt fmt
#endif

#endif /* KUTILS_H_ */