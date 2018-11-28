#ifndef BITS_H_
#define BITS_H_

#define SET(bit)			(1 << bit)
#define UNSET(bit)			(0 << bit)
#define	set_bit(y,bit)		(y|=(1<<bit))
#define	clr_bit(y,bit)		(y&=~(1<<bit))
#define cpl_bit(y,bit) 		(y^=(1<<bit))
#define tst_bit(y,bit) 		(y&(1<<bit))
#define chg_nibh(y,nib)		(y = (y&0x0F)|(nib<<4))
#define chg_nibl(y,nib)		(y = ((y&0xF0) | (nib)))

#endif /* BITS_H_ */
