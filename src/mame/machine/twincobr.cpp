// license:BSD-3-Clause
// copyright-holders:Quench
/****************************************************************************
 *  Twin Cobra                                                              *
 *  Communications and memory functions between shared CPU memory spaces    *
 ****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/tms32010/tms32010.h"
#include "includes/twincobr.h"


#define LOG_DSP_CALLS 0
#define LOG(x) do { if (LOG_DSP_CALLS) logerror x; } while (0)


INTERRUPT_GEN_MEMBER(twincobr_state::twincobr_interrupt)
{
	if (m_intenable) {
		m_intenable = 0;
		device.execute().set_input_line(M68K_IRQ_4, HOLD_LINE);
	}
}

INTERRUPT_GEN_MEMBER(twincobr_state::wardner_interrupt)
{
	if (m_intenable) {
		m_intenable = 0;
		device.execute().set_input_line(0, HOLD_LINE);
	}
}


WRITE16_MEMBER(twincobr_state::twincobr_dsp_addrsel_w)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Top three bits of data need to be shifted left 3 places */
	/*  to select which memory bank from main CPU address */
	/*  space to use */
	/* Lower thirteen bits of this data is shifted left one position */
	/*  to move it to an even address word boundary */

	m_main_ram_seg = ((data & 0xe000) << 3);
	m_dsp_addr_w   = ((data & 0x1fff) << 1);

	LOG(("DSP PC:%04x IO write %04x (%08x) at port 0\n",space.device().safe_pcbase(),data,m_main_ram_seg + m_dsp_addr_w));
}

READ16_MEMBER(twincobr_state::twincobr_dsp_r)
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	uint16_t input_data = 0;
	switch (m_main_ram_seg) {
		case 0x30000:
		case 0x40000:
		case 0x50000:  {address_space &mainspace = m_maincpu->space(AS_PROGRAM);
						input_data = mainspace.read_word(m_main_ram_seg + m_dsp_addr_w);
						break;}
		default:        logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n",space.device().safe_pcbase(),m_main_ram_seg + m_dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO read %04x at %08x (port 1)\n",space.device().safe_pcbase(),input_data,m_main_ram_seg + m_dsp_addr_w));
	return input_data;
}

WRITE16_MEMBER(twincobr_state::twincobr_dsp_w)
{
	/* Data written to main CPU RAM via DSP IO port 1 */
	m_dsp_execute = 0;
	switch (m_main_ram_seg) {
		case 0x30000:   if ((m_dsp_addr_w < 3) && (data == 0)) m_dsp_execute = 1;
		case 0x40000:
		case 0x50000:  {address_space &mainspace = m_maincpu->space(AS_PROGRAM);
						mainspace.write_word(m_main_ram_seg + m_dsp_addr_w, data);
						break;}
		default:        logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n",space.device().safe_pcbase(),m_main_ram_seg + m_dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO write %04x at %08x (port 1)\n",space.device().safe_pcbase(),data,m_main_ram_seg + m_dsp_addr_w));
}

WRITE16_MEMBER(twincobr_state::wardner_dsp_addrsel_w)
{
	/* This sets the main CPU RAM address the DSP should */
	/*  read/write, via the DSP IO port 0 */
	/* Lower twelve bits of this data is shifted left one position */
	/*  to move it to an even address boundary */

	m_main_ram_seg =  (data & 0xe000);
	m_dsp_addr_w   = ((data & 0x07ff) << 1);

	if (m_main_ram_seg == 0x6000) m_main_ram_seg = 0x7000;

	LOG(("DSP PC:%04x IO write %04x (%08x) at port 0\n",space.device().safe_pcbase(),data,m_main_ram_seg + m_dsp_addr_w));
}

READ16_MEMBER(twincobr_state::wardner_dsp_r)
{
	/* DSP can read data from main CPU RAM via DSP IO port 1 */

	uint16_t input_data = 0;
	switch (m_main_ram_seg) {
		case 0x7000:
		case 0x8000:
		case 0xa000:   {address_space &mainspace = m_maincpu->space(AS_PROGRAM);
						input_data =  mainspace.read_byte(m_main_ram_seg + (m_dsp_addr_w + 0))
									| (mainspace.read_byte(m_main_ram_seg + (m_dsp_addr_w + 1)) << 8);
						break;}
		default:        logerror("DSP PC:%04x Warning !!! IO reading from %08x (port 1)\n",space.device().safe_pcbase(),m_main_ram_seg + m_dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO read %04x at %08x (port 1)\n",space.device().safe_pcbase(),input_data,m_main_ram_seg + m_dsp_addr_w));
	return input_data;
}

WRITE16_MEMBER(twincobr_state::wardner_dsp_w)
{
	/* Data written to main CPU RAM via DSP IO port 1 */
	m_dsp_execute = 0;
	switch (m_main_ram_seg) {
		case 0x7000:    if ((m_dsp_addr_w < 3) && (data == 0)) m_dsp_execute = 1;
		case 0x8000:
		case 0xa000:   {address_space &mainspace = m_maincpu->space(AS_PROGRAM);
						mainspace.write_byte(m_main_ram_seg + (m_dsp_addr_w + 0), (data & 0xff));
						mainspace.write_byte(m_main_ram_seg + (m_dsp_addr_w + 1), ((data >> 8) & 0xff));
						break;}
		default:        logerror("DSP PC:%04x Warning !!! IO writing to %08x (port 1)\n",space.device().safe_pcbase(),m_main_ram_seg + m_dsp_addr_w); break;
	}
	LOG(("DSP PC:%04x IO write %04x at %08x (port 1)\n",space.device().safe_pcbase(),data,m_main_ram_seg + m_dsp_addr_w));
}

WRITE16_MEMBER(twincobr_state::twincobr_dsp_bio_w)
{
	/* data 0xffff  means inhibit BIO line to DSP and enable */
	/*              communication to main processor */
	/*              Actually only DSP data bit 15 controls this */
	/* data 0x0000  means set DSP BIO line active and disable */
	/*              communication to main processor*/
	LOG(("DSP PC:%04x IO write %04x at port 3\n",space.device().safe_pcbase(),data));
	if (data & 0x8000) {
		m_dsp_BIO = CLEAR_LINE;
	}
	if (data == 0) {
		if (m_dsp_execute) {
			LOG(("Turning the main CPU on\n"));
			m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_dsp_execute = 0;
		}
		m_dsp_BIO = ASSERT_LINE;
	}
}

READ16_MEMBER(twincobr_state::fsharkbt_dsp_r)
{
	/* IO Port 2 used by Flying Shark bootleg */
	/* DSP reads data from an extra MCU (8741) at IO port 2 */
	/* Port is read three times during startup. First and last data */
	/*   read must equal, but second data read must be different */
	m_fsharkbt_8741 += 1;
	LOG(("DSP PC:%04x IO read %04x from 8741 MCU (port 2)\n",space.device().safe_pcbase(),(m_fsharkbt_8741 & 0x08)));
	return (m_fsharkbt_8741 & 1);
}

WRITE16_MEMBER(twincobr_state::fsharkbt_dsp_w)
{
	/* Flying Shark bootleg DSP writes data to an extra MCU (8741) at IO port 2 */
#if 0
	logerror("DSP PC:%04x IO write from DSP RAM:%04x to 8741 MCU (port 2)\n",space.device().safe_pcbase(),m_fsharkbt_8741);
#endif
}

READ_LINE_MEMBER(twincobr_state::twincobr_BIO_r)
{
	return m_dsp_BIO;
}


WRITE_LINE_MEMBER(twincobr_state::int_enable_w)
{
	m_intenable = state;
}

WRITE_LINE_MEMBER(twincobr_state::dsp_int_w)
{
	m_dsp_on = state;
	if (state)
	{
		// assert the INT line to the DSP
		LOG(("Turning DSP on and main CPU off\n"));
		m_dsp->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
		m_dsp->set_input_line(0, ASSERT_LINE); // TMS32010 INT
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
	else
	{
		// inhibit the INT line to the DSP
		LOG(("Turning DSP off\n"));
		m_dsp->set_input_line(0, CLEAR_LINE); // TMS32010 INT
		m_dsp->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

void twincobr_state::twincobr_restore_dsp()
{
	dsp_int_w(m_dsp_on);
}


WRITE_LINE_MEMBER(twincobr_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(twincobr_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE_LINE_MEMBER(twincobr_state::coin_lockout_1_w)
{
	machine().bookkeeping().coin_counter_w(0, !state);
}

WRITE_LINE_MEMBER(twincobr_state::coin_lockout_2_w)
{
	machine().bookkeeping().coin_counter_w(1, !state);
}


READ16_MEMBER(twincobr_state::twincobr_sharedram_r)
{
	return m_sharedram[offset];
}

WRITE16_MEMBER(twincobr_state::twincobr_sharedram_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_sharedram[offset] = data & 0xff;
	}
}


MACHINE_RESET_MEMBER(twincobr_state,twincobr)
{
	m_dsp_addr_w = 0;
	m_main_ram_seg = 0;
	m_dsp_execute = 0;
	m_dsp_BIO = CLEAR_LINE;
	m_fsharkbt_8741 = -1;
}

void twincobr_state::twincobr_driver_savestate()
{
	save_item(NAME(m_intenable));
	save_item(NAME(m_dsp_on));
	save_item(NAME(m_dsp_addr_w));
	save_item(NAME(m_main_ram_seg));
	save_item(NAME(m_dsp_BIO));
	save_item(NAME(m_dsp_execute));
	save_item(NAME(m_fsharkbt_8741));

	machine().save().register_postload(save_prepost_delegate(FUNC(twincobr_state::twincobr_restore_dsp), this));
}
