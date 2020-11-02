section .data

global _house1
_house1 incbin 'assets/house.dds'
_house1_end:
global _house1_len
_house1_len dd _house1_end - _house1

global _house2
_house2 incbin 'assets/house2.dds'
_house2_end:
global _house2_len
_house2_len dd _house2_end - _house2

global _house3
_house3 incbin 'assets/house3.dds'
_house3_end:
global _house3_len
_house3_len dd _house3_end - _house3


global _car1
_car1 incbin 'assets/car1.dds'
_car1_end:
global _car1_len
_car1_len dd _car1_end - _car1

global _car2
_car2 incbin 'assets/car2.dds'
_car2_end:
global _car2_len
_car2_len dd _car2_end - _car2

global _car3
_car3 incbin 'assets/car3.dds'
_car3_end:
global _car3_len
_car3_len dd _car3_end - _car3

global _car4
_car4 incbin 'assets/car4.dds'
_car4_end:
global _car4_len
_car4_len dd _car4_end - _car4


global _cloud_dat
_cloud_dat incbin 'assets/cloud.dds'
_cloud_end:
global _cloud_len
_cloud_len dd _cloud_end - _cloud_dat

global _sun_dat
_sun_dat incbin 'assets/sun.dds'
_sun_end:
global _sun_len
_sun_len dd _sun_end - _sun_dat

global _mountain_dat
_mountain_dat incbin 'assets/mountains.dds'
_mountain_end:
global _mountain_len
_mountain_len dd _mountain_end - _mountain_dat

global _road_dat
_road_dat incbin 'assets/road.dds'
_road_end:
global _road_len
_road_len dd _road_end - _road_dat


global _play_dat
_play_dat incbin 'assets/play.dds'
_play_end:
global _play_len
_play_len dd _play_end - _play_dat

global _stop_dat
_stop_dat incbin 'assets/stop.dds'
_stop_end:
global _stop_len
_stop_len dd _stop_end - _stop_dat

global _prev_dat
_prev_dat incbin 'assets/prev.dds'
_prev_end:
global _prev_len
_prev_len dd _prev_end - _prev_dat

global _next_dat
_next_dat incbin 'assets/next.dds'
_next_end:
global _next_len
_next_len dd _next_end - _next_dat

global _playover_dat
_playover_dat incbin 'assets/play_over.dds'
_playover_end:
global _playover_len
_playover_len dd _playover_end - _playover_dat

global _stopover_dat
_stopover_dat incbin 'assets/stop_over.dds'
_stopover_end:
global _stopover_len
_stopover_len dd _stopover_end - _stopover_dat

global _prevover_dat
_prevover_dat incbin 'assets/prev_over.dds'
_prevover_end:
global _prevover_len
_prevover_len dd _prevover_end - _prevover_dat

global _nextover_dat
_nextover_dat incbin 'assets/next_over.dds'
_nextover_end:
global _nextover_len
_nextover_len dd _nextover_end - _nextover_dat
