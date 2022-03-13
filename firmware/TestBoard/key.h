#pragma once

#include <stdint.h>

#define ke_key_id(ke) ( (ke) & 0x7fff )
#define ke_key_is_pressed(ke) ( ((ke) & 0x8000 ) != 0 )
#define ke_create(kID,pressed) ( (kID & 0x7fff) | ((pressed != 0) ? 0x8000 : 0x0000) )
#define ke_noevent 0x0000

#ifdef __cplusplus
extern "C" {
#endif

	typedef uint16_t KeyEvent;

	void key_init();
	void key_process();
    KeyEvent key_next_event();

#ifdef __cplusplus
}
#endif
