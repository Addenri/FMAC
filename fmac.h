#ifndef __FMAC_H
#define __FMAC_H

#define IIR_MAX_ORDER 4                 //define max order of IIR filter
#define FIR_MAX_ORDER 40                //define max order of FIR filter

typedef struct                          // IIR filter struct
{
  unsigned short m;                     /*!< Num of A-coeffs (Feed-back) */
  unsigned short n;                     /*!< Num of B-coeffs (Feed-forward) */
  unsigned short a[IIR_MAX_ORDER];      /*!< A-coeffs */                  //Need to be inverted for loading in to FMAC X2 buffer
  unsigned short b[IIR_MAX_ORDER + 1];  /*!< B-coeffs */
  unsigned short shift;                 /*!< Shift */                     //Должен быть равен двоичному логарифму от делителя коэффициентов (если есть)
} IIR_struct;

typedef struct                          //FIR filter struct
{
  unsigned short tap;                   /*!< Num of coeffs */
  unsigned short coeffs[FIR_MAX_ORDER]; /*!< Coeffs of FIR filter */
} FIR_struct;

#endif /* __FMAC_H */
