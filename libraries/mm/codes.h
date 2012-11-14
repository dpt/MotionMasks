/* codes.h -- Motion Mask byte encoding */

#ifndef MM_CODES_H
#define MM_CODES_H

#define MASK(N)                           ((1 << (N)) - 1)

#define VAL(ID)                           ((0x80 >> (ID)) & 0xFF)

#define LOG2MAXSOURCES                    1
#define MAXSOURCES                        (1 << LOG2MAXSOURCES)

/* IDs (== CLZ of type) */
#define MMCopy_ID                         0
#define MMBlendConst_ID                   1
#define MMBlendArray_ID                   2
#define MMCopyLong_ID                     3
#define MMBlendConstLong_ID               4
#define MMBlendArrayLong_ID               5
#define MMUndefined_ID                    6 /* not yet used */
#define MMSetSource_ID                    7
#define MMStop_ID                         8

/* copy (short) */
#define MMCopy_ID_BITS                    (MMCopy_ID + 1)
#define MMCopy_VAL                        VAL(MMCopy_ID)
#define MMCopy_WIDTH                      8
#define MMCopy_SOURCE_BITS                1
#define MMCopy_SOURCE_SHIFT               6
#define MMCopy_SOURCE_MASK                (MASK(MMCopy_SOURCE_BITS) << MMCopy_SOURCE_SHIFT)
#define MMCopy_LENGTH_BITS                (MMCopy_WIDTH - MMCopy_SOURCE_BITS - MMCopy_ID_BITS)
#define MMCopy_LENGTH_SHIFT               0
#define MMCopy_LENGTH_MASK                (MASK(MMCopy_LENGTH_BITS) << MMCopy_LENGTH_SHIFT)
#define MMCopy_LENGTH_MAX                 (1 << MMCopy_LENGTH_BITS)

/* blend const (short) */
#define MMBlendConst_ID_BITS              (MMBlendConst_ID + 1)
#define MMBlendConst_VAL                  VAL(MMBlendConst_ID)
#define MMBlendConst_WIDTH                8
#define MMBlendConst_LENGTH_BITS          (MMBlendConst_WIDTH - MMBlendConst_ID_BITS)
#define MMBlendConst_LENGTH_SHIFT         0
#define MMBlendConst_LENGTH_MASK          (MASK(MMBlendConst_LENGTH_BITS) << MMBlendConst_LENGTH_SHIFT)
#define MMBlendConst_LENGTH_MAX           (1 << MMBlendConst_LENGTH_BITS)

/* blend array (short) */
#define MMBlendArray_ID_BITS              (MMBlendArray_ID + 1)
#define MMBlendArray_VAL                  VAL(MMBlendArray_ID)
#define MMBlendArray_WIDTH                8
#define MMBlendArray_LENGTH_BITS          (MMBlendArray_WIDTH - MMBlendArray_ID_BITS)
#define MMBlendArray_LENGTH_SHIFT         0
#define MMBlendArray_LENGTH_MASK          (MASK(MMBlendArray_LENGTH_BITS) << MMBlendArray_LENGTH_SHIFT)
#define MMBlendArray_LENGTH_MAX           (1 << MMBlendArray_LENGTH_BITS)

/* copy (long) */
#define MMCopyLong_ID_BITS                (MMCopyLong_ID + 1)
#define MMCopyLong_VAL                    VAL(MMCopyLong_ID)
#define MMCopyLong_WIDTH                  16
#define MMCopyLong_SOURCE_BITS            1
#define MMCopyLong_SOURCE_SHIFT           11
#define MMCopyLong_SOURCE_MASK            (MASK(MMCopyLong_SOURCE_BITS) << MMCopyLong_SOURCE_SHIFT)
#define MMCopyLong_LENGTH_BITS            (MMCopyLong_WIDTH - MMCopyLong_SOURCE_BITS - MMCopyLong_ID_BITS)
#define MMCopyLong_LENGTH_SHIFT           0
#define MMCopyLong_LENGTH_MASK            (MASK(MMCopyLong_LENGTH_BITS) << MMCopyLong_LENGTH_SHIFT)
#define MMCopyLong_LENGTH_MAX             (1 << MMCopyLong_LENGTH_BITS)

/* blend const (long) */
#define MMBlendConstLong_ID_BITS          (MMBlendConstLong_ID + 1)
#define MMBlendConstLong_VAL              VAL(MMBlendConstLong_ID)
#define MMBlendConstLong_WIDTH            16
#define MMBlendConstLong_LENGTH_BITS      (MMBlendConstLong_WIDTH - MMBlendConstLong_ID_BITS)
#define MMBlendConstLong_LENGTH_SHIFT     0
#define MMBlendConstLong_LENGTH_MASK      (MASK(MMBlendConstLong_LENGTH_BITS) << MMBlendConstLong_LENGTH_SHIFT)
#define MMBlendConstLong_LENGTH_MAX       (1 << MMBlendConstLong_LENGTH_BITS)

/* blend array (long) */
#define MMBlendArrayLong_ID_BITS          (MMBlendArrayLong_ID + 1)
#define MMBlendArrayLong_VAL              VAL(MMBlendArrayLong_ID)
#define MMBlendArrayLong_WIDTH            16
#define MMBlendArrayLong_LENGTH_BITS      (MMBlendArrayLong_WIDTH - MMBlendArrayLong_ID_BITS)
#define MMBlendArrayLong_LENGTH_SHIFT     0
#define MMBlendArrayLong_LENGTH_MASK      (MASK(MMBlendArrayLong_LENGTH_BITS) << MMBlendArrayLong_LENGTH_SHIFT)
#define MMBlendArrayLong_LENGTH_MAX       (1 << MMBlendArrayLong_LENGTH_BITS)

/* set source */
#define MMSetSource_ID_BITS               (MMSetSource_ID + 1)
#define MMSetSource_VAL                   VAL(MMSetSource_ID)
#define MMSetSource_WIDTH                 16
#define MMSetSource_SOURCE_BITS           4
#define MMSetSource_SOURCE1_BITS          MMSetSource_SOURCE_BITS
#define MMSetSource_SOURCE1_SHIFT         0
#define MMSetSource_SOURCE1_MASK          (MASK(MMSetSource_SOURCE1_BITS) << MMSetSource_SOURCE1_SHIFT)
#define MMSetSource_SOURCE2_BITS          MMSetSource_SOURCE_BITS
#define MMSetSource_SOURCE2_SHIFT         4
#define MMSetSource_SOURCE2_MASK          (MASK(MMSetSource_SOURCE2_BITS) << MMSetSource_SOURCE2_SHIFT)

/* Stop (end of line) */
#define MMStop_VAL                        VAL(MMStop_ID)

#endif /* MM_CODES_H */
