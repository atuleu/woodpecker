#include "display.h"

#include "pico/stdlib.h"

#include "tlc5973.pio.h"

#include <tuple>
#include <functional>

typedef std::tuple<uint8_t,uint8_t,uint8_t>    RGB8;
typedef std::tuple<uint16_t,uint16_t,uint16_t> RGB16;
typedef std::tuple<uint8_t,uint8_t>            Position;


class LEDAddressable {
public:
	virtual void SetPixel(size_t index, const RGB16 & color) = 0;
};

template <size_t Size>
class LEDStripQueue : public LEDAddressable {
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
		while( d_index < 2*Size  ) {
			pio_sm_put_blocking(d_pio,d_sm,d_data[d_index]);
			++d_index;
		}
		return 2*Size - d_index;
	}

	void ReArm() {
		d_index = 0;
	}

	void SetPixel(size_t index, const RGB16 & color) override {
		d_data[2*index + 0] = (47 << 16) | ( 0x3aa << 4 ) | ( (std::get<0>(color) & 0xfff) >> 8);
		d_data[2*index + 1] = ( std::get<0>(color) << 24 )
			| ( ( std::get<1>(color) & 0xfff ) << 12 )
			| ( std::get<2>(color) & 0xfff );
	}

private:

	PIO        d_pio;
	uint       d_sm;
	size_t     d_index;
	uint32_t   d_data[Size*2];
};

typedef std::function<RGB16 (const RGB8 & )> GammaConverter;

struct PixelData {
	LEDAddressable * Queue;
	size_t           Index;
	GammaConverter   Gamma;
};

RGB16 convertKey(const RGB8 & c) {
	return { uint16_t(std::get<0>(c)) << 2,
			 uint16_t(std::get<1>(c)) << 1,
			 uint16_t(std::get<2>(c)) << 2 };
}

RGB16 convertEncoder(const RGB8 & c ) {
	return { uint16_t(std::get<0>(c)),
			 uint16_t(std::get<1>(c))>> 1,
			 uint16_t(std::get<2>(c))};
}

class DisplayImpl {
public:
	DisplayImpl()
		: d_topQueue(LEDStripQueue<20>(pio0,0,10))
		, d_bottomQueue(LEDStripQueue<10>(pio0,1,1)) {
		d_last = time_us_64() - PERIOD_US;
		for ( size_t y = 0; y < 4; ++y ) {
			for (size_t x = 0; x < 5; ++x) {
				size_t index = y * 5 + x;
				d_pixelData[index].Queue = &d_topQueue;
				d_pixelData[index].Index = x * 4;
				if ( (x % 2) == 0 ) {
					d_pixelData[index].Index += 3 - y;
				} else {
					d_pixelData[index].Index += y;
				}
				if ( (y % 2) == 1 ) {
					d_pixelData[index].Gamma = &convertKey;
				} else {
					d_pixelData[index].Gamma = &convertEncoder;
				}
				//				d_pixelData[index].Index = 19 - d_pixelData[index].Index;
			}
		}


		for ( size_t y = 4; y < 6; ++y ) {
			for (size_t x = 0; x < 5; ++x) {
				size_t index = y * 5 + x;
				d_pixelData[index].Queue = &d_bottomQueue;
				d_pixelData[index].Index = x * 2;
				if ( x % 2 == 1 ) {
					d_pixelData[index].Index += 5 - y;
				} else {
					d_pixelData[index].Index += y - 4;
				}
				//				d_pixelData[index].Index = 9 - d_pixelData[index].Index;
				d_pixelData[index].Gamma = &convertKey;
			}
		}

	}

	void PrintIndex() {
		printf("-------------\n");
		for ( size_t y = 0 ; y < 6; ++y) {
			for ( size_t x = 0; x < 5; ++ x) {
				printf("%d ",d_pixelData[y*5+x].Index);
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
			PrintIndex();
		}
		d_topQueue.SendAvailable();
		d_bottomQueue.SendAvailable();
	}



	void SetPixel(const Position & position,
				  const RGB8 & color) {
		if ( std::get<0>(position) >= 5 || std::get<1>(position) >=6 ) {
			return;
		}
		const auto & p = d_pixelData[std::get<1>(position) * 5 + std::get<0>(position)];
		p.Queue->SetPixel(p.Index,p.Gamma(color));
	}

private:
	uint64_t              d_last;
	const static uint64_t PERIOD_US=25e3;

	LEDStripQueue<20> d_topQueue;
	LEDStripQueue<10> d_bottomQueue;

	PixelData d_pixelData[30];
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
