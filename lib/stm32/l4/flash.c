/** @defgroup flash_file FLASH
 *
 * @ingroup STM32L4xx
 *
 * @brief <b>libopencm3 STM32L4xx FLASH</b>
 *
 * @version 1.0.0
 *
 * Benjamin Levine <benjamin@jesco.karoo.co.uk>
 *
 * @date 12 February 2016
 *
 * This library supports the FLASH memory controller in the STM32L4
 * series of ARM Cortex Microcontrollers by ST Microelectronics.
 *
 * For the STM32L4xx, accessing FLASH memory is described briefly in
 * section 3 of the STM32L4x6 Reference Manual.
 *
 * LGPL License Terms @ref lgpl_license
 */

/*
 * This file is part of the libopencm3 project.
 *
 * Copyright (C) 2016 Benjamin Levine <benjamin@jesco.karoo.co.uk>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

/**@{*/

#include <libopencm3/stm32/flash.h>

/** @brief Clear the Programming Error Status Flag
 */
void flash_clear_pgperr_flag(void)
{
	FLASH_SR |= FLASH_SR_PROGERR;
}

/** @brief Wait until Last Operation has Ended
 * This loops indefinitely until an operation (write or erase) has completed
 * by testing the busy flag.
 */
void flash_wait_for_last_operation(void)
{
	while ((FLASH_SR & FLASH_SR_BSY) == FLASH_SR_BSY);
}

/** @brief Clear the Programming Sequence Error Flag
 * This flag is set when incorrect programming configuration has been made.
 */
void flash_clear_pgserr_flag(void)
{
	FLASH_SR |= FLASH_SR_PGSERR;
}

/** @brief Clear the Programming Alignment Error Flag
 */
void flash_clear_pgaerr_flag(void)
{
	FLASH_SR |= FLASH_SR_PGAERR;
}

/** @brief Clear the Write Protect Error Flag
 */
void flash_clear_wrperr_flag(void)
{
	FLASH_SR |= FLASH_SR_WRPERR;
}

/** @brief Clear All Status Flags
 * Program error, end of operation, write protect error, busy.
 */
void flash_clear_status_flags(void)
{
	flash_clear_pgserr_flag();
	flash_clear_pgaerr_flag();
	flash_clear_wrperr_flag();
	flash_clear_pgperr_flag();
	flash_clear_eop_flag();
}

/** @brief Lock the Option Byte Access
 * This disables write access to the option bytes. It is locked by default on
 * reset.
 */
void flash_lock_option_bytes(void)
{
	FLASH_CR |= FLASH_CR_OPTLOCK;
}

/** @brief Program a 32 bit Word to FLASH
 * This performs all operations necessary to program a 32 bit word to FLASH
 * memory. The program error flag should be checked separately for the event
 * that memory was not properly erased.
 *
 * @param[in] address Starting address in Flash.
 * @param[in] data word to write
 */
void flash_program_word(uint32_t address, uint32_t data)
{
	/* Ensure that all flash operations are complete. */
	flash_wait_for_last_operation();

	/* Enable writes to flash. */
	FLASH_CR |= FLASH_CR_PG;

	/* Program the word. */
	MMIO32(address) = data;

	/* Wait for the write to complete. */
	flash_wait_for_last_operation();

	/* Disable writes to flash. */
	FLASH_CR &= ~FLASH_CR_PG;
}

/** @brief Program a Data Block to FLASH
 * This programs an arbitrary length data block to FLASH memory.
 * The program error flag should be checked separately for the event that
 * memory was not properly erased.
 * @param[in] address Starting address in Flash.
 * @param[in] data Pointer to start of data block.
 * @param[in] len Length of data block.
 */
void flash_program(uint32_t address, uint8_t *data, uint32_t len)
{
	/* TODO: Use dword and word size program operations where possible for
	 * turbo speed.
	 */
	uint32_t i;
	for (i = 0; i < len; i++) {
		flash_program_word(address+i, data[i]);
	}
}

/** @brief Erase a page of FLASH
 * @param[in] page (0 - 255 for bank 1, 256-511 for bank 2)
 */
void flash_erase_page(uint32_t page)
{
	flash_wait_for_last_operation();

	/* page and bank are contiguous bits */
	FLASH_CR &= ~((FLASH_CR_PNB_MASK << FLASH_CR_PNB_SHIFT) | FLASH_CR_BKER);
	FLASH_CR |= page << FLASH_CR_PNB_SHIFT;
	FLASH_CR |= FLASH_CR_PER;
	FLASH_CR |= FLASH_CR_START;

	flash_wait_for_last_operation();
	FLASH_CR &= ~FLASH_CR_PER;
}

/** @brief Erase All FLASH
 * This performs all operations necessary to erase all sectors in the FLASH
 * memory.
 */
void flash_erase_all_pages(void)
{
	flash_wait_for_last_operation();

	FLASH_CR |= FLASH_CR_MER1 | FLASH_CR_MER2;
	FLASH_CR |= FLASH_CR_START;

	flash_wait_for_last_operation();
	FLASH_CR &= ~FLASH_CR_MER1 & ~FLASH_CR_MER2;
}

/** @brief Program the Option Bytes
 * This performs all operations necessary to program the option bytes.
 * The option bytes do not need to be erased first.
 * @param[in] data value to be programmed.
 */
void flash_program_option_bytes(uint32_t data)
{
	flash_wait_for_last_operation();

	if (FLASH_CR & FLASH_CR_OPTLOCK) {
		flash_unlock_option_bytes();
	}

	FLASH_OPTR = data & ~0x3;
	FLASH_OPTR |= FLASH_CR_OPTSTRT;
	flash_wait_for_last_operation();
}
/**@}*/

