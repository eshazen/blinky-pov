
#include "twi.h"


/*
 * Initialize TWI interface
 */
void twi_init(void)
{

  /* initialize TWI clock: 100 kHz clock, TWPS = 0 => prescaler = 1 */
#if defined(TWPS0)
  /* has prescaler (mega128 & newer) */
  TWSR = 0;
#endif

#if F_CPU < 3600000UL
  TWBR = 10;			/* smallest TWBR value, see note [5] */
#else
  TWBR = (F_CPU / 100000UL - 16) / 2;
#endif

#ifdef TWI_PORT
  TWI_PORT = TWI_BITS;
#endif
}



int twi_write_bytes( uint8_t twi_addr, int len, uint8_t *dat)
{
  uint8_t sla, twcr, n = 0;
  int rv = 0;
  uint8_t twst;

 restart:
  if (n++ >= MAX_ITER)
    return -1;
 begin:

  sla = twi_addr;
  
  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); /* send start condition */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_REP_START:		/* OK, but should not happen */
    case TW_START:
      break;

    case TW_MT_ARB_LOST:	/* Note [9] */
      goto begin;

    default:
      return -1;		/* error: not in start condition */
				/* NB: do /not/ send stop condition */
    }

  /* send SLA+W */
  TWDR = sla | TW_WRITE;
  TWCR = _BV(TWINT) | _BV(TWEN); /* clear interrupt to start transmission */
  while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
  switch ((twst = TW_STATUS))
    {
    case TW_MT_SLA_ACK:
      break;

    case TW_MT_SLA_NACK:	/* nack during select: device busy writing */
      goto restart;

    case TW_MT_ARB_LOST:	/* re-arbitrate */
      goto begin;

    default:
      goto error;		/* must send stop condition */
    }
  
  for(; len > 0; len--) {
    TWDR = *dat++;
    TWCR = _BV(TWINT) | _BV(TWEN); /* start transmission */
    while ((TWCR & _BV(TWINT)) == 0) ; /* wait for transmission */
    switch ((twst = TW_STATUS))
      {
      case TW_MT_DATA_NACK:
	goto error;		/* device write protected -- Note [16] */

      case TW_MT_DATA_ACK:
	rv++;
	break;

      default:
	goto error;
      }
  }
 quit:
  TWCR = _BV(TWINT) | _BV(TWSTO) | _BV(TWEN); /* send stop condition */

  return rv;

 error:
  rv = -1;
  goto quit;
}

