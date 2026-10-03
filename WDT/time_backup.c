/*
 * time_backup.c
 *
 * time_backup.h 구현부.
 *
 * 빌드 예:
 *   gcc -Wall -Wextra -O2 -c time_backup.c
 *
 * 시간대 처리: RTC가 "로컬타임" 기준으로 운용된다는 전제로
 * mktime()/localtime_r()을 사용한다. 이 두 함수는 시스템에 설정된
 * TZ(시간대)를 참고하므로, 보드의 TZ 설정이 실제 RTC와 일치해야 한다.
 */

#define _DEFAULT_SOURCE   /* O_DIRECTORY 등 사용 */

#include "time_backup.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>

/* ------------------------------------------------------------------ */
/* 내부 유틸: path의 부모 디렉터리를 fsync 해서, rename 자체가 디스크에
 * 반영되도록 강제한다 (전원 순간 차단 대비). 실패해도 치명적이지 않으므로
 * 에러는 무시한다. */
static void sync_parent_dir(const char *path)
{
    char dir_path[PATH_MAX];
    char *slash;
    int dfd;

    if (strlen(path) >= sizeof(dir_path))
        return;

    strncpy(dir_path, path, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';

    slash = strrchr(dir_path, '/');
    if (slash) {
        if (slash == dir_path)
            slash[1] = '\0';   /* 루트 디렉터리("/xxx") 케이스 */
        else
            *slash = '\0';
    } else {
        /* 디렉터리 구분자가 없으면 현재 디렉터리 */
        strncpy(dir_path, ".", sizeof(dir_path));
    }

    dfd = open(dir_path, O_RDONLY | O_DIRECTORY);
    if (dfd < 0)
        return;

    fsync(dfd);
    close(dfd);
}

/* ------------------------------------------------------------------ */
int time_backup_save(const char *path)
{
    char tmp_path[PATH_MAX];
    int fd;
    time_t now;
    ssize_t written;

    if (!path) {
        errno = EINVAL;
        return -1;
    }

    now = time(NULL);
    if (now == (time_t)-1)
        return -1;

    if (snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", path)
        >= (int)sizeof(tmp_path)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    fd = open(tmp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0)
        return -1;

    written = write(fd, &now, sizeof(now));
    if (written != (ssize_t)sizeof(now)) {
        int saved_errno = errno ? errno : EIO;
        close(fd);
        unlink(tmp_path);
        errno = saved_errno;
        return -1;
    }

    if (fsync(fd) != 0) {
        int saved_errno = errno;
        close(fd);
        unlink(tmp_path);
        errno = saved_errno;
        return -1;
    }

    close(fd);

    if (rename(tmp_path, path) != 0) {
        int saved_errno = errno;
        unlink(tmp_path);
        errno = saved_errno;
        return -1;
    }

    sync_parent_dir(path);

    return 0;
}

/* ------------------------------------------------------------------ */
int time_backup_load(const char *path, time_t *out_time)
{
    int fd;
    time_t val;
    ssize_t r;

    if (!path || !out_time) {
        errno = EINVAL;
        return -1;
    }

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    r = read(fd, &val, sizeof(val));
    close(fd);

    if (r != (ssize_t)sizeof(val)) {
        errno = EIO;
        return -1;
    }

    *out_time = val;
    return 0;
}

/* ------------------------------------------------------------------ */
int time_backup_get_hwclock(time_t *out_time)
{
    int fd;
    struct rtc_time rtc;
    struct tm tm_val;
    time_t t;

    if (!out_time) {
        errno = EINVAL;
        return -1;
    }

    fd = open(TIME_BACKUP_RTC_DEVICE, O_RDONLY);
    if (fd < 0)
        return -1;

    if (ioctl(fd, RTC_RD_TIME, &rtc) < 0) {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }
    close(fd);

    memset(&tm_val, 0, sizeof(tm_val));
    tm_val.tm_sec  = rtc.tm_sec;
    tm_val.tm_min  = rtc.tm_min;
    tm_val.tm_hour = rtc.tm_hour;
    tm_val.tm_mday = rtc.tm_mday;
    tm_val.tm_mon  = rtc.tm_mon;
    tm_val.tm_year = rtc.tm_year;
    tm_val.tm_isdst = -1;  /* DST 적용 여부는 mktime이 TZ 규칙으로 판단하게 함 */

    /* RTC는 "로컬타임" 기준으로 가정 -> mktime으로 time_t 변환 */
    t = mktime(&tm_val);
    if (t == (time_t)-1)
        return -1;

    *out_time = t;
    return 0;
}

/* ------------------------------------------------------------------ */
int time_backup_set_hwclock(time_t t)
{
    int fd;
    struct tm tm_val;
    struct rtc_time rtc;

    /* t를 "로컬타임" 기준 struct tm으로 변환 (시스템 TZ 설정을 따름) */
    if (localtime_r(&t, &tm_val) == NULL)
        return -1;

    fd = open(TIME_BACKUP_RTC_DEVICE, O_RDWR);
    if (fd < 0)
        return -1;

    memset(&rtc, 0, sizeof(rtc));
    rtc.tm_sec  = tm_val.tm_sec;
    rtc.tm_min  = tm_val.tm_min;
    rtc.tm_hour = tm_val.tm_hour;
    rtc.tm_mday = tm_val.tm_mday;
    rtc.tm_mon  = tm_val.tm_mon;
    rtc.tm_year = tm_val.tm_year;

    if (ioctl(fd, RTC_SET_TIME, &rtc) < 0) {
        int saved_errno = errno;
        close(fd);
        errno = saved_errno;
        return -1;
    }

    close(fd);
    return 0;
}

/* ------------------------------------------------------------------ */
int time_backup_set_systime(time_t t)
{
    struct timeval tv;

    tv.tv_sec = t;
    tv.tv_usec = 0;

    return settimeofday(&tv, NULL);
}

/* ------------------------------------------------------------------ */
int time_backup_sync_on_boot(const char *path, time_t min_valid_time)
{
    time_t hw_time;
    time_t saved_time;
    int hw_valid = 0;

    if (time_backup_get_hwclock(&hw_time) == 0) {
        hw_valid = (hw_time >= min_valid_time);
    } else {
        /* RTC 접근 자체가 실패하면, 커널이 부팅 시 이미 RTC로부터
         * system time을 세팅했다고 가정하고 그 값으로 대신 판단한다. */
        hw_time = time(NULL);
        hw_valid = (hw_time >= min_valid_time);
    }

    if (hw_valid) {
        /* RTC가 정상 범위 => 손대지 않음 (커널이 이미 system time을
         * RTC 기준으로 세팅한 상태로 간주) */
        return 0;
    }

    /* RTC가 리셋된 것으로 판단 => 백업 파일에서 복구 시도 */
    if (time_backup_load(path, &saved_time) != 0) {
        /* 백업 파일이 없거나 읽기 실패: 복구할 수단이 없음 */
        return -1;
    }

    if (saved_time < min_valid_time) {
        /* 백업된 값 자체도 신뢰할 수 없음 */
        errno = EINVAL;
        return -1;
    }

    if (time_backup_set_systime(saved_time) != 0)
        return -1;

    if (time_backup_set_hwclock(saved_time) != 0)
        return -1;

    return 0;
}
