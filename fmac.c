#include "fmac.h"

void fmac_iir_config(IIR_struct* iir)
{
  int buffsize = 1;

  //X2 for coeffs buffer
  FMAC->X2BUFCFG |= 0 << FMAC_X2BUFCFG_X2_BASE_Pos;
  FMAC->X2BUFCFG |= (iir->n + iir->m) << FMAC_X2BUFCFG_X2_BUF_SIZE_Pos;
  //X1 for input buffer
  FMAC->X1BUFCFG |= (iir->n + iir->m) << FMAC_X1BUFCFG_X1_BASE_Pos;             //N + M
  FMAC->X1BUFCFG |= (iir->n + buffsize) << FMAC_X1BUFCFG_X1_BUF_SIZE_Pos;       //N + d1
  FMAC->X1BUFCFG |= 0 << FMAC_X1BUFCFG_FULL_WM_Pos;
  //Y for output buffer
  FMAC->YBUFCFG |= (2 * iir->n + iir->m + buffsize) << FMAC_YBUFCFG_Y_BASE_Pos; //2 * N + M + d1
  FMAC->YBUFCFG |= (iir->m + buffsize) << FMAC_YBUFCFG_Y_BUF_SIZE_Pos;          //M + d2
  FMAC->YBUFCFG |= 0 << FMAC_YBUFCFG_EMPTY_WM_Pos;

  //Cleanup buffer
  FMAC->PARAM &= ~FMAC_PARAM_FUNC;
  FMAC->PARAM |= FMAC_PARAM_FUNC_1 + FMAC_PARAM_FUNC_0;                         //Load Y
  FMAC->PARAM &= ~FMAC_PARAM_P;
  FMAC->PARAM |= iir->m << FMAC_PARAM_P_Pos;
  FMAC->PARAM &= ~FMAC_PARAM_Q;
  FMAC->PARAM &= ~FMAC_PARAM_R;
  FMAC->PARAM |= FMAC_PARAM_START;
  for(int i = 0; i < (iir->m); i++)
  {
    FMAC->WDATA = 0;
  }

  //Load filter coeffs in advance
  FMAC->PARAM &= ~FMAC_PARAM_FUNC;
  FMAC->PARAM |= FMAC_PARAM_FUNC_1;                                             //Load X2
  FMAC->PARAM &= ~FMAC_PARAM_P;
  FMAC->PARAM |= (iir->n) << FMAC_PARAM_P_Pos;
  FMAC->PARAM &= ~FMAC_PARAM_Q;
  FMAC->PARAM |= (iir->m) << FMAC_PARAM_Q_Pos;
  FMAC->PARAM &= ~FMAC_PARAM_R;
  FMAC->PARAM |= FMAC_PARAM_START;

  for(int i = 0; i < (iir->n); i++)
  {
    FMAC->WDATA = iir->b[i];
  }

  for(int i = 0; i < (iir->m); i++)
  {
    FMAC->WDATA = -(iir->a[i]);                                 //Feedback coeffs must be inverted
  }

  //Switch function to IIR
  FMAC->PARAM &= ~FMAC_PARAM_FUNC;
  FMAC->PARAM |= FMAC_PARAM_FUNC_3 + FMAC_PARAM_FUNC_0;         //FUNC = 9 (IIR filter)
  FMAC->PARAM &= ~FMAC_PARAM_P;
  FMAC->PARAM |= iir->n << FMAC_PARAM_P_Pos;                    //P = N (feed-forward coeffs.)
  FMAC->PARAM &= ~FMAC_PARAM_Q;
  FMAC->PARAM |= iir->m << FMAC_PARAM_Q_Pos;                    //Q = M (feed-back coeffs.)
  FMAC->PARAM &= ~FMAC_PARAM_R;
  FMAC->PARAM |= iir->shift << FMAC_PARAM_R_Pos;                //R = shift

  FMAC->CR &= ~FMAC_CR_WIEN;                                    //Disable writing interrupts
  FMAC->CR &= ~FMAC_CR_RIEN;                                    //...and same for reading
  FMAC->CR |= FMAC_CR_DMAREN + FMAC_CR_CLIPEN;                  //Enable DMA requests, clipping
  FMAC->PARAM |= FMAC_PARAM_START;                              //Enable filter
}

void fmac_fir_config(FIR_struct* fir)
{
  int buffsize = 11;
  //X2 for coeffs buffer
  FMAC->X2BUFCFG |= 0 << FMAC_X2BUFCFG_X2_BASE_Pos;
  FMAC->X2BUFCFG |= (fir->tap) << FMAC_X2BUFCFG_X2_BUF_SIZE_Pos;
  //X1 for input buffer
  FMAC->X1BUFCFG |= (fir->tap) << FMAC_X1BUFCFG_X1_BASE_Pos;
  FMAC->X1BUFCFG |= (fir->tap + buffsize) << FMAC_X1BUFCFG_X1_BUF_SIZE_Pos;     //N + buffsize
  //Y for output buffer
  FMAC->YBUFCFG |= (2 * fir->tap + buffsize) << FMAC_YBUFCFG_Y_BASE_Pos;        //2 * N + buffsize
  FMAC->YBUFCFG |= buffsize << FMAC_YBUFCFG_Y_BUF_SIZE_Pos;
  //Load filter coeffs
  FMAC->PARAM &= ~FMAC_PARAM_FUNC;
  FMAC->PARAM |= FMAC_PARAM_FUNC_1;
  FMAC->PARAM &= ~FMAC_PARAM_P;
  FMAC->PARAM |= fir->tap << FMAC_PARAM_P_Pos;
  FMAC->PARAM |= FMAC_PARAM_START;
  for(int i = 0; i < fir->tap; i++)
  {
    FMAC->WDATA = fir->coeffs[i];
  }

  //Switch function to FIR
  FMAC->PARAM &= ~FMAC_PARAM_FUNC;
  FMAC->PARAM |= FMAC_PARAM_FUNC_3;                                             //FUNC = 8 (FIR filter) - Convolution
  FMAC->PARAM &= ~FMAC_PARAM_P;
  FMAC->PARAM |= fir->tap << FMAC_PARAM_P_Pos;                                  //P = N (feed-forward coeffs.)
  FMAC->PARAM &= ~FMAC_PARAM_Q;
  //FMAC->PARAM |= fir->shift << FMAC_PARAM_R_Pos;                              //R = shift

  FMAC->CR &= ~FMAC_CR_WIEN;                                                    //Disable writing interrupts
  FMAC->CR &= ~FMAC_CR_RIEN;                                                    //...and same for reading
  FMAC->CR |= FMAC_CR_DMAREN + FMAC_CR_CLIPEN;                                  //Enable DMA requests, clipping
  FMAC->PARAM |= FMAC_PARAM_START;                                              //Enable filter
}
