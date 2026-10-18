#include "oled.h"

#include <string.h>

#include "hardware/dma.h"
#include "hardware/i2c.h"
#include "hardware/structs/i2c.h"
#include "pico/stdlib.h"

#define OLED_I2C          i2c1

#define OLED_SDA          17
#define OLED_SCL          16

#define OLED_ADDRESS      0x3C

#define OLED_FB_SIZE      (OLED_WIDTH * OLED_HEIGHT / 8)
#define OLED_DMA_WORDS    (OLED_FB_SIZE + 1)
#define OLED_DMA_STOP_BIT (1u << I2C_IC_DATA_CMD_STOP_LSB)

static uint8_t framebuffer[OLED_FB_SIZE];

/*
 * DMA cannot read the framebuffer directly while the application is changing
 * it. Therefore we copy the current frame into this staging buffer and let DMA
 * send this immutable snapshot to the I2C TX FIFO.
 *
 * Each element is a 32-bit I2C DATA_CMD value:
 *   bits 7:0  - data byte
 *   bit 9     - STOP on the last byte
 */
static uint32_t dma_buffer[OLED_DMA_WORDS];
static int dma_channel = -1;
static dma_channel_config dma_config;


static void oled_command(uint8_t command) {
    uint8_t data[2] = { 0x00,    // Co=0, D/C#=0
                        command };

    i2c_write_blocking(OLED_I2C, OLED_ADDRESS, data, sizeof(data), false);
}


static bool oled_dma_idle(void) {
    if (dma_channel < 0)
        return false;

    /* DMA finished writing to the I2C TX FIFO. The I2C peripheral may still
     * be physically shifting the last bytes onto SDA/SCL, so also wait until
     * the bus is idle before reusing dma_buffer or starting another transfer.
     */
    if (dma_channel_is_busy((uint)dma_channel))
        return false;

    return (i2c_get_hw(OLED_I2C)->status & I2C_IC_STATUS_ACTIVITY_BITS) == 0;
}


// --------------------------------------------------
// Инициализация
// --------------------------------------------------
void oled_init(void) {
    i2c_init(OLED_I2C, 400 * 1000);

    gpio_set_function(OLED_SDA, GPIO_FUNC_I2C);
    gpio_set_function(OLED_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(OLED_SDA);
    gpio_pull_up(OLED_SCL);

    dma_channel = dma_claim_unused_channel(true);
    dma_config  = dma_channel_get_default_config((uint)dma_channel);

    channel_config_set_transfer_data_size(&dma_config, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_config, true);
    channel_config_set_write_increment(&dma_config, false);
    channel_config_set_dreq(&dma_config, i2c_get_dreq(OLED_I2C, true));

    sleep_ms(100);

    oled_command(0xAE);    // Display OFF

    oled_command(0x20);    // Set Memory Addressing Mode
    oled_command(0x00);    // Horizontal addressing mode

    oled_command(0xC8);    // COM scan direction
    oled_command(0x40);    // Start line = 0

    oled_command(0x81);    // Contrast
    oled_command(0x7F);

    oled_command(0xA1);    // Segment remap
    oled_command(0xA6);    // Normal display

    oled_command(0xA8);    // Multiplex ratio
    oled_command(0x3F);    // 64 rows

    oled_command(0xA4);    // Display follows RAM

    oled_command(0xD3);    // Display offset
    oled_command(0x00);

    oled_command(0xD5);    // Display clock
    oled_command(0x80);

    oled_command(0xD9);    // Pre-charge
    oled_command(0xF1);

    oled_command(0xDA);    // COM pins
    oled_command(0x12);

    oled_command(0xDB);    // VCOMH
    oled_command(0x40);

    oled_command(0x8D);    // Charge pump
    oled_command(0x14);

    oled_command(0xAF);    // Display ON

    oled_clear();
    oled_update();
}


// --------------------------------------------------
// Буфер экрана
// --------------------------------------------------

void oled_clear(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}


// --------------------------------------------------
// Вывести framebuffer на экран через DMA
// --------------------------------------------------

void oled_update(void) {
    if (!oled_dma_idle())
        return;

    oled_command(0x21);    // Set column address
    oled_command(0x00);    // Column start = 0
    oled_command(0x7F);    // Column end   = 127

    oled_command(0x22);    // Set page address
    oled_command(0x00);    // Page start = 0
    oled_command(0x07);    // Page end   = 7

    /*
     * Snapshot the framebuffer. The main loop can continue modifying
     * framebuffer while DMA sends this copy.
     */
    dma_buffer[0] = 0x40;    // Co=0, D/C#=1: following bytes are display data

    for (size_t i = 0; i < OLED_FB_SIZE; ++i) {
        dma_buffer[i + 1] = framebuffer[i];
    }

    /* STOP belongs to the final DATA_CMD word, not to the data byte itself. */
    dma_buffer[OLED_FB_SIZE] |= OLED_DMA_STOP_BIT;

    dma_channel_configure(
        (uint)dma_channel,
        &dma_config,
        &i2c_get_hw(OLED_I2C)->data_cmd,
        dma_buffer,
        OLED_DMA_WORDS,
        true
    );
}


// --------------------------------------------------
// Один пиксель
// --------------------------------------------------

void oled_draw_pixel(int x, int y, uint8_t value) {
    if (x < 0 || x >= OLED_WIDTH)
        return;

    if (y < 0 || y >= OLED_HEIGHT)
        return;

    uint32_t index = x + (y / 8) * OLED_WIDTH;
    uint8_t mask   = (uint8_t)(1u << (y % 8));

    if (value)
        framebuffer[index] |= mask;
    else
        framebuffer[index] &= (uint8_t)~mask;
}
