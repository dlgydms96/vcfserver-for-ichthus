#pragma once

#define BITS_PER_BYTE		8
#define BITS_PER_LONG		32
#define BIT(nr)				(1UL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define test_flags(mask, flags, addr)		\
  (((*addr) & mask) == flags)

#define test_bit(nr, addr)					\
  (1UL & (addr[BIT_WORD(nr)] >> (nr & (BITS_PER_LONG-1))))

#define set_bit(nr, addr)					\
  do {								\
    unsigned long mask = BIT_MASK(nr);				\
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);	\
    *p |= mask;							\
  } while (0)

#define clear_bit(nr, addr)					\
  do {								\
    unsigned long mask = BIT_MASK(nr);				\
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);	\
    *p &= ~mask;						\
  } while (0)

#define change_bit(nr, addr)					\
  do {								\
    unsigned long mask = BIT_MASK(nr);				\
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);	\
    *p ^= mask;							\
  } while (0)

#define clear_mask(mask, addr)			\
  do {						\
    *addr &= ~mask;				\
  } while (0)

#define test_and_set_bit(nr, addr, ret)				\
  do {								\
    unsigned long mask = BIT_MASK(nr);				\
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);	\
    unsigned long old = *p;					\
    *p = old & ~mask;						\
    ret = (old & mask) != 0;					\
  } while (0)

#define test_and_clear_bit(nr, addr, ret)			\
  do {								\
    unsigned long mask = BIT_MASK(nr);				\
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);	\
    unsigned old = *p;						\
    *p = old & ~mask;						\
    ret = (old & mask) != 0;					\
  } while (0)
