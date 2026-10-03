/*
 * time_backup.h
 *
 * HW RTC의 backup 유지시간이 짧은(예: 약 5분) 보드에서,
 * 1) 주기적으로/필요시 현재 시각(time_t)을 파일에 저장하고
 * 2) 부팅 시 hwclock(RTC) 값이 비정상적으로 오래된 값(=1970-01-01 근처)이면
 *    저장해둔 시각을 system 시각 / hwclock 에 다시 설정해주기 위한 유틸리티.
 *
 * 대상 환경: Linux (glibc), RTC 캐릭터 디바이스(/dev/rtc0 등) 사용 가능한 보드
 *
 * 시간대 처리: RTC가 "로컬타임" 기준으로 운용된다는 전제로,
 * mktime()/localtime_r() 을 사용한다 (mktime/localtime_r 은 시스템에
 * 설정된 TZ 를 참고한다). RTC가 UTC 기준이면 이 두 함수를 각각
 * timegm()/gmtime_r() 로 바꿔야 한다.
 */

#ifndef TIME_BACKUP_H
#define TIME_BACKUP_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 시각을 저장할 기본 경로 (전원 차단에도 남아야 하므로 eMMC/NAND 등
 * 비휘발성 파일시스템 위의 경로를 지정할 것. tmpfs(/tmp, /run 등)는 안됨) */
#define TIME_BACKUP_DEFAULT_PATH   "/mnt/bin/time_backup.dat"

/* 사용할 RTC 디바이스 노드. 보드에 따라 /dev/rtc, /dev/rtc0 등으로 다를 수 있음 */
#define TIME_BACKUP_RTC_DEVICE     "/dev/rtc0"

/* 이 값보다 이전 시각이면 "RTC가 리셋되어 신뢰할 수 없는 시각(1970-01-01 부근)"
 * 으로 간주한다. 빌드 시점 등을 기준으로 배포 때마다 최신값으로 올려줄 것.
 * (예 값: 2023-11-15 00:00:00 UTC) */
#define TIME_BACKUP_MIN_VALID_TIME ((time_t)1700000000)

/*
 * 현재 system time(time_t)을 path 파일에 원자적으로 저장한다.
 * (임시파일에 쓰고 fsync 후 rename 하여, 쓰는 도중 전원이 나가도
 *  기존 파일이 깨지지 않도록 한다.)
 *
 * 반환값: 성공 0, 실패 -1 (errno 참고)
 */
int time_backup_save(const char *path);

/*
 * path 파일에 저장되어 있던 time_t 값을 읽어 out_time 에 채운다.
 * 반환값: 성공 0, 실패 -1 (errno 참고)
 */
int time_backup_load(const char *path, time_t *out_time);

/*
 * RTC(hwclock)에서 현재 시각을 읽어 out_time(time_t)에 채운다.
 * RTC 값은 "로컬타임"으로 해석한다 (mktime 사용, 시스템 TZ 설정을 따름).
 * 반환값: 성공 0, 실패 -1 (errno 참고)
 */
int time_backup_get_hwclock(time_t *out_time);

/*
 * RTC(hwclock)에 t(time_t) 시각을 기록한다.
 * t는 "로컬타임"으로 변환되어 RTC에 저장된다 (localtime_r 사용).
 * 반환값: 성공 0, 실패 -1 (errno 참고)
 */
int time_backup_set_hwclock(time_t t);

/*
 * system time(커널 시각)을 t로 설정한다. (settimeofday)
 * 보통 root 권한 필요.
 * 반환값: 성공 0, 실패 -1 (errno 참고)
 */
int time_backup_set_systime(time_t t);

/*
 * 부팅 시 1회 호출하는 함수.
 *  1) RTC(hwclock) 시각을 읽는다 (실패하면 현재 system time으로 대체 판단)
 *  2) 그 값이 min_valid_time 보다 오래된 값이면 "리셋된 것"으로 보고,
 *     path 에 저장된 백업 시각을 읽어와 system time과 hwclock을 모두
 *     그 값으로 갱신한다.
 *  3) RTC가 정상 범위이면 아무 것도 하지 않는다 (커널이 부팅시 이미
 *     RTC 기준으로 system time을 세팅했다고 가정).
 *
 * 반환값:
 *   0  : 정상 (RTC가 유효했거나, 혹은 백업값으로 복구 성공)
 *  -1  : RTC도 무효하고 백업 파일도 없거나/무효하여 복구 실패 (errno 참고)
 */
int time_backup_sync_on_boot(const char *path, time_t min_valid_time);

#ifdef __cplusplus
}
#endif

#endif /* TIME_BACKUP_H */
