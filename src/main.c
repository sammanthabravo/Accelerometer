#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/display/mb_display.h>
#include <zephyr/sys/printk.h>
#include <math.h>

#define ACCEL_NODE DT_ALIAS(accel0)
#define STEP_ANGLE 45
#define M_PI 3.1416

const struct device *accel = DEVICE_DT_GET(ACCEL_NODE);
struct mb_display *disp;

static const struct mb_image multi_imag[] = {
    // Flechas hacia abajo según XY
    MB_IMAGE({0,0,1,0,0}, //bien
	     {0,0,0,1,0},
	     {1,1,1,1,1}, // -->
	     {0,0,0,1,0},
	     {0,0,1,0,0}),
    MB_IMAGE({0,0,1,1,1},
	     {0,0,0,1,1},
	     {0,0,1,0,1}, // intercambia esta por la 
	     {0,1,0,0,0},
	     {1,0,0,0,0}),
    MB_IMAGE({0,0,1,0,0},
	     {0,1,1,1,0},
             {1,0,1,0,1}, //bien
             {0,0,1,0,0},
             {0,0,1,0,0}),
    MB_IMAGE({1,1,1,0,0},
	     {1,1,0,0,0},
	     {1,0,1,0,0},
             {0,0,0,1,0},
             {0,0,0,0,1}),
    MB_IMAGE({0,0,1,0,0}, //bien
 	     {0,1,0,0,0},
	     {1,1,1,1,1},
	     {0,1,0,0,0},
	     {0,0,1,0,0}),
     MB_IMAGE({0,0,0,0,1},
	     {0,0,0,1,0},
	     {1,0,1,0,0},
	     {1,1,0,0,0},
	     {1,1,1,0,0}),
    MB_IMAGE({0,0,1,0,0},
	     {0,0,1,0,0},
	     {1,0,1,0,1}, // bien
	     {0,1,1,1,0},
	     {0,0,1,0,0}),
     MB_IMAGE({1,0,0,0,0},
	     {0,1,0,0,0},
	     {0,0,1,0,1}, //  veremos
	     {0,0,0,1,1},
	     {0,0,1,1,1}), 
    
    // Panel hacia arriba
    MB_IMAGE({0,0,0,0,0},
	     {0,1,0,1,0},{0,0,1,0,0},{0,1,0,1,0},{0,0,0,0,0}),
    // Panel hacia abajo
    MB_IMAGE({0,0,0,0,0},{0,0,0,0,0},{0,0,1,0,0},{0,0,0,0,0},{0,0,0,0,0})
};

void main(void) {
    disp = mb_display_get();
    if (!device_is_ready(accel)) {
        printk("Acelerómetro no disponible\n");
        return;
    }

    int last_part = -1;

    while (1) {
        struct sensor_value ax, ay, az;
        if (sensor_sample_fetch(accel) != 0 ||
            sensor_channel_get(accel, SENSOR_CHAN_ACCEL_X, &ax) != 0 ||
            sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Y, &ay) != 0 ||
            sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Z, &az) != 0) {
            printk("Error al leer acelerómetro\n");
            k_msleep(500);
            continue;
        }

        double Ax = ax.val1 + ax.val2 / 1000000.0;
        double Ay = ay.val1 + ay.val2 / 1000000.0;
        double Az = az.val1 + az.val2 / 1000000.0;

        double angle = atan2(Ay, Ax) * (180 / M_PI);
        if (angle < 0) angle += 360;

        int part = ((int)(angle / STEP_ANGLE)) % 8;

        if (part != last_part) {
            last_part = part;

            if (Az < -9.0) {
                printk("Panel hacia arriba: imagen 7\n");
                mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, 5 * MSEC_PER_SEC, &multi_imag[8], 1);
            } else if (Az > 9.0) {
                printk("Panel hacia abajo: imagen 8\n");
                mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, 5 * MSEC_PER_SEC, &multi_imag[9], 1);
            } else {
                printk("Orientación XY: parte %d (%.1f°–%.1f°)\n", part, part * STEP_ANGLE, (part + 1) * STEP_ANGLE);
                mb_display_image(disp, MB_DISPLAY_MODE_SINGLE, 5 * MSEC_PER_SEC, &multi_imag[part], 1);
            }
        }

        k_msleep(500);
    }
}





