/**
 * STM32 Integration Example
 * Demonstrates complete bit(N)-to-firmware workflow
 */

#include "generated.h"

void led_init(void) {
    uint32_t ctrl = GPIO->GPIO_OE;
    ctrl = GPIO_OE_LED_SET(ctrl, 1);
    GPIO->GPIO_OE = ctrl;
    
    uint32_t out = GPIO->GPIO_OUT;
    out = GPIO_OUT_LED_SET(out, 0);
    GPIO->GPIO_OUT = out;
}

void led_on(void) {
    uint32_t out = GPIO->GPIO_OUT;
    out = GPIO_OUT_LED_SET(out, 1);
    GPIO->GPIO_OUT = out;
}

void led_off(void) {
    uint32_t out = GPIO->GPIO_OUT;
    out = GPIO_OUT_LED_SET(out, 0);
    GPIO->GPIO_OUT = out;
}

void led_toggle(void) {
    uint32_t out = GPIO->GPIO_OUT;
    uint32_t current = GPIO_OUT_LED_GET(out);
    out = GPIO_OUT_LED_SET(out, !current);
    GPIO->GPIO_OUT = out;
}

void uart_init(uint32_t baudrate) {
    uint32_t ibrd = UART->UARTIBRD;
    ibrd = UART_IBRD_VALUE_SET(ibrd, 26);
    UART->UARTIBRD = ibrd;
    
    uint32_t fbrd = UART->UARTFBRD;
    fbrd = UART_FBRD_VALUE_SET(fbrd, 3);
    UART->UARTFBRD = fbrd;
    
    uint32_t lcr = UART->UARTLCR_H;
    lcr = UART_LCR_WLEN_SET(lcr, 0x3);
    lcr = UART_LCR_STOP_SET(lcr, 0);
    lcr = UART_LCR_PEN_SET(lcr, 0);
    UART->UARTLCR_H = lcr;
    
    uint32_t cr = UART->UARTCR;
    cr = UART_CR_UARTEN_SET(cr, 1);
    cr = UART_CR_TXE_SET(cr, 1);
    cr = UART_CR_RXE_SET(cr, 1);
    UART->UARTCR = cr;
}

void uart_putc(char c) {
    while ((UART->UARTFR & (1 << UART_FR_TXFF_BIT)) != 0) {
        // Poll TX FIFO full flag
    }
    
    uint32_t dr = UART->UARTDR;
    dr = UART_DR_DATA_SET(dr, (uint32_t)c);
    UART->UARTDR = dr;
}

void uart_puts(const char *str) {
    while (*str) {
        uart_putc(*str++);
    }
}

void spi_init(void) {
    uint32_t cpsr = SPI->SSPCPSR;
    cpsr = SPI_CPSR_CPSDVSR_SET(cpsr, 48);
    SPI->SSPCPSR = cpsr;
    
    uint32_t cr0 = SPI->SSPCR0;
    cr0 = SPI_CR0_SCR_SET(cr0, 0);
    cr0 = SPI_CR0_SPH_SET(cr0, 0);
    cr0 = SPI_CR0_SPO_SET(cr0, 0);
    cr0 = SPI_CR0_FRF_SET(cr0, 0);
    cr0 = SPI_CR0_DSS_SET(cr0, 0x7);
    SPI->SSPCR0 = cr0;
    
    uint32_t cr1 = SPI->SSPCR1;
    cr1 = SPI_CR1_SSE_SET(cr1, 1);
    cr1 = SPI_CR1_MS_SET(cr1, 0);
    SPI->SSPCR1 = cr1;
}

uint32_t spi_transfer(uint32_t data) {
    while ((SPI->SSPSR & (1 << SPI_SR_TNF_BIT)) == 0) {
        // Poll Transmit FIFO not full
    }
    
    uint32_t dr = SPI->SSPDR;
    dr = SPI_DR_DATA_SET(dr, data);
    SPI->SSPDR = dr;
    
    while ((SPI->SSPSR & (1 << SPI_SR_RNE_BIT)) == 0) {
        // Poll Receive FIFO not empty
    }
    
    return SPI_DR_DATA_GET(SPI->SSPDR);
}

void i2c_init(void) {
    uint32_t hcnt = I2C->IC_SS_SCL_HCNT;
    hcnt = I2C_SS_SCL_HCNT_VALUE_SET(hcnt, 240);
    I2C->IC_SS_SCL_HCNT = hcnt;
    
    uint32_t lcnt = I2C->IC_SS_SCL_LCNT;
    lcnt = I2C_SS_SCL_LCNT_VALUE_SET(lcnt, 240);
    I2C->IC_SS_SCL_LCNT = lcnt;
    
    uint32_t con = I2C->IC_CON;
    con = I2C_CON_MASTER_MODE_SET(con, 1);
    con = I2C_CON_SPEED_SET(con, 1);
    con = I2C_CON_IC_RESTART_EN_SET(con, 1);
    I2C->IC_CON = con;
    
    uint32_t en = I2C->IC_ENABLE;
    en = I2C_ENABLE_ENABLE_SET(en, 1);
    I2C->IC_ENABLE = en;
}

uint8_t i2c_read_register(uint8_t slave_addr, uint8_t reg_addr) {
    uint32_t tar = I2C->IC_TAR;
    tar = I2C_TAR_ADDRESS_SET(tar, slave_addr);
    I2C->IC_TAR = tar;
    
    uint32_t cmd = I2C->IC_DATA_CMD;
    cmd = I2C_DATA_CMD_DAT_SET(cmd, reg_addr);
    cmd = I2C_DATA_CMD_CMD_SET(cmd, 0);
    I2C->IC_DATA_CMD = cmd;
    
    cmd = I2C->IC_DATA_CMD;
    cmd = I2C_DATA_CMD_CMD_SET(cmd, 1);
    I2C->IC_DATA_CMD = cmd;
    
    while ((I2C->IC_STATUS & (1 << I2C_STATUS_RFNE_BIT)) == 0) {
        // Wait for receive FIFO not empty
    }
    
    return (uint8_t)I2C_DATA_CMD_DAT_GET(I2C->IC_DATA_CMD);
}

int main(void) {
    led_init();
    uart_init(115200);
    spi_init();
    i2c_init();
    
    uart_puts("\r\n=== bit(N) STM32F4 Example ===\r\n");
    
    while (1) {
        led_toggle();
        uart_puts("LED toggle\r\n");
        
        for (volatile int i = 0; i < 1000000; i++) {
            // Spin
        }
    }
    
    return 0;
}
