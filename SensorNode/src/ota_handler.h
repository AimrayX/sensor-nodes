#pragma once

void ota_init(const char* hostname);
void ota_loop();
bool ota_in_progress();