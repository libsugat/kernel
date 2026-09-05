#ifndef KERNEL_BITS_H
#define KERNEL_BITS_H

#ifndef __ASSEMBLER__

#include <kernel/types.h>

#define BIT_TYPE(type, nr)  (((type)1) << (nr))

#define GETMASK_TYPE(type, h, l) \
    (((((type)~0ULL)) << l ) \
    & ((((type)~0ULL)) >> ((sizeof(type) * 8) - 1 - (h))))

#define BIT(nr)  BIT_TYPE(reg_t, nr)
#define GETMASK(h, l) GETMASK_TYPE(reg_t, h, l)

#define BIT_U8(nr)   BIT_TYPE(u8, nr)
#define BIT_U16(nr)  BIT_TYPE(u16, nr)
#define BIT_U32(nr)  BIT_TYPE(u32, nr)
#define BIT_U64(nr)  BIT_TYPE(u64, nr)

#define GETMASK_U8(h, l) GETMASK_TYPE(u8, h, l)
#define GETMASK_U16(h, l) GETMASK_TYPE(u16, h, l)
#define GETMASK_U32(h, l) GETMASK_TYPE(u32, h, l)
#define GETMASK_U64(h, l) GETMASK_TYPE(u64, h, l)

#ifndef typeof
#define typeof __typeof__
#endif

/**
 * FIELD_LSB - Returns the lowest set bit index of a mask.
 */
#define FIELD_LSB(mask)            ((typeof(mask))__builtin_ctzll(mask))

/**
 * FIELD_GET - Extracts a field from a raw register value.
 * @val:  The raw register value read from MMIO/Port
 * @mask: The mask covering the field [h:l]
 */
#define FIELD_GET(val, mask) \
    (   (typeof(mask)) (((val) & (mask)) >> FIELD_LSB(mask))    )

/**
 * FIELD_PREP - Packs a raw value into a field position.
 * @mask: The mask defining the target field [h:l]
 * @val:  The value to pack into the field
 */
#define FIELD_PREP(mask, val) \
    (   ((typeof(mask))(val) << FIELD_LSB(mask)) & (mask)  )

/**
 * FIELD_MODIFY - Returns 'reg' with the bitfield specified by 'mask' replaced by 'val'.
 * @reg:  Original register / variable value.
 * @mask: Target bitfield mask.
 * @val:  Raw value to write into the bitfield.
 */
#define FIELD_MODIFY(reg, mask, val) \
    (((reg) & ~(mask)) | FIELD_PREP(mask, val))

/**
 * FIELD_ASSIGN - Assigns 'val' into 'var''s bitfield in-place.
 * @var:  Variable to update in-place.
 * @mask: Target bitfield mask.
 * @val:  Raw value to write into the bitfield.
 */
#define FIELD_ASSIGN(var, mask, val) \
    ((var) = FIELD_MODIFY(var, mask, val))

#endif // __ASSEMBLER__
#endif // KERNEL_BITS_H
