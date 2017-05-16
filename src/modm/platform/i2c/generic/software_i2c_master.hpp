/*
 * Copyright (c) 2009-2012, Fabian Greif
 * Copyright (c) 2010, Thorsten Lajewski
 * Copyright (c) 2010-2011, Georgi Grinshpun
 * Copyright (c) 2012, Sascha Schade
 * Copyright (c) 2012-2016, Niklas Hauser
 *
 * This file is part of the modm project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
// ----------------------------------------------------------------------------

#ifndef MODM_SOFTWARE_I2C_HPP
#define MODM_SOFTWARE_I2C_HPP

#include <modm/architecture/driver/delay.hpp>
#include <modm/architecture/interface/gpio.hpp>
#include <modm/architecture/interface/i2c_master.hpp>
#include "type_ids.hpp"

namespace modm
{

/**
 * Software emulation of a I2C master implementation
 *
 * @tparam	SCL			an Open-Drain pin
 * @tparam	SDA			an Open-Drain pin
 *
 * @ingroup	i2c
 * @author	Niklas Hauser
 * @see		gpio
 */
template< class SCL,
		  class SDA >
class SoftwareI2cMaster : public modm::I2cMaster
{
public:
	static const TypeId::SoftwareI2cMasterSda Sda;
	static const TypeId::SoftwareI2cMasterScl Scl;

public:
	/// Initializes the hardware, with the baudrate limited to about 250kbps.
	template< class SystemClock, uint32_t baudrate=Baudrate::Standard,
			uint16_t tolerance = modm::Tolerance::FivePercent >
	static void
	initialize()
	{
		delayTime = uint32_t(250000000) / baudrate;
		if (delayTime == 0) delayTime = 1;

		SCL::set();
		SDA::set();
	}

public:
	// start documentation inherited
	static bool
	start(I2cTransaction *transaction, ConfigurationHandler handler = nullptr);

	static Error modm_always_inline
	getErrorState()
	{ return errorState; }

	static inline void
	reset();
	// end documentation inherited

private:
	// error handling
	/// releases bus lines, sets error state and detaches the TO
	static inline void
	error(Error error);

	// bus condition operations
	/// generate a start or restart condition
	/// @return	`true` if success, `false` if bus in unknown condition
	static inline bool
	startCondition();

	/// generate a stop condition
	/// @return	`true` if success, `false` if bus in unknown condition
	static inline bool
	stopCondition();

	/// release the clock and wait for any slaves to release it too
	/// @return	`true` if success, `false` if slave stretched the clock for too long
	static inline bool
	sclSetAndWait();

	// byte operations
	/// write one byte to the bus
	/// @return	`true` if success, `false` if arbitation, too much clock stretching or NACK received
	static inline bool
	write(uint8_t data);

	/// read one byte from the bus
	/// @param	ack	acknowledge bit of read operation, `ACK` or `NACK`
	/// @return	`true` if success, `false` if arbitation occured
	static inline bool
	read(uint8_t &data, bool ack);

	/// write one bit to the bus
	/// @return	`true` if success, `false` if slave stretched the clock for too long
	static inline bool
	writeBit(bool bit);

	// bit operations
	/// read one bit from the bus
	/// @return	`true` if success, `false` if slave stretched the clock for too long
	static inline bool
	readBit(uint8_t &data);

	// timings
	/// busy waits a **half** clock cycle
	static modm_always_inline void
	delay2()
	{ modm::delayNanoseconds(delayTime*2); }

	// timings
	/// busy waits **quarter** clock cycle
	static modm_always_inline void
	delay4()
	{ modm::delayNanoseconds(delayTime); }

	enum
	{
		ACK = true,
		NACK = false,
	};

	// calculate the delay in microseconds needed to achieve the
	// requested I2C frequency
	static uint16_t delayTime;

	static modm::I2c::Operation nextOperation;
	static modm::I2cTransaction *transactionObject;
	static Error errorState;
	static modm::I2c::ConfigurationHandler configuration;

	static modm::I2cTransaction::Starting starting;
	static modm::I2cTransaction::Writing writing;
	static modm::I2cTransaction::Reading reading;
};

} // namespace modm

#include "i2c_master_impl.hpp"

#endif // MODM_SOFTWARE_I2C_HPP