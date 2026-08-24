/*
 * gpio_lan_ctrl.c - IMX6SX GPIO1_14(LAN2) / GPIO1_15(LAN3) 제어 프로그램
 *
 * Linux 5.15.52 / IMX6SX
 *
 * GPIO 번호 계산 (GPIO 번호 = (Controller-1)*32 + Pin):
 *   GPIO1_IO14 (LAN2) = (1-1)*32 + 14 = 14
 *   GPIO1_IO15 (LAN3) = (1-1)*32 + 15 = 15
 *
 * 사용법:
 *   ./gpio_lan_ctrl <target> <command> [options]
 *
 *   target  : lan2 | lan3 | all
 *   command : on | off | toggle | blink [count] [delay_ms]
 *
 * 예시:
 *   ./gpio_lan_ctrl lan2 on            - LAN2 ON
 *   ./gpio_lan_ctrl lan3 off           - LAN3 OFF
 *   ./gpio_lan_ctrl lan2 toggle        - LAN2 상태 반전
 *   ./gpio_lan_ctrl all on             - LAN2 + LAN3 동시 ON
 *   ./gpio_lan_ctrl all blink 5 300    - LAN2 + LAN3 동시 5회 깜박임
 *
 * 컴파일:
 *   arm-linux-gnueabihf-gcc -o gpio_lan_ctrl gpio_lan_ctrl.c
 *   또는 보드에서: gcc -o gpio_lan_ctrl gpio_lan_ctrl.c
 
 * 2026-05-11 오후 5:54:26  by Claude
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/* -------------------------------------------------------
 * GPIO 핀 정의
 * GPIO 번호 = (Controller - 1) * 32 + Pin
 * GPIO1_IO14 (LAN2) = (1-1)*32 + 14 = 14
 * GPIO1_IO15 (LAN3) = (1-1)*32 + 15 = 15
 * ------------------------------------------------------- */
#define LAN2_GPIO           14
#define LAN3_GPIO           15

#define GPIO_DIRECTION_OUT  "out"
#define GPIO_DIRECTION_IN   "in"
#define GPIO_VALUE_HIGH     "1"
#define GPIO_VALUE_LOW      "0"

#define SYSFS_GPIO_PATH     "/sys/class/gpio"
#define GPIO_EXPORT_PATH    SYSFS_GPIO_PATH "/export"
#define GPIO_UNEXPORT_PATH  SYSFS_GPIO_PATH "/unexport"

/* -------------------------------------------------------
 * GPIO 컨텍스트 구조체
 *  - 2개의 GPIO를 독립적으로 관리하기 위해
 *    경로 버퍼를 구조체로 분리
 * ------------------------------------------------------- */
typedef struct {
    int  gpio_num;                  /* sysfs GPIO 번호         */
    char name[16];                  /* 표시용 이름 (LAN2/LAN3) */
    char dir_path[64];              /* /sys/class/gpio/gpioN   */
    char value_path[72];            /* .../gpioN/value         */
    char direction_path[76];        /* .../gpioN/direction     */
} gpio_ctx_t;

/* 전역 GPIO 컨텍스트 (LAN2, LAN3) */
static gpio_ctx_t g_lan2;
static gpio_ctx_t g_lan3;

/* -------------------------------------------------------
 * 유틸리티: sysfs 파일에 문자열 쓰기
 * ------------------------------------------------------- */
static int sysfs_write(const char *path, const char *value)
{
    int     fd;
    ssize_t len;

    fd = open(path, O_WRONLY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open(%s) failed: %s\n",
                path, strerror(errno));
        return -1;
    }

    len = write(fd, value, strlen(value));
    if (len < 0) {
        fprintf(stderr, "[ERROR] write(%s, %s) failed: %s\n",
                path, value, strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

/* -------------------------------------------------------
 * GPIO Export
 *  - 이미 export 되어있으면 skip
 * ------------------------------------------------------- */
static int gpio_export(gpio_ctx_t *ctx)
{
    char num_str[8];

    if (access(ctx->dir_path, F_OK) == 0) {
        printf("[INFO] GPIO%d (%s) is already exported.\n",
               ctx->gpio_num, ctx->name);
        return 0;
    }

    snprintf(num_str, sizeof(num_str), "%d", ctx->gpio_num);

    if (sysfs_write(GPIO_EXPORT_PATH, num_str) < 0)
        return -1;

    usleep(100 * 1000); /* export 후 sysfs 노드 생성 대기: 100ms */
    printf("[INFO] GPIO%d (%s) exported.\n", ctx->gpio_num, ctx->name);
    return 0;
}

/* -------------------------------------------------------
 * GPIO Unexport
 * ------------------------------------------------------- */
static int gpio_unexport(gpio_ctx_t *ctx)
{
    char num_str[8];

    snprintf(num_str, sizeof(num_str), "%d", ctx->gpio_num);

    if (sysfs_write(GPIO_UNEXPORT_PATH, num_str) < 0)
        return -1;

    printf("[INFO] GPIO%d (%s) unexported.\n", ctx->gpio_num, ctx->name);
    return 0;
}

/* -------------------------------------------------------
 * GPIO Direction 읽기
 *  반환값: 1=output, 0=input, -1=error
 * ------------------------------------------------------- */
static int gpio_get_direction(gpio_ctx_t *ctx)
{
    int  fd;
    char buf[8] = {0};

    fd = open(ctx->direction_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open(%s) failed: %s\n",
                ctx->direction_path, strerror(errno));
        return -1;
    }

    if (read(fd, buf, sizeof(buf) - 1) < 0) {
        fprintf(stderr, "[ERROR] read(%s) failed: %s\n",
                ctx->direction_path, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);

    return (strncmp(buf, "out", 3) == 0) ? 1 : 0;
}

/* -------------------------------------------------------
 * GPIO Direction 설정
 * ------------------------------------------------------- */
static int gpio_set_direction(gpio_ctx_t *ctx, const char *direction)
{
    return sysfs_write(ctx->direction_path, direction);
}

/* -------------------------------------------------------
 * GPIO Value 읽기
 *  반환값: 0 또는 1, 실패 시 -1
 * ------------------------------------------------------- */
static int gpio_get_value(gpio_ctx_t *ctx)
{
    int  fd;
    char buf[4] = {0};

    fd = open(ctx->value_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "[ERROR] open(%s) failed: %s\n",
                ctx->value_path, strerror(errno));
        return -1;
    }

    if (read(fd, buf, sizeof(buf) - 1) < 0) {
        fprintf(stderr, "[ERROR] read(%s) failed: %s\n",
                ctx->value_path, strerror(errno));
        close(fd);
        return -1;
    }
    close(fd);

    return atoi(buf); /* 0 or 1 */
}

/* -------------------------------------------------------
 * GPIO Value 설정
 * ------------------------------------------------------- */
static int gpio_set_value(gpio_ctx_t *ctx, int value)
{
    return sysfs_write(ctx->value_path,
                       value ? GPIO_VALUE_HIGH : GPIO_VALUE_LOW);
}

/* -------------------------------------------------------
 * GPIO 초기화
 *  - 경로 문자열 설정
 *  - Export
 *  - Direction = out  (이미 out이면 Skip → value 보존)
 * ------------------------------------------------------- */
static int gpio_init(gpio_ctx_t *ctx)
{
    int already_exported;

    snprintf(ctx->dir_path,       sizeof(ctx->dir_path),
             SYSFS_GPIO_PATH "/gpio%d",           ctx->gpio_num);
    snprintf(ctx->value_path,     sizeof(ctx->value_path),
             SYSFS_GPIO_PATH "/gpio%d/value",     ctx->gpio_num);
    snprintf(ctx->direction_path, sizeof(ctx->direction_path),
             SYSFS_GPIO_PATH "/gpio%d/direction", ctx->gpio_num);

    /* export 전에 이미 존재 여부 확인 */
    already_exported = (access(ctx->dir_path, F_OK) == 0);

    if (gpio_export(ctx) < 0)
        return -1;

    /*
     * ★ direction 재설정 Skip 로직 ★
     * 이미 "out"으로 설정된 경우 재설정하지 않는다.
     * direction에 "out"을 쓰면 커널이 value를 0으로 강제
     * 초기화하는 부작용이 있어 toggle 동작이 깨지기 때문.
     */
    if (already_exported && gpio_get_direction(ctx) == 1) {
        printf("[INFO] GPIO%d (%s) already output. Skip direction set.\n",
               ctx->gpio_num, ctx->name);
    } else {
        if (gpio_set_direction(ctx, GPIO_DIRECTION_OUT) < 0) {
            fprintf(stderr, "[ERROR] GPIO%d (%s) direction set failed.\n",
                    ctx->gpio_num, ctx->name);
            return -1;
        }
        printf("[INFO] GPIO%d (%s) initialized as output.\n",
               ctx->gpio_num, ctx->name);
    }

    return 0;
}

/* -------------------------------------------------------
 * lan_on() - 지정 GPIO LAN 포트 활성화
 *   gpio_number : LAN2_GPIO(14) 또는 LAN3_GPIO(15)
 * ------------------------------------------------------- */
static int lan_on(int gpio_number)
{
    gpio_ctx_t *ctx;

    if (gpio_number == LAN2_GPIO)
        ctx = &g_lan2;
    else if (gpio_number == LAN3_GPIO)
        ctx = &g_lan3;
    else {
        fprintf(stderr, "[ERROR] lan_on: unknown gpio_number %d\n",
                gpio_number);
        return -1;
    }

    if (gpio_set_value(ctx, 1) < 0) {
        fprintf(stderr, "[ERROR] %s ON failed.\n", ctx->name);
        return -1;
    }
    printf("[INFO] %s (GPIO%d) ON\n", ctx->name, ctx->gpio_num);
    return 0;
}

/* -------------------------------------------------------
 * lan_off() - 지정 GPIO LAN 포트 비활성화
 *   gpio_number : LAN2_GPIO(14) 또는 LAN3_GPIO(15)
 * ------------------------------------------------------- */
static int lan_off(int gpio_number)
{
    gpio_ctx_t *ctx;

    if (gpio_number == LAN2_GPIO)
        ctx = &g_lan2;
    else if (gpio_number == LAN3_GPIO)
        ctx = &g_lan3;
    else {
        fprintf(stderr, "[ERROR] lan_off: unknown gpio_number %d\n",
                gpio_number);
        return -1;
    }

    if (gpio_set_value(ctx, 0) < 0) {
        fprintf(stderr, "[ERROR] %s OFF failed.\n", ctx->name);
        return -1;
    }
    printf("[INFO] %s (GPIO%d) OFF\n", ctx->name, ctx->gpio_num);
    return 0;
}
#if 0
/* -------------------------------------------------------
 * lan_toggle() - 현재 상태를 읽어 반전
 *   ON  → OFF
 *   OFF → ON
 * ------------------------------------------------------- */
static int lan_toggle(gpio_ctx_t *ctx)
{
    int current = gpio_get_value(ctx);

    if (current < 0) {
        fprintf(stderr, "[ERROR] %s: Failed to read current value.\n",
                ctx->name);
        return -1;
    }

    printf("[INFO] %s (GPIO%d) Current: %s  ->  ",
           ctx->name, ctx->gpio_num, current ? "ON" : "OFF");

    if (gpio_set_value(ctx, !current) < 0) {
        fprintf(stderr, "[ERROR] %s toggle failed.\n", ctx->name);
        return -1;
    }

    printf("%s\n", current ? "OFF" : "ON");
    return 0;
}


/* -------------------------------------------------------
 * lan_blink() - 지정 횟수/간격으로 깜박임
 * ------------------------------------------------------- */
static int lan_blink(gpio_ctx_t *ctx, int count, int delay_ms)
{
    int i;

    printf("[INFO] %s (GPIO%d) Blink: %d times, %dms interval\n",
           ctx->name, ctx->gpio_num, count, delay_ms);

    for (i = 0; i < count; i++) {
        if (gpio_set_value(ctx, 1) < 0) return -1;
        printf("  [%d/%d] %s ON\n",  i + 1, count, ctx->name);
        usleep(delay_ms * 1000);

        if (gpio_set_value(ctx, 0) < 0) return -1;
        printf("  [%d/%d] %s OFF\n", i + 1, count, ctx->name);
        usleep(delay_ms * 1000);
    }
    return 0;
}

/* -------------------------------------------------------
 * 사용법 출력
 * ------------------------------------------------------- */
static void print_usage(const char *prog)
{
    printf("\n");
    printf("Usage: %s <target> <command> [count] [delay_ms]\n", prog);
    printf("\n");
    printf("  target:\n");
    printf("    lan2          - LAN2 (GPIO1_IO14 = gpio%d) 제어\n", LAN2_GPIO);
    printf("    lan3          - LAN3 (GPIO1_IO15 = gpio%d) 제어\n", LAN3_GPIO);
    printf("    all           - LAN2 + LAN3 동시 제어\n");
    printf("\n");
    printf("  command:\n");
    printf("    on            - LAN 포트 활성화 (HIGH)\n");
    printf("    off           - LAN 포트 비활성화 (LOW)\n");
    printf("    toggle        - 현재 상태 반전 (ON->OFF / OFF->ON)\n");
    printf("    blink [N] [T] - N회 T ms 간격으로 깜박임 (기본: 5회, 500ms)\n");
    printf("\n");
    printf("  GPIO 정보:\n");
    printf("    LAN2 : GPIO1_IO14  sysfs -> /sys/class/gpio/gpio%d/\n", LAN2_GPIO);
    printf("    LAN3 : GPIO1_IO15  sysfs -> /sys/class/gpio/gpio%d/\n", LAN3_GPIO);
    printf("\n");
    printf("  예시:\n");
    printf("    %s lan2 on\n", prog);
    printf("    %s lan3 off\n", prog);
    printf("    %s lan2 toggle\n", prog);
    printf("    %s all on\n", prog);
    printf("    %s all blink 10 300\n", prog);
    printf("\n");
}
#endif 
#if 0
/* -------------------------------------------------------
 * 단일 GPIO 컨텍스트에 대한 명령 처리
 * ------------------------------------------------------- */
static int do_command(gpio_ctx_t *ctx, int argc, char *argv[], int cmd_idx)
{
    const char *cmd = argv[cmd_idx];

    if (strcmp(cmd, "on") == 0) {
        return lan_on(ctx->gpio_num);

    } else if (strcmp(cmd, "off") == 0) {
        return lan_off(ctx->gpio_num);

    } else if (strcmp(cmd, "toggle") == 0) {
        return lan_toggle(ctx);

    } else if (strcmp(cmd, "blink") == 0) {
        int count    = (argc > cmd_idx + 1) ? atoi(argv[cmd_idx + 1]) : 5;
        int delay_ms = (argc > cmd_idx + 2) ? atoi(argv[cmd_idx + 2]) : 500;

        if (count <= 0 || delay_ms <= 0) {
            fprintf(stderr,
                    "[ERROR] count와 delay_ms는 양수여야 합니다.\n");
            return -1;
        }
        int ret = lan_blink(ctx, count, delay_ms);
        lan_off(ctx->gpio_num); /* 종료 시 OFF */
        return ret;

    } else {
        fprintf(stderr, "[ERROR] 알 수 없는 명령: %s\n", cmd);
        return -1;
    }
}
#endif 
/* -------------------------------------------------------
 * 단일 GPIO 컨텍스트에 대한 명령 처리
 *   ctx     : 제어할 GPIO 컨텍스트
 *   cmd     : "on" | "off" | "toggle" | "blink"
 *   count   : blink 횟수    (blink 이외 명령에서는 무시됨)
 *   delay_ms: blink 간격 ms (blink 이외 명령에서는 무시됨)
 * ------------------------------------------------------- */
static int do_command(gpio_ctx_t *ctx, const char *cmd)
{
    if (strcmp(cmd, "on") == 0) {
        return lan_on(ctx->gpio_num);
 
    } else if (strcmp(cmd, "off") == 0) {
        return lan_off(ctx->gpio_num);
 
    } 
    else {
        fprintf(stderr, "[ERROR] 알 수 없는 명령: %s\n", cmd);
        return -1;
    }
}

void lan_gpio_init(void)
{
    /* ── GPIO 컨텍스트 초기화 ── */
    g_lan2.gpio_num = LAN2_GPIO;
    snprintf(g_lan2.name, sizeof(g_lan2.name), "LAN2");

    g_lan3.gpio_num = LAN3_GPIO;
    snprintf(g_lan3.name, sizeof(g_lan3.name), "LAN3");    
  
  
    if (gpio_init(&g_lan2) < 0) {
            fprintf(stderr, "[ERROR] LAN2 GPIO init failed.\n");
            exit(1);
    }  
    
    if ( do_command(&g_lan2,"on") < 0 )
    {
        printf("WDT-ERR> LAN2 ON FAILED!!!\r\n");
        exit(1);
    }
    sleep(1);
    
    if (gpio_init(&g_lan3) < 0) {
            fprintf(stderr, "[ERROR] LAN3 GPIO init failed.\n");
            exit(1);
    }  
    
    if ( do_command(&g_lan3,"on") < 0 )
    {
        printf("WDT-ERR> LAN3 ON FAILED!!!\r\n");
        exit(1);
    }    
    
    if ( do_command(&g_lan3,"on") < 0 )
    {
        printf("WDT-ERR> LAN3 ON FAILED!!!\r\n");
        exit(1);
    }       

    (void)gpio_unexport;
}

#if 0

/* -------------------------------------------------------
 * main
 * ------------------------------------------------------- */
int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* ── GPIO 컨텍스트 초기화 ── */
    g_lan2.gpio_num = LAN2_GPIO;
    snprintf(g_lan2.name, sizeof(g_lan2.name), "LAN2");

    g_lan3.gpio_num = LAN3_GPIO;
    snprintf(g_lan3.name, sizeof(g_lan3.name), "LAN3");

    /* ── target 파싱 ── */
    const char *target = argv[1];
    const char *cmd    = argv[2];

    if (strcmp(target, "lan2") == 0) {
        /* LAN2 단독 제어 */
        if (gpio_init(&g_lan2) < 0) {
            fprintf(stderr, "[ERROR] LAN2 GPIO init failed.\n");
            return EXIT_FAILURE;
        }
        ret = do_command(&g_lan2, argc, argv, 2);

    } else if (strcmp(target, "lan3") == 0) {
        /* LAN3 단독 제어 */
        if (gpio_init(&g_lan3) < 0) {
            fprintf(stderr, "[ERROR] LAN3 GPIO init failed.\n");
            return EXIT_FAILURE;
        }
        ret = do_command(&g_lan3, argc, argv, 2);

    } else if (strcmp(target, "all") == 0) {
        /* LAN2 + LAN3 동시 제어 */
        if (gpio_init(&g_lan2) < 0) {
            fprintf(stderr, "[ERROR] LAN2 GPIO init failed.\n");
            return EXIT_FAILURE;
        }
        if (gpio_init(&g_lan3) < 0) {
            fprintf(stderr, "[ERROR] LAN3 GPIO init failed.\n");
            return EXIT_FAILURE;
        }

        printf("[INFO] ---- LAN2 ----\n");
        ret  = do_command(&g_lan2, argc, argv, 2);

        printf("[INFO] ---- LAN3 ----\n");
        ret |= do_command(&g_lan3, argc, argv, 2);

    } else {
        fprintf(stderr, "[ERROR] 알 수 없는 target: %s\n\n", target);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* 미사용 경고 억제 (gpio_unexport는 필요시 호출) */
    (void)gpio_unexport;

    return (ret == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#endif 