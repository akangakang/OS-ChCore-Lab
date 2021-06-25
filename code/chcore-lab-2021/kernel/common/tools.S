/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS), Shanghai Jiao Tong University (SJTU)
 * OS-Lab-2020 (i.e., ChCore) is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 *   PURPOSE.
 *   See the Mulan PSL v1 for more details.
 */

#include <common/asm.h>

BEGIN_FUNC(put32)
    str w1,[x0]
    ret
END_FUNC(put32)

BEGIN_FUNC(get32)
    ldr w0,[x0]
    ret
END_FUNC(put32)

BEGIN_FUNC(put64)
    str x1,[x0]
    ret
END_FUNC(put32)

BEGIN_FUNC(get64)
    ldr x0,[x0]
    ret
END_FUNC(put32)
