/* flash.c */
/*  {{{
 *
 * Copyright (C) 2008 Nélio Laranjeiro
 *
 * APBTeam:
 *        Web: http://apbteam.org/
 *      Email: team AT apbteam DOT org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * }}} */
#include "flash.h"
#include "modules/proto/proto.h"
#include "modules/spi/spi.h"

static flash_t flash_global; 

/** Flash access.
  * The flash contains an address of 21 bits in a range from 0x0-0x1fffff.
  * This function shall access the memory directly by the SPI.
  * \param  addr  the address to provide to the flash memory.
  */
void
flash_address (uint32_t addr)
{
    /* The address must be sent */
    spi_send ((addr >> 16) & 0x1f);
    spi_send (addr >> 8);
    spi_send (addr);
}

/** Erase the memory.
  * \param  erase_type  the erase type..
  * \param  start_addr  the start address.
  */
void
flash_erase (uint8_t cmd, uint32_t start_addr)
{
    /* send the command. */
    spi_send (cmd);

    /* verify if the cmd is the full erase. */
    if (cmd != FLASH_ERASE_FULL)
      {
	/* Send the start address */
	flash_address (start_addr);
      }
}

/** Initialise the flsah memory.
  */
void
flash_init (void)
{
    uint8_t rsp[3];

    flash_global.addr = 0x0;
    AC_FLASH_PORT |= _BV(AC_FLASH_BIT_SS);
    AC_FLASH_DDR |= _BV(AC_FLASH_BIT_SS);

    /* send the read-ID instruction. */
    spi_init (SPI_IT_DISABLE | SPI_ENABLE | SPI_MASTER | SPI_MSB_FIRST
	      | SPI_MASTER | SPI_CPOL_FALLING | SPI_CPHA_SETUP
	      | SPI_FOSC_DIV16);
    AC_FLASH_PORT &= ~_BV(AC_FLASH_BIT_SS);
    spi_send (FLASH_READ_ID); 
    rsp[0] = spi_recv ();
    rsp[1] = spi_recv ();
    rsp[2] = spi_recv ();
    AC_FLASH_PORT |= _BV(AC_FLASH_BIT_SS);

    proto_send3b ('f',rsp[0], rsp[1], rsp[2]);
    /* TODO: disable flash usage if no flash is found? */

    /* Search for the next address to start writting. */
    /*for (flash_global.addr = 0, rsp = 0xFF; rsp != 0xFF; flash_global.addr +=
	 FLASH_PAGE_SIZE - 1)
      {
	rsp = flash_read ();
      }
      */
}

/** Write in the flash byte provided in parameter.
  * \param  data  the buffer to store the data.
  */
void
flash_write (uint8_t data)
{
    spi_init (SPI_IT_DISABLE | SPI_ENABLE | SPI_MSB_FIRST | SPI_MASTER |
	      SPI_CPOL_RISING | SPI_CPHA_SAMPLE | SPI_FOSC_DIV2); 

    /* Write instruction. */
    spi_send (FLASH_WRITE);
    flash_address (flash_global.addr);
    spi_send (data);
    /* increment the address and verify if the address is in the range of the
     * flash memory */
    FLASH_ADDRESS_INC_MASK(flash_global.addr);
}

/** Read the data at the address provided.
  * \return  the data read.
  */
uint8_t
flash_read (void)
{
    uint8_t data;

    spi_init (SPI_IT_DISABLE | SPI_ENABLE | SPI_MSB_FIRST | SPI_MASTER |
	      SPI_CPOL_RISING | SPI_CPHA_SAMPLE | SPI_FOSC_DIV2); 

    /* Send the read instruction. */
    spi_send (FLASH_READ);
    flash_address (flash_global.addr);
    data = spi_recv ();

    /* increment the address and verify if the address is in the range of the
     * flash memory */
    FLASH_ADDRESS_INC_MASK(flash_global.addr);
    return data;
}

/** Read a data from the flash memory from the address provided and for a
 * length of the number of bytes provided.
 * \param  address at wich the data should be read.
 * \param  buffer  the buffer to fill with the read data.
 * \param  length  the length of the data to read.
 */
void
flash_read_array (uint32_t addr, uint8_t *buffer, uint32_t length);

/** Write in the flash byte provided in parameter.
  * \param  addr  the address to store the data.
  * \param  data  the array to store.
  * \param  length  the array length
  */
void
flash_write_array (uint32_t addr, uint8_t *data, uint32_t length);

/** Read the memory from the address automaticaly managed, the offset of the
 * data shall be provided to get the data.
 * \param offset  the offset from the current address managed.
 * \return data  read from the memory.
 */
uint8_t
flash_read_managed (uint32_t offset);

/** Read an array of data from the memory starting at the address less the
 * offset provided in parameters. The data read will be stored in the buffer
 * provided in memory too. The buffer shall have at list the length of the
 * offset provided.
 * \param  offset  the offset of the current position to read the data.
 * \param  buffer  the buffer to store the data.
 */
void
flash_read_managed_array (uint32_t offset, uint8_t *buffer);

/** Write a data with a managed array.
  * \param  data to store in the memory.
  */
void
flash_write_managed (uint8_t data);

/** Write an array of data to the flash memory. The length of the array shall
 * be provided.
 * \param  buffer  the buffer containing the data to write.
 * \param  length  the data length to write.
 */
void
flash_write_managed_array (uint8_t *buffer, uint8_t length);

