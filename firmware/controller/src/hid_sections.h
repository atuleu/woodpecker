#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int  hid_sections_init();
void hid_sections_deinit();

void hid_section_lock();
void hid_section_unlock();

#ifdef __cplusplus
}
#endif
