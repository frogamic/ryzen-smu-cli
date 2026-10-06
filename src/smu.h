// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (C) 2026 Dominic Shelton <frogamic@protonmail.com>

#ifndef SMU_H
#define SMU_H

#include <libsmu.h>

smu_return_val smu_send_mp1(smu_obj_t *obj, unsigned int op, int arg, int *ret);

#endif // SMU_H
