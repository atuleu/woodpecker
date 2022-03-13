#include "key.h"

#include <list>
#include <functional>

#include <pico/stdlib.h>
#include "hardware/spi.h"
class KeyModule {
public:
#define spiBot spi0
#define spiTop spi1
#define CS_BOT 4
#define CS_TOP_COL 8
#define CS_TOP_ROW 9

	static inline void SelectSpi(uint pin) {
		asm volatile("nop \n nop \n nop");
		gpio_put(pin, 0);
		asm volatile("nop \n nop \n nop");
	}

	static inline void UnselectSpi(uint pin) {
		asm volatile("nop \n nop \n nop");
		gpio_put(pin, 1);
		asm volatile("nop \n nop \n nop");
	}

	static inline void WriteRegister(spi_inst_t * spi, uint pin, uint8_t address, uint8_t value) {
		uint8_t buffer[3] = { 0x40, address, value };

		SelectSpi(pin);
		spi_write_blocking(spi,buffer,3);
		UnselectSpi(pin);
	}

	static inline uint8_t ReadRegister(spi_inst_t * spi, uint pin, uint8_t address ) {
		uint8_t out[3] = { 0x41, address, 0x00 };
		uint8_t in[3];
		SelectSpi(pin);
		spi_write_read_blocking(spi,in,out,3);
		UnselectSpi(pin);
		return in[2];
	}


	KeyModule() {
		spi_init(spiBot,1e6);
		spi_init(spiTop,1e6);
		gpio_set_function(0,GPIO_FUNC_SPI);
		gpio_set_function(2,GPIO_FUNC_SPI);
		gpio_set_function(3,GPIO_FUNC_SPI);
		gpio_set_function(12,GPIO_FUNC_SPI);
		gpio_set_function(14,GPIO_FUNC_SPI);
		gpio_set_function(15,GPIO_FUNC_SPI);

		for ( const auto & pin : { CS_BOT,CS_TOP_COL,CS_TOP_ROW } ) {
			gpio_init(pin);
			gpio_set_dir(pin,GPIO_OUT);
			gpio_put(pin,1);
		}

		uint16_t IDs[50] = {
							301, 302, 304, 304, 305, // R4
							0,   0,   0,   0,   0, // QA1
							0,   0,   0,   0,   0,  // QB1
							601, 602, 603, 604, 605, //R1
							401, 402, 403, 404, 405, // R2
							0,   0,   0,   0,   0, // QA2
							0,   0,   0,   0,   0,  // QB2
							501, 502, 503, 504, 505, // R3
							201, 202, 204, 204, 205, // R5
							101, 102, 104, 104, 105, // R6
		};

		for ( uint i = 0; i < 50; ++i ) {
			keys[i].ID = 0;
			keys[i].Current = false;
			keys[i].Last = false;
		}

		WriteRegister(spiTop,CS_TOP_COL,0x06,0xff);
		WriteRegister(spiTop,CS_TOP_ROW,0x00,0x00);

		WriteRegister(spiBot,CS_BOT,0x00,0xfc);
		WriteRegister(spiBot,CS_BOT,0x06,0xfc);

		uint64_t now = time_us_64();
		d_nextCycle = now + 1000e3;
		d_nextOperationTime = d_nextCycle;
		d_currentRow = 0;
		d_nextOperation = [this]( uint64_t now) { ActivateScan(now); };
	}



	void Process() {
		uint64_t now = time_us_64();
		if ( now >= d_nextOperationTime ) {
			d_nextOperation(now);
		}
	}

	void SelectRow(uint8_t row) {
		if ( row < 8 ) {
			WriteRegister(spiTop,CS_TOP_ROW,0x9,~(1 << row));
		} else {
			WriteRegister(spiBot,CS_BOT,0x9,~(1 << (row-8)));
		}
	}

	void UnselectRow(uint8_t row) {
		if ( row < 8 ) {
			WriteRegister(spiTop,CS_TOP_ROW,0x9,0xff);
		} else {
			WriteRegister(spiBot,CS_BOT,0x9,0x03);
		}
	}

	uint8_t ReadCol() {
		if ( d_currentRow < 8) {
			return ReadRegister(spiTop,CS_TOP_COL,0x9);
		} else {
			return ReadRegister(spiBot,CS_BOT,0x9) >> 2;
		}
	}

	void ActivateScan(uint64_t now) {
		SelectRow(d_currentRow);

		d_nextOperation = [this](uint64_t now) { ReadKey(now); };
		d_nextOperationTime = now + 50;
	}

	void ReadKey(uint64_t now) {
		uint8_t col = ReadCol();
		UnselectRow(d_currentRow);
		//		printf("%llu: row: %d col is %x\n",now, d_currentRow, col);

		for ( uint x = 0; x < 5; ++x) {
			auto & k = keys[d_currentRow * 5 + x];
			if (k.ID == 0 ) {
				continue;
			}
			bool currentRead = (col & ( 1 << x) ) == 0;
			if ( currentRead != k.Last ) {
				k.Since = now;
			} else if ( currentRead != k.Current
						&& (now - k.Since) > 5000 ) {
				k.Current = currentRead;
				d_events.push_back(ke_create(k.ID,currentRead));
			}
			k.Last = currentRead;
		}


		++d_currentRow;
		d_nextOperation = [this] ( uint64_t now) { ActivateScan(now); };
		if ( d_currentRow >= 10 /* end of cycle */ ) {
			d_currentRow = 0;
			d_nextOperationTime = d_nextCycle;
			d_nextCycle += 500e3;
		} {
			d_nextOperationTime = now;
		}
	}


	KeyEvent NextEvent() {
		if ( d_events.empty() == true ) {
			return ke_noevent;
		}
		KeyEvent res = d_events.front();
		d_events.pop_front();
		return res;
	}

private:
	typedef std::function<void (uint64_t)> Operation;

	std::list<KeyEvent> d_events;

	uint64_t  d_nextOperationTime;
	uint64_t  d_nextCycle;
	Operation d_nextOperation;
	uint8_t   d_currentRow;

	struct KeyData {
		uint16_t ID;
		bool     Current,Last;
		uint64_t Since;
	};

	struct EncoderData {
		uint16_t ID;
		uint8_t state;
		uint64_t Since;
	};

	KeyData keys[50];

};
static KeyModule * keyModule = nullptr;


extern "C" {

	void key_init() {
		keyModule = new KeyModule();
	}

	void key_process() {
		keyModule->Process();
	}

	KeyEvent key_next_event() {
		return keyModule->NextEvent();
	}
}
