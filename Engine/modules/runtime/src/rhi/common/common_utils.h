#pragma once

#define cyber_round_up(value, multiple) (((value + multiple - 1) / (multiple)) * multiple)
#define cyber_round_down(value, multiple) (value - value % multiple)