/*
  제미나이가 맹글어준..( calude 인가) ...
*/


#include	"localLib.h"

/*
 * gpio_led.c - IMX6SX GPIO1_17 LED On/Off 제어 프로그램
 *
 * Linux 5.15.52 / IMX6SX
 * GPIO 번호 계산: GPIO1_17 = (1-1)*32 + 17 = 17
 *
 * 사용법:
 *   ./gpio_led on   - LED 켜기
 *   ./gpio_led off  - LED 끄기
 *   ./gpio_led blink [횟수] [간격ms] - LED 깜박임 (기본: 5회, 500ms)
 *
 * 컴파일:
 *   arm-linux-gnueabihf-gcc -o gpio_led gpio_led.c
 *   또는 보드 위에서: gcc -o gpio_led gpio_led.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* -------------------------------------------------------
 * GPIO 설정
 * GPIO 번호 = (Controller - 1) * 32 + Pin
 * GPIO1_17  = (1 - 1) * 32 + 17 = 17
 * ------------------------------------------------------- */
#define GPIO_NUMBER         17


#define LAN2_GPIO_NUMBER  14
#define LNA3_GPIO_NUMBER  15

#define GPIO_DIRECTION_OUT  "out"
#define GPIO_DIRECTION_IN   "in"
#define GPIO_VALUE_HIGH     "1"
#define GPIO_VALUE_LOW      "0"



#define SYSFS_GPIO_PATH     "/sys/class/gpio"
#define GPIO_EXPORT_PATH    SYSFS_GPIO_PATH "/export"
#define GPIO_UNEXPORT_PATH  SYSFS_GPIO_PATH "/unexport"

/* -------------------------------------------------------
 * 경로 버퍼
 * ------------------------------------------------------- */
static char gpio_dir_path[64];
static char gpio_value_path[64];
static char gpio_direction_path[64];

/* -------------------------------------------------------
 * 유틸리티: sysfs 파일에 문자열 쓰기
 * ------------------------------------------------------- */
static int sysfs_write(const char *path, const char *value)
{
    int fd;
    ssize_t len;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open(%s) failed: %s\n", path, strerror(errno));
        return -1;
    }

    len = write(fd, value, strlen(value));
    if (len < 0) {
        fprintf(stderr, "[ERROR] write(%s, %s) failed: %s\n", path, value, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/* -------------------------------------------------------
 * GPIO Export
 * /sys/class/gpio/export 에 GPIO 번호를 기록하면
 * /sys/class/gpio/gpio17/ 디렉터리가 생성된다.
 * 이미 export된 경우 EBUSY 에러가 발생하므로 무시한다.
 * ------------------------------------------------------- */
static int gpio_export(int gpio)
{
    char num_str[8];

    snprintf(num_str, sizeof(num_str), "%d", gpio);

    if (access(gpio_dir_path, F_OK) == 0) {
        /* 이미 export 되어있음 */
        printf("[INFO] GPIO%d is already exported.\n", gpio);
        return 0;
    }

    if (sysfs_write(GPIO_EXPORT_PATH, num_str) < 0) {
        return -1;
    }

    /* export 후 sysfs 노드 생성까지 짧은 대기 */
    usleep(100 * 1000); /* 100ms */
    printf("[INFO] GPIO%d exported.\n", gpio);
    return 0;
}

/* -------------------------------------------------------
 * GPIO Unexport
 * ------------------------------------------------------- */
static int gpio_unexport(int gpio)
{
    char num_str[8];

    snprintf(num_str, sizeof(num_str), "%d", gpio);

    if (sysfs_write(GPIO_UNEXPORT_PATH, num_str) < 0) {
        return -1;
    }

    printf("[INFO] GPIO%d unexported.\n", gpio);
    return 0;
}

/* -------------------------------------------------------
 * GPIO Direction 설정
 * ------------------------------------------------------- */
static int gpio_set_direction(const char *direction)
{
    return sysfs_write(gpio_direction_path, direction);
}

/* -------------------------------------------------------
 * GPIO Value 설정 (LED ON/OFF)
 * ------------------------------------------------------- */
static int gpio_set_value(int value)
{
    return sysfs_write(gpio_value_path, value ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW);
}

/* -------------------------------------------------------
 * GPIO direction 읽기
 * 반환값: 1=output, 0=input, -1=error
 * ------------------------------------------------------- */
static int gpio_get_direction(void)
{
    int fd;
    char buf[8] = {0};

    fd = open(gpio_direction_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open(%s) failed: %s\n", gpio_direction_path, strerror(errno));
        return -1;
    }

    if (read(fd, buf, sizeof(buf) - 1) < 0) {
        fprintf(stderr, "[ERROR] read(%s) failed: %s\n", gpio_direction_path, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);

    /* "out\n" 또는 "in\n" */
    return (strncmp(buf, "out", 3) == 0) ? 1 : 0;
}

/* -------------------------------------------------------
 * GPIO 초기화
 *  - Export
 *  - Direction: 이미 "out"으로 설정된 경우 재설정 SKIP
 *                → value가 0으로 초기화되는 부작용 방지
 * ------------------------------------------------------- */
static int gpio_init(int gpio)
{
    int already_exported;

    snprintf(gpio_dir_path,       sizeof(gpio_dir_path),       SYSFS_GPIO_PATH "/gpio%d",           gpio);
    snprintf(gpio_value_path,     sizeof(gpio_value_path),     SYSFS_GPIO_PATH "/gpio%d/value",     gpio);
    snprintf(gpio_direction_path, sizeof(gpio_direction_path), SYSFS_GPIO_PATH "/gpio%d/direction", gpio);

    /* export 전에 이미 존재하는지 확인 */
    already_exported = (access(gpio_dir_path, F_OK) == 0);

    if (gpio_export(gpio) < 0)
        return -1;

    /*
     * ★ 핵심 수정 ★
     * 이미 export 되어 있고 direction이 "out"인 경우
     * direction을 다시 쓰지 않는다.
     * → "out" 재설정 시 커널이 value를 0으로 초기화하는
     *   부작용(side-effect)을 방지한다.
     */
    if (already_exported && gpio_get_direction() == 1) {
        printf("[INFO] GPIO%d already configured as output. Skip direction set.\n", gpio);
    } else {
        if (gpio_set_direction(GPIO_DIRECTION_OUT) < 0) {
            fprintf(stderr, "[ERROR] Failed to set GPIO%d direction to output.\n", gpio);
            return -1;
        }
        printf("[INFO] GPIO%d initialized as output.\n", gpio);
    }

    return 0;
}

/* -------------------------------------------------------
 * LED ON
 * ------------------------------------------------------- */
static int led_on(void)
{
    if (gpio_set_value(1) < 0) {
        fprintf(stderr, "[ERROR] LED ON failed.\n");
        return -1;
    }
    printf("[INFO] LED ON\n");
    return 0;
}

/* -------------------------------------------------------
 * LED OFF
 * ------------------------------------------------------- */
static int led_off(void)
{
    if (gpio_set_value(0) < 0) {
        fprintf(stderr, "[ERROR] LED OFF failed.\n");
        return -1;
    }
    printf("[INFO] LED OFF\n");
    return 0;
}

/* -------------------------------------------------------
 * GPIO Value 읽기
 * 반환값: 0 또는 1, 실패 시 -1
 * ------------------------------------------------------- */
static int gpio_get_value(void)
{
    int fd;
    char buf[4] = {0};
    int value;

    fd = open(gpio_value_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open(%s) failed: %s\n", gpio_value_path, strerror(errno));
        return -1;
    }

    if (read(fd, buf, sizeof(buf) - 1) < 0) {
        fprintf(stderr, "[ERROR] read(%s) failed: %s\n", gpio_value_path, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);

    value = atoi(buf);
    return value;  /* 0 or 1 */
}

/* -------------------------------------------------------
 * LED Toggle
 *  현재 상태를 읽어서 반전 출력
 *  ON  → OFF
 *  OFF → ON
 * ------------------------------------------------------- */
/* sttic 제거 ..called by wdt */
int led_toggle(void)
{
    int current = gpio_get_value();

    if (current < 0) {
        fprintf(stderr, "[ERROR] Failed to read current GPIO value.\n");
        return -1;
    }

 //   printf("[INFO] Current state: %s  →  ", current ? "ON" : "OFF");

    if (gpio_set_value(!current) < 0) {
        fprintf(stderr, "[ERROR] LED Toggle failed.\n");
        return -1;
    }

//    printf("%s\n", current ? "OFF" : "ON");
    return 0;
}

/* -------------------------------------------------------
 * LED Blink
 *  count   : 깜박임 횟수
 *  delay_ms: ON/OFF 간격 (밀리초)
 * ------------------------------------------------------- */
static int led_blink(int count, int delay_ms)
{
    int i;

    printf("[INFO] LED Blink: %d times, interval %dms\n", count, delay_ms);

    for (i = 0; i < count; i++) {
        if (gpio_set_value(1) < 0) return -1;
        printf("  [%d/%d] ON\n", i + 1, count);
        usleep(delay_ms * 1000);

        if (gpio_set_value(0) < 0) return -1;
        printf("  [%d/%d] OFF\n", i + 1, count);
        usleep(delay_ms * 1000);
    }
    return 0;
}


int initRunLad(void)
{
    int ret = 0;

    /* GPIO 초기화 */
    if (gpio_init(GPIO_NUMBER) < 0) {
        fprintf(stderr, "[ERROR] GPIO initialization failed.\n");
        return -1 ; // EXIT_FAILURE;
    }
    return 0; 

}
#if 0

/* -------------------------------------------------------
 * 사용법 출력
 * ------------------------------------------------------- */
static void print_usage(const char *prog)
{
    printf("Usage:\n");
    printf("  %s on                       - LED 켜기\n", prog);
    printf("  %s off                      - LED 끄기\n", prog);
    printf("  %s toggle                   - LED 현재 상태 반전 (ON↔OFF)\n", prog);
    printf("  %s blink [count] [delay_ms] - LED 깜박임\n", prog);
    printf("                                 count   : 깜박임 횟수 (기본값: 5)\n");
    printf("                                 delay_ms: 간격(ms)    (기본값: 500)\n");
    printf("\n");
    printf("GPIO 정보:\n");
    printf("  Controller : GPIO1\n");
    printf("  Pin        : IO17\n");
    printf("  sysfs 번호 : %d  (= (1-1)*32 + 17)\n", GPIO_NUMBER);
    printf("  sysfs 경로 : /sys/class/gpio/gpio%d/\n", GPIO_NUMBER);
}

/* -------------------------------------------------------
 * main
 * ------------------------------------------------------- */
int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* GPIO 초기화 */
    if (gpio_init(GPIO_NUMBER) < 0) {
        fprintf(stderr, "[ERROR] GPIO initialization failed.\n");
        return EXIT_FAILURE;
    }

    /* 명령 처리 */
    if (strcmp(argv[1], "on") == 0) {
        ret = led_on();

    } else if (strcmp(argv[1], "off") == 0) {
        ret = led_off();

    } else if (strcmp(argv[1], "toggle") == 0) {
        ret = led_toggle();

    } else if (strcmp(argv[1], "blink") == 0) {
        int count    = (argc >= 3) ? atoi(argv[2]) : 5;
        int delay_ms = (argc >= 4) ? atoi(argv[3]) : 500;

        if (count <= 0 || delay_ms <= 0) {
            fprintf(stderr, "[ERROR] count와 delay_ms는 양수여야 합니다.\n");
            ret = -1;
        } else {
            ret = led_blink(count, delay_ms);
            /* 종료 시 LED OFF */
            led_off();
        }

    } else {
        fprintf(stderr, "[ERROR] 알 수 없는 명령: %s\n\n", argv[1]);
        print_usage(argv[0]);
        ret = -1;
    }

    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif 