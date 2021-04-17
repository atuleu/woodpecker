#include "display.h"

#include "pico/stdlib.h"

#include "tlc5973.pio.h"

#include <tuple>


typedef std::tuple<uint8_t,uint8_t,uint8_t> RGBColor;
typedef std::tuple<uint8_t,uint8_t>         Position;


class LEDAdressable {
public:
	virtual void SetPixel(size_t index, const RGBColor & color) = 0;
};

template <size_t Size>
class LEDStripQueue : public LEDAdressable {
public:
	LEDStripQueue(PIO pio, uint sm, uint pin)
		: d_index(Size*2)
		, d_pio(pio)
		, d_sm(sm){
		for ( size_t i = 0; i < Size; ++i ) {
			SetPixel(i,std::make_tuple(0,0,0));
		}
		uint offset = pio_add_program(pio, &tlc5973_program);

		tlc5973_program_init(pio, sm, offset, pin, 2e6);
	}

	size_t SendAvailable() {
		//		printf("Sending...");
		while( d_index < 2*Size  ) {
			//	printf(" %d",d_index);
			pio_sm_put_blocking(d_pio,d_sm,d_data[d_index]);
			++d_index;
		}
		//		printf("DONE\n");
		return 2*Size - d_index;
	}

	void ReArm() {
		d_index = 0;
	}

	void SetPixel(size_t index, const RGBColor & color) override {
		d_data[2*index + 0] = (47 << 16) | ( 0x3aa << 4 ) | (std::get<0>(color) >> 4);
		d_data[2*index + 1] = ( std::get<0>(color) << 28 ) | ( 0xf << 24 )
			| ( std::get<1>(color) << 16 ) | ( 0xf << 12 )
			| ( std::get<2>(color) << 4 ) | 0xf;
	}

private:

	PIO        d_pio;
	uint       d_sm;
	size_t     d_index;
	uint32_t   d_data[Size*2];
};


class DisplayImpl {
public:
	DisplayImpl()
		: d_topQueue(LEDStripQueue<20>(pio0,0,10))
		, d_bottomQueue(LEDStripQueue<10>(pio0,1,1)) {
		d_last = time_us_64() - PERIOD_US;
		for ( size_t y = 0; y < 4; ++y ) {
			for (size_t x = 0; x < 5; ++x) {
				size_t index = y * 5 + x;
				d_queueMap[index] = &d_topQueue;
				d_indexMap[index] = x * 4;
				if ( (x % 2) == 0 ) {
					d_indexMap[index] += 3 - y;
				} else {
					d_indexMap[index] += y;
				}
				d_indexMap[index] = 19 - d_indexMap[index];
			}
		}


		for ( size_t y = 4; y < 6; ++y ) {
			for (size_t x = 0; x < 5; ++x) {
				size_t index = y * 5 + x;
				d_queueMap[index] = &d_bottomQueue;
				d_indexMap[index] = x * 2;
				if ( x % 2 == 1 ) {
					d_indexMap[index] += 5 - y;
				} else {
					d_indexMap[index] += y - 4;
				}
			}
		}

	}

	void PrintIndex() {
		printf("-------------\n");
		for ( size_t y = 0 ; y < 6; ++y) {
			for ( size_t x = 0; x < 5; ++ x) {
				printf("%d ",d_indexMap[y*5+x]);
			}
			printf("\n");
		}
		printf("-------------\n");
	}

	void Process() {
		uint64_t now = time_us_64();
		if ( ( now - d_last) >= PERIOD_US ) {
			d_last += PERIOD_US;
			d_topQueue.ReArm();
			d_bottomQueue.ReArm();
		}
		d_topQueue.SendAvailable();
		d_bottomQueue.SendAvailable();
	}

	void SetPixel(const Position & position,
				  const RGBColor & color) {
		if ( std::get<0>(position) >= 5 || std::get<1>(position) >=6 ) {
			return;
		}
		size_t pixelIndex = std::get<1>(position) * 5 + std::get<0>(position);
		d_queueMap[pixelIndex]->SetPixel(d_indexMap[pixelIndex],color);
	}

private:
	uint64_t              d_last;
	const static uint64_t PERIOD_US=25e3;

	LEDStripQueue<20> d_topQueue;
	LEDStripQueue<10> d_bottomQueue;

	LEDAdressable * d_queueMap[30];
	size_t          d_indexMap[30];
};

static DisplayImpl * s_impl = nullptr;

extern "C" {

	void display_init() {
		s_impl = new DisplayImpl();
	}
	void display_process() {
		s_impl->Process();
	}

	void display_set_pixel(uint8_t x, uint8_t y,
						   uint8_t r, uint8_t g, uint8_t b) {
		s_impl->SetPixel({x,y},{r,g,b});
	}
}
